#include "channel.h"
//#include "event.h"
#include "util.h"
using namespace tudou;
class Eventloop;
Channel::Channel(EventLoop* loop,  int fd)
: _fd(fd)
, _loop(loop)
{
	_event = EPOLLIN | EPOLLET;
	setNonBlock(fd);
	_loop->addChannel(this);
}

void Channel::enableRead(bool flag)
{
	if(flag)
	{
		_event |= (EPOLLIN | EPOLLET);
	}
	else
	{
		_event &= ~(EPOLLIN | EPOLLET);
	}
	//auto ptr = shared_from_this();
	_loop->update(this);
}

void Channel::enableWrite(bool flag)
{
	if(flag)
	{
		_event |= EPOLLOUT;
	}
	else
	{
		_event &= ~EPOLLOUT;
	}
	_loop->update(this);
}

void Channel::update()
{
	_loop->update(this);
}


Epoll::~Epoll()
{
	for(auto i = _active_channel.begin(); i!=_active_channel.end(); ++i)
	{
		std::cout << "close" << std::endl;
		(*i)->close();
	}
	close(_epoll_root);
}

void Epoll::addChannel(Channel* ch) //add channel的时候会把channel的文件描述符加入epoll树
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = ch->getEvent();
	ev.data.ptr = ch;
	int ret = epoll_ctl(_epoll_root, EPOLL_CTL_ADD, ch->getFd(), &ev);
	if(ret == -1)
	{
		std::cout << "errcout" << std::endl;
	}
	_active_channel.emplace(ch);

}
void Epoll::removeChannel(Channel* ch) 
{
	_active_channel.erase(ch);
	epoll_ctl(_epoll_root, EPOLL_CTL_DEL, ch->getFd(), nullptr);
}
void Epoll::updateChannel(Channel* ch)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = ch->getEvent();
	ev.data.ptr = ch;
	int ret = epoll_ctl(_epoll_root, EPOLL_CTL_MOD, ch->getFd(), &ev);
}
int Epoll::size() const
{
	return _active_channel.size();
}
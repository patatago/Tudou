#include "epoll.h"
using namespace tudou;


Epoll::~Epoll()
{
	for(auto i = _active_channel.begin(); i!=_active_channel.end(); ++i)
	{
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
		Warn << strerror(errno) << std::endl;
		return ;
	}
	_active_channel.emplace(ch);

}
void Epoll::removeChannel(Channel* ch) 
{
	_active_channel.erase(ch);
	int ret = epoll_ctl(_epoll_root, EPOLL_CTL_DEL, ch->getFd(), nullptr);
	if(ret == -1)
	{
		Warn << "remove channel from epoll error" << std::endl;
	}
}
void Epoll::updateChannel(Channel* ch)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = ch->getEvent();
	ev.data.ptr = ch;
	int ret = epoll_ctl(_epoll_root, EPOLL_CTL_MOD, ch->getFd(), &ev);
	if(ret == -1)
	{
		Warn << "update epoll error" << std::endl;
	}
}
int Epoll::size() const
{
	return _active_channel.size();
}
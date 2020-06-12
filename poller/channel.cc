#include "channel.h"

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

bool Channel::isRead()
{
	if(_event & EPOLLIN) { return true; }
	return false;
}

bool Channel::isWrite()
{
	if(_event & EPOLLOUT) { return true; }
	return false;
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
	_loop->update(this);
}

void Channel::enableWrite(bool flag)
{
	if(flag)
	{
		_event |= (EPOLLOUT | EPOLLET);
	}
	else
	{
		_event &= ~(EPOLLOUT | EPOLLET);
	}
	_loop->update(this);
}

void Channel::update()
{
	_loop->update(this);
}

void Channel::setReadCb(const DefaultFunction & read_cb)
{
	_read_cb = read_cb;
}
void Channel::setWriteCb(const DefaultFunction & write_cb)
{
	_write_cb = write_cb;
}
void Channel::setErrorCb(const DefaultFunction & error_cb)
{
	_error_cb = error_cb;
}
//////////////
void Channel::setCloseCb(const DefaultFunction & close_cb)
{
	_close_cb = close_cb;
}

void Channel::close()
{
	enableRead(false);
	enableWrite(false);
	if(_fd >= 0)
	{
		::close(_fd);
		_fd = -1;
		_close_cb(); 
	}

}
void Channel::runRead() 
{
	if(_read_cb)
	{
		_read_cb(); 
	}
}
void Channel::runWrite() 
{
	if(_write_cb)
	{
		_write_cb();
	}
}
void Channel::runError() 
{
	if(_error_cb)
	{
		_error_cb(); 
	}
}
void Channel::runClose()
{
	if(_close_cb)
	{
		_close_cb();
	}
}


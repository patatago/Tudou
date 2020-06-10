#ifndef __TUDOU_CHANNEL_H__
#define __TUDOU_CHANNEL_H__
#include "event.h"	
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unordered_set>
#include <vector>
#include <string.h>
#include <memory>

using namespace std;
namespace tudou
{
	
class EventLoop;

class Channel //是对epoll fd的封装，所有channel事件都会挂到epoll树上
//: enable_shared_from_this<Channel>
{
public:
	using Ptr = shared_ptr<Channel>;
	Channel(EventLoop* loop, int fd);

	~Channel() {}
	//读写监听
	void enableRead(bool flag = true);

	void enableWrite(bool flag = true);

	void update();

	int getFd() { return _fd; }
	int getEvent() { return _event; }
	int getIndex() { return _index; }
	void setReadCb(const DefaultFunction & read_cb)
	{
		_read_cb = read_cb;
	}
	void setWriteCb(const DefaultFunction & write_cb)
	{
		_write_cb = write_cb;
	}
	void setErrorCb(const DefaultFunction & error_cb)
	{
		_error_cb = error_cb;
	}
	//////////////
	void setCloseCb(const DefaultFunction & close_cb)
	{
		_close_cb = close_cb;
	}
	////////////////
	void close()
	{
		enableRead(false);
		enableWrite(false);
		if(_fd >= 0)
		{
			::close(_fd);
			_fd = -1;
			_close_cb(); 
			std::cout << "close" << std::endl;
		}

	}
	void runRead() 
	{
		if(_read_cb)
		{
			std::cout << "start read cb" << std::endl;
			 //std::cout << std::endl<<strerror(errno) << "0--------" << "+++++"<<std::endl;
			_read_cb(); 
		}
	}
	void runWrite() 
	{
		if(_write_cb)
		{
			_write_cb();
		}
	}
	void runError() 
	{
		if(_error_cb)
		{
			_error_cb(); 
		}
	}
	void runClose()
	{
		if(_close_cb)
		{
			_close_cb();
		}
	}
private:
	int _fd;
	int _event; //可以探测的事件事件
	int _index; //纪录是否被epoll注册
	//int _revent; //具体的事件下标
	EventLoop* _loop;
	DefaultFunction _write_cb, _error_cb, _read_cb, _close_cb;
};

class Epoll
{
public:
	using Ptr = shared_ptr<Epoll>;
	Epoll()
	{
		_epoll_root = epoll_create1(EPOLL_CLOEXEC);
	}
	
	~Epoll();

	void addChannel(Channel* ch); //add channel的时候会把channel的文件描述符加入epoll树

	void removeChannel(Channel* ch);

	void updateChannel(Channel* ch);

	int size() const;
	
public:
	int _epoll_root;
	unordered_set<Channel*> _active_channel;
};



}
#endif
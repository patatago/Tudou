#ifndef __TUDOU_CHANNEL_H__
#define __TUDOU_CHANNEL_H__
#include "event.h"	
#include "../log/log.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unordered_set>
#include <vector>
#include <string.h>
#include <memory>
#include "epoll.h"
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
	bool isRead();
	bool isWrite();
	int getFd() { return _fd; }
	int getEvent() { return _event; }
	int getIndex() { return _index; }
	void setReadCb(const DefaultFunction & read_cb);
	void setWriteCb(const DefaultFunction & write_cb);
	void setErrorCb(const DefaultFunction & error_cb);
	//////////////
	void setCloseCb(const DefaultFunction & close_cb);
	////////////////
	void close();
	void runRead();
	void runWrite();
	void runError();
	void runClose();
private:
	int _fd;
	int _event; //可以探测的事件事件
	int _index; //纪录是否被epoll注册
	//int _revent; //具体的事件下标
	EventLoop* _loop;
	DefaultFunction _write_cb, _error_cb, _read_cb, _close_cb;
};




}
#endif
#ifndef __TUDOU_EPOLL_H__
#define __TUDOU_EPOLL_H__

#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unordered_set>
#include <vector>
#include <string.h>
#include <memory>
#include "channel.h"
#include "../log/log.h"
using std::unordered_set;
namespace tudou
{
class Channel;
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
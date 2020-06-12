#ifndef __TUDOU_EPOLL_H__
#define __TUDOU_EPOLL_H__

#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <set>
#include <vector>
#include <string.h>
#include "channel.h"
using namespace std;
namespace tudou
{
class Epoll
{
public:
	Epoll();
	
	~Epoll();
	
	void addChannel(Channel *ch); //add channel的时候会把channel的文件描述符加入epoll树
	void removeChannel(Channel *ch);
	void updateChannel(Channel *ch);
public:
	int _epoll_root;
	set<Channel *> _active_channel;
};
}

#endif
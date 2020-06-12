#include "epoll.h"
using namespace std;
namespace tudou
{
class Epoll
{
public:
	Epoll()
	{
		_epoll_root = epoll_create1(EPOLL_CLOEXEC);
	}
	
	~Epoll()
	{
		for(auto i = _active_channel.begin(); i!=_active_channel.end(); ++i)
		{
			(*i)->close();
		}
		close(_epoll_root);
	}
	
	void addChannel(Channel *ch) //add channel的时候会把channel的文件描述符加入epoll树
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.events = ch->getEvent();
		ev.data.ptr = ch;
		epoll_ctl(_epoll_root, EPOLL_CTL_ADD, ch->getFd(), &ev);
		_active_channel.emplace(ch);
		
	}
	void removeChannel(Channel *ch) 
	{
		_active_channel.erase(ch);
		epoll_ctl(_epoll_root, EPOLL_CTL_DEL, ch->getFd(), nullptr);
	}
	void updateChannel(Channel *ch)
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.events = ch->getEvent();
		ev.data.ptr = ch;
		epoll_ctl(_epoll_root, EPOLL_CTL_MOD, ch->getFd(), &ev);
	}
	
public:
	int _epoll_root;
	set<Channel *> _active_channel;
};
}

#endif
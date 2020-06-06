#ifndef __TUDOU_EVENT_H__
#define __TUDOU_EVENT_H__

#include "util.h"
#include "pipe.h"

#include "channel.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <list>
#include <memory>
using namespace std;
//eventloop --> channel --> request
namespace tudou
{
const int DefaultArraySize = 1024;

//事件loop，	
class Channel;
class Epoll;

class EventLoop
{

public:
	using Ptr = shared_ptr<EventLoop>;
	EventLoop(int taskSize = 0);
	void addChannel(Channel* ch);// { _epoll->addChannel(ch); }
	void modChannel(Channel* ch);// { _epoll->updateChannel(ch); }
	void delChannel(Channel* ch);// { _epoll->removeChannel(ch); }
	bool isExit() { return _is_exit; }
	~EventLoop()
	{
		if(_loop_thread)
		{
			delete _loop_thread;
		}
		pipeEvent();
	}
	
	bool runAsync(DefaultFunction task, bool sync)
	{
		std::cout << "pipr1" << std::endl;
		if(!task)
		{
			return false;
		}
		if(sync && _thread_id == this_thread::get_id()) //IO线程
		{
			std::cout << "pipr2" << std::endl;
			task();
			return true;
		}
		{
			lock_guard<mutex> mtx(_eventloop_mutex);
			_task_list.push_back(task); //加入任务队列
			std::cout << "pipr3" << std::endl;
			//std::cout << "push back" << _task_list.size() << std::endl;
		}
		wakeup(); //唤醒主线程，不再等待
		std::cout << "pipr4" << std::endl;
		return true;
		
	}
	EventLoop* get() { return this; }
	void pipeEvent();
	//static EventLoop& getInstance();
	
	int debu();
	void wakeup()
	{
		_pipe.write("", 1);
	}
	void update(Channel* ch);// { _epoll->updateChannel(ch); }
	void shutdown();
	
	void loop();

private:
	shared_ptr<Epoll> _epoll;
	mutex _looprun_mutex;
	mutex _eventloop_mutex;
	list<DefaultFunction> _task_list;
	atomic<bool> _is_exit;
	atomic<bool> _loop_run;
	PipeBase _pipe;
	atomic<int> _id;
	thread *_loop_thread;
	thread::id _thread_id;
	Channel::Ptr _pipe_channel;
//	vector<EventBase> _event_list; //多个eventbase
};



}

#endif
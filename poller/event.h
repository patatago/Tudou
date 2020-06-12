#ifndef __TUDOU_EVENT_H__
#define __TUDOU_EVENT_H__

#include "../util/util.h"
#include "pipe.h"

#include "channel.h"
#include <signal.h>
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
	void addChannel(Channel* ch);
	void modChannel(Channel* ch);
	void delChannel(Channel* ch);
	bool isExit();
	~EventLoop();
	
	bool runAsync(DefaultFunction task, bool sync);
	EventLoop* get();
	void pipeEvent();

	void wakeup();
	void update(Channel* ch);
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
	shared_ptr<Channel> _pipe_channel;
};



}

#endif
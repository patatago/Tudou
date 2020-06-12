#ifndef __TUDOU_EVENTLOOP_POOL_H__
#define __TUDOU_EVENTLOOP_POOL_H__
#include "../thread/threadgroup.h"
#include "event.h"
#include <functional>
using std::bind;
namespace tudou
{
	
class EventLoopThread
{
public:
	using Ptr = shared_ptr<EventLoopThread>;
	
	EventLoopThread();
	~EventLoopThread();
	
	//返回一个eventloop对象
	EventLoop::Ptr start();
	
	void threadFunc();
private:
	ThreadGroup _thread; //eventloop线程
	mutex _mutex;
	EventLoop::Ptr _loop; //返回一个eventloop对象
	condition_variable _condition;
};	
		
	
	

class EventLoopPool
{
private:
	EventLoopPool(EventLoop::Ptr ptr, size_t threadnum);
public:
	~EventLoopPool();
	
	EventLoop::Ptr choose();

	static EventLoopPool &getInstance(EventLoop::Ptr ptr = nullptr, size_t threadnum = 0);
private:
	size_t _thread_num; //线程个数
	vector<EventLoop::Ptr> _loop; //eventloop列表
	vector<EventLoopThread::Ptr> _thread; //线程列表
	EventLoop::Ptr _main_eventloop;
	size_t _loop_pos; //选择哪个eventloop
private:
	static unique_ptr<EventLoopPool> _pInstance;
};


}

#endif
#ifndef __TUDOU_EVENTLOOP_POOL_H__
#define __TUDOU_EVENTLOOP_POOL_H__
#include "threadgroup.h"
#include "event.h"
#include <functional>
using std::bind;
namespace tudou
{
	
class EventLoopThread
{
public:
	using Ptr = shared_ptr<EventLoopThread>;
	
	EventLoopThread()
	: _loop(nullptr)
	{}
	
	~EventLoopThread()
	{
		_thread.joinAll(); //线程回收
		_loop->shutdown();
	}
	
	//返回一个eventloop对象
	EventLoop::Ptr start()
	{
		_thread.createThread(bind(&EventLoopThread::threadFunc, this)); //绑定回调函数
		
		unique_lock<mutex> mtx(_mutex);
		if(_loop == nullptr) //io线程还没有吧eventloop准备好
		{
			_condition.wait(mtx);
		}
		return _loop; //此时eventloop准备好了
	}
	
	void threadFunc()
	{
		EventLoop::Ptr loop = make_shared<EventLoop>();
		{
			lock_guard<mutex> mtx(_mutex);
			_loop = loop;
			_condition.notify_one();
		}
		_loop->loop(); //开始监听
	}
private:
	ThreadGroup _thread; //eventloop线程
	mutex _mutex;
	EventLoop::Ptr _loop; //返回一个eventloop对象
	condition_variable _condition;
};	
		
	
	

class EventLoopPool
{
private:
	EventLoopPool(EventLoop* ptr, size_t threadnum)
	: _main_eventloop(ptr)
	, _thread_num(threadnum)
	, _loop_pos(0)
	{
		for(size_t i=0; i<threadnum; ++i)
		{
			EventLoopThread::Ptr ptr(new EventLoopThread());
			_thread.emplace_back(ptr);
			_loop.emplace_back(ptr->start());
		}
	}
public:
	~EventLoopPool() {}
	
	EventLoop* choose()
	{
		if(!_loop.size())	
		{ 
			return _main_eventloop.get(); 
		} //此时不使用one loop per thread模式
		auto ret = _loop[_loop_pos];
		_loop_pos = (_loop_pos == _loop.size()-1) ? 0 : (_loop_pos + 1);
		//返回一个eventloop对象
		return ret.get();
	}

	static EventLoopPool &getInstance(EventLoop* ptr = nullptr, size_t threadnum = 0)
	{
		if(_pInstance == nullptr)
		{
			mutex _mutex;
			lock_guard<mutex> mtx(_mutex);
			if(_pInstance == nullptr)
			{
				_pInstance.reset(new EventLoopPool(ptr, threadnum));
			}
		}
		return *_pInstance;
	}
private:
	size_t _thread_num; //线程个数
	vector<EventLoop::Ptr> _loop; //eventloop列表
	vector<EventLoopThread::Ptr> _thread; //线程列表
	EventLoop* _main_eventloop;
	size_t _loop_pos; //选择哪个eventloop
private:
	static unique_ptr<EventLoopPool> _pInstance;
};

unique_ptr<EventLoopPool> EventLoopPool::_pInstance = nullptr;
}

#endif
#ifndef __TUDOU_THREADPOOL_H__
#define __TUDOU_THREADPOOL_H__

#include <iostream>
#include <unistd.h>
#include "threadgroup.h"
#include "taskqueue.h"

using std::bind;

namespace tudou
{
class ThreadPool
{
public:
	ThreadPool(size_t thread_num=10, size_t task_num=0)
	: _task_queue(task_num)
	, _thread_num(thread_num) //线程个数
	, _is_exit(false)
	{
        if(_thread_num <= 0)
            return ;
        for(size_t i=0; i<_thread_num; ++i)
        {
            _thread_group.createThread(bind(&ThreadPool::run, this));
        }
	}

	void run()
	{
		while(1)
		{
			DefaultFunction task = nullptr;
			task = _task_queue.pop();
			if(task)
			{
				try
				{
					task();
				}
				catch(std::exception &ex)
				{
					std::cerr << ex.what() << std::endl;
				}
			}
			
		}
	
		//_task_queue.pop();
		
	}
	
public:
	bool addTask(DefaultFunction task)
	{	
		return _task_queue.push(task);
	}
	
	void shutdown()
	{
		_task_queue.exit();
		_thread_group.joinAll();
		_is_exit = true;
	}
	
	~ThreadPool()
	{
		if(!_is_exit)
		{
			shutdown();
		}
	}
private:
	size_t _thread_num;
	TaskQueue _task_queue; //任务
	atomic<bool> _is_exit;
	ThreadGroup _thread_group; //线程组
};
}

#endif
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
	ThreadPool(size_t thread_num=10, size_t task_num=0);

	void run();
public:
	bool addTask(DefaultFunction task);
	
	void shutdown();
	~ThreadPool();
private:
	size_t _thread_num;
	TaskQueue _task_queue; //任务
	atomic<bool> _is_exit;
	ThreadGroup _thread_group; //线程组
};
}

#endif
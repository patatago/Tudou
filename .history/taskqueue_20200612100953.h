#ifndef __TUDOU_TASKQUEUE_H__
#define __TUDOU_TASKQUEUE_H__


#include <memory>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include "util.h"
using std::mutex;
using std::lock_guard;
using std::bind;

using namespace std;
namespace tudou
{
class TaskQueue
: noncopyable
{
public:
	TaskQueue(size_t capacity=0); //0代表不限制容量

	
	bool push(DefaultFunction task);
	
	DefaultFunction pop(); 
	size_t taskQueueSize();  //返回task列表大小

	bool isExit();
	void exit();
	
	~TaskQueue();
	
private:
	std::list<DefaultFunction> _task_queue; //任务队列列表
	size_t _capacity;   //最大容量
	mutex _mutex;
	std::condition_variable_any _condition; //条件变量
	std::atomic<bool> _is_exit;
	
};
}

//mutex TaskQueue::_mutex;


#endif
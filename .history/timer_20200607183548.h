#ifndef __TUDOU_TIMER_H__
#define __TUDOU_TIMER_H__
#include <memory>
#include <mutex>
#include <time.h>
#include <queue>
#include "threadgroup.h"
#include <vector>
#include <chrono>
#include <netinet/in.h>
#include <functional>
#include <condition_variable>
#include "util.h"

#include "tcpconnection.h"
//使用时间堆实现
// std priority_queue
using std::condition_variable;
using std::priority_queue;
using std::vector;
namespace tudou
{
#define DEFAULT_TIME 5
class TcpConnection;
class Timer
{
	friend class TimerManager;
public:
	using Ptr = shared_ptr<Timer>;
	Timer(uint64_t timeout, function<bool()> cb, const shared_ptr<TcpConnection> client = nullptr);
	
	uint64_t getDeadline() const;
	
	bool reset(); //重置时间
	
	void delTimer();

	~Timer();
private:
	bool _deleted;
	uint64_t _timeout; //延迟时间
	uint64_t _deadline; //生效截止时间
	function<bool()> _cb; //回调函数,true表示循环，false表示不循环
	shared_ptr<TcpConnection> _client; //客户端信息
};


struct SetCompare
{
	bool operator()(const Timer::Ptr &x, const Timer::Ptr &y)
	{
		return x->getDeadline() < y->getDeadline();
	}
};

class TimerManager
{
public:
	void addTimer(Timer::Ptr &timer);
	
	Timer::Ptr top() const;
	void pop(); 
	//心跳函数
	void tick();
	
	static TimerManager& getInstance(int waitTime=0);
	
	~TimerManager(); 

private:
	TimerManager();
private:
	priority_queue<Timer::Ptr, vector<Timer::Ptr>, SetCompare> _time_heap;
	ThreadGroup _thread;
	mutex _mutex;
	atomic<bool> _is_exit;
	static unique_ptr<TimerManager> _pInstance;
	condition_variable _condition;
};
}

#endif
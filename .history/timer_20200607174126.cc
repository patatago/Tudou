#include "timer.h"

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


Timer::Timer(uint64_t timeout, function<bool()> cb, const shared_ptr<TcpConnection> client = nullptr)
: _cb(cb)
, _client(client)
, _deleted(false)
{
    _deadline = getMsec() + timeout;
    _timeout = timeout;
}
uint64_t Timer::getDeadline() const { return _deadline; }

bool Timer::reset() //重置时间
{
    _deadline = getMsec() + _timeout;	
}
void Timer::delTimer()
{
    _deleted = true;
}
Timer::~Timer() {}


void TimerManager::addTimer(Timer::Ptr &timer)
{
    if(!timer)
    {
        return ;
    }
    _time_heap.push(timer);
    timer->_client->setTimer(timer);  //每个tcpconnection拥有一个timer
}



Timer::Ptr TimerManager::top() const { return _time_heap.top(); }

TimerManage::TimerManager()
: _is_exit(false)
{
    _thread.createThread(bind(&TimerManager::tick, this));
}

void TimerManager::pop() { _time_heap.pop(); }
//心跳函数
void TimerManager::tick()
{
    bool recurring = false;
    
    while(!_is_exit)
    {
        unique_lock<mutex> mtx(_mutex);
        if(_time_heap.size())
        {
            _condition.wait_for(mtx, std::chrono::milliseconds(DEFAULT_TIME));
        }
        else //暂时还没有延时任务就阻塞等待一会
        {
            _condition.wait_for(mtx, std::chrono::minutes(DEFAULT_TIME));
        }
        auto cur = getMsec(); //获取当前时间
        //如果此时有延迟任务
        while(_time_heap.size())
        {
            if(_time_heap.top()->_deleted) //已删除
            {
                _time_heap.pop();
            }
            //还没到时间
            if(cur < _time_heap.top()->_deadline)
            {
                break;
            }
            
            //有回调函数
            if(_time_heap.top()->_cb)
            {
                recurring = _time_heap.top()->_cb();
            }

            if(recurring) //这是重复任务
            {
                _time_heap.top()-> reset();//重置时间
            }
            else
            {
                _time_heap.pop(); //执行完就pop出来
            }
        }
        
    }	
}


static TimerManager& TimerManager::getInstance(int waitTime=0)
{
    mutex _mutex;
    if(_pInstance == nullptr)
    {
        lock_guard<mutex> lck(_mutex);
        if(_pInstance == nullptr)
        {
            _pInstance.reset(new TimerManager());
        }
    }
    return *_pInstance.get();
}

TimerManager::~TimerManager() 
{ 
    _is_exit = true;
    _thread.joinAll(); 


std::unique_ptr<TimerManager> 
TimerManager::_pInstance = nullptr;

}
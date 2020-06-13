#include "taskqueue.h"
using namespace tudou;


TaskQueue::TaskQueue(size_t capacity) //0代表不限制容量
: _capacity(capacity)
, _is_exit(false) //还没有退出
{}

bool TaskQueue::push(DefaultFunction task)
{
    std::lock_guard<mutex> mtx(_mutex);
    if(_is_exit)	
    {
        return false;
    }
    if(_capacity && _capacity <= _task_queue.size())
    {
        return false; //队列满
    }
    _task_queue.emplace_back(std::move(task)); //加入任务队列
    
    _condition.notify_one();
    return true;
}

DefaultFunction TaskQueue::pop() 
{
    std::unique_lock<mutex> mtx(_mutex);
    if(_task_queue.size() == 0)
    {
        _condition.wait(mtx); //等待通知
        if(_is_exit)
        {
            return nullptr;
        }
    }
    if(_task_queue.size())
    {
        auto temp = _task_queue.front();
        
        _task_queue.pop_front();
        return temp;
    }
    return nullptr;
    
    
}
size_t TaskQueue::taskQueueSize()  //返回task列表大小
{
    std::lock_guard<mutex> mtx(_mutex);
    return _task_queue.size(); 
}
bool TaskQueue::isExit() 
{ 
    return _is_exit; 
}
void TaskQueue::exit()
{
    _is_exit = true;
    //std::lock_guard<mutex> mtx(_mutex);
    _condition.notify_all(); //让所有线程唤醒
}

TaskQueue::~TaskQueue()
{
    _task_queue.clear();
}


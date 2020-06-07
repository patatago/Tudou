#include "eventlooppool.h"

using namespace tudou;
EventLoopThread::EventLoopThread()
: _loop(nullptr)
{}

EventLoopThread::~EventLoopThread()
{
    _thread.joinAll(); //线程回收
    _loop->shutdown();
}

//返回一个eventloop对象
EventLoop::Ptr EventLoopThread::start()
{
    _thread.createThread(bind(&EventLoopThread::threadFunc, this)); //绑定回调函数
    
    unique_lock<mutex> mtx(_mutex);
    if(_loop == nullptr) //io线程还没有吧eventloop准备好
    {
        _condition.wait(mtx);
    }
    return _loop; //此时eventloop准备好了
}

void EventLoopThread::threadFunc()
{
    EventLoop::Ptr loop = make_shared<EventLoop>();
    {
        lock_guard<mutex> mtx(_mutex);
        _loop = loop;
        _condition.notify_one();
    }
    _loop->loop(); //开始监听
}




EventLoopPool::EventLoopPool(EventLoop::Ptr ptr, size_t threadnum)
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

EventLoopPool::~EventLoopPool() {}

EventLoop::Ptr EventLoopPool::choose()
{
    if(!_loop.size())	
    { 
        return _main_eventloop; 
    } //此时不使用one loop per thread模式
    auto ret = _loop[_loop_pos];
    _loop_pos = (_loop_pos == _loop.size()-1) ? 0 : (_loop_pos + 1);
    //返回一个eventloop对象
    return ret;
}

EventLoopPool &EventLoopPool::getInstance(EventLoop::Ptr ptr, size_t threadnum)
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


unique_ptr<EventLoopPool> EventLoopPool::_pInstance = nullptr;

#include "threadpool.h"
using namespace tudou;


ThreadPool::ThreadPool(size_t thread_num, size_t task_num)
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

void ThreadPool::run()
{
    while(!_is_exit)
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
                Warn << ex.what() << std::endl;
                std::cerr << ex.what() << std::endl;
            }
        }
        
    }

    //_task_queue.pop();
    
}

bool ThreadPool::addTask(DefaultFunction task)
{	
    return _task_queue.push(task);
}

void ThreadPool::shutdown()
{
    _is_exit = true;
    _task_queue.exit();
    _thread_group.joinAll();    
}

ThreadPool::~ThreadPool()
{
    if(!_is_exit)
    {
        shutdown();
    }
}

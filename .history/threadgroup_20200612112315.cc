#include "threadgroup.h"
using namespace tudou;

ThreadGroup::ThreadGroup(){}

ThreadGroup::~ThreadGroup(){ joinAll(); }

template <typename Type>
thread* ThreadGroup::createThread(Type && func)
{
    auto _temp_thread = std::make_shared<thread>(func);
    _thread_id = _temp_thread->get_id();
    //std::cout << "group" << _thread_id <<std::endl;
    _threads[_thread_id] = _temp_thread;
    return _temp_thread.get();
}

bool ThreadGroup::isThisThreadIn()
{
    auto _id = this_thread::get_id();
    
    if(_threads.find(_id) == _threads.end())	
    {
        return false;
    }
    return true;
}

void ThreadGroup::joinAll()
{
    if(isThisThreadIn())
    {
        throw runtime_error("threadgroup: join itself");
    }
    for(auto &i : _threads)
    {
        if(i.second->joinable())
        {
            i.second->join();
        }
    }
    _threads.clear();
    
}



#include "threadgroup.h"
using namespace tudou;

ThreadGroup::ThreadGroup(){}

ThreadGroup::~ThreadGroup(){ joinAll(); }



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
        return ;
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



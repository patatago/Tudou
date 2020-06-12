#include "blockqueue.h"

using namespace tudou;
	
BlockQueue::BlockQueue(unsigned int size)
: _max_queue_size(size)
{}

void BlockQueue::clear() 
{
    lock_guard<mutex> lock(_mutex_block_queue);
    _block_queue.clear();
}

bool BlockQueue::full() const
{
    lock_guard<mutex> lock(_mutex_block_queue);
    if(_block_queue.size() >= _max_queue_size)
    {
        return true;
    }
    return false;
}

bool BlockQueue::empty() const
{
    lock_guard<mutex> lock(_mutex_block_queue);
    if(_block_queue.size())
    {
        return false;
    }
    return true;
}

bool BlockQueue::front(LogPair &log) const
{
    lock_guard<mutex> lock(_mutex_block_queue);
    bool flag = empty();
    if(flag)
    {
        return false;
    }
    log = _block_queue.front();
    return true;
}

bool BlockQueue::back(LogPair & log) const
{
    lock_guard<mutex> lock(_mutex_block_queue);
    bool flag = empty();
    if(flag)
    {
        return false;
    }
    log = _block_queue.back();
    return true;
}

int BlockQueue::size() const
{
    lock_guard<mutex> lock(_mutex_block_queue);
    int temp = _block_queue.size();
    return temp;
}

int BlockQueue::maxSize()
{
    lock_guard<mutex> lck(_mutex_block_queue);
    int temp = _max_queue_size;
    return temp;
}

bool BlockQueue::push(const LogPair &recv)
{

    int temp = size();
    lock_guard<mutex> lock(_mutex_block_queue);
        
    if(temp >= _max_queue_size)
    {
        _condition.notify_all();
        return false;
    }
    _block_queue.push_back(recv);
    _condition.notify_all();
    return true;
    
}
bool BlockQueue::pop(LogPair &log) 
{
    unique_lock<mutex> lock(_mutex_block_queue);
    while(_block_queue.size() <= 0)
    {
        _condition.wait(lock, [&, this](){return !empty();});
        
    }
    log = _block_queue.front();
    _block_queue.pop_front();
    return true;
}

bool BlockQueue::pop(LogPair &log, int timeSecond) 
{
    
    int temp = size();
    unique_lock<mutex> lock(_mutex_block_queue);
    if(!_condition.wait_for(lock, std::chrono::seconds(timeSecond), [&, temp](){return temp>0;}) )
    {
        
        return false;
    }
    if(_block_queue.size() <=0 )
    {
        return false;
    }
    log = _block_queue.front();
    _block_queue.pop_front();
    return true;
}

BlockQueue::~BlockQueue()
{
    if(_block_queue.size())
    {
        _condition.notify_all();
    }
}


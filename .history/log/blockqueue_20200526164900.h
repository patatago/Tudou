#ifndef __TUDOU_BLOCKQUEUE_H__
#define __TUDOU_BLOCKQUEUE_H__

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <list>
using namespace std;
namespace tudou
{

class BlockQueue
{
public:
	using LogPair = pair<string, string>; 
    explicit BlockQueue(unsigned int size=1000)
    : _max_queue_size(size)
    {}

    void clear() 
    {
        lock_guard<mutex> lock(_mutex_block_queue);
        _block_queue.clear();
    }

    bool full() const
    {
        lock_guard<mutex> lock(_mutex_block_queue);
        if(_block_queue.size() >= _max_queue_size)
        {
            return true;
        }
        return false;
    }

    bool empty() const
    {
        lock_guard<mutex> lock(_mutex_block_queue);
        if(_block_queue.size())
        {
            return false;
        }
        return true;
    }

    bool front(LogPair &log) const
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

    bool back(LogPair & log) const
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

    int size() const
    {
        lock_guard<mutex> lock(_mutex_block_queue);
        int temp = _block_queue.size();
        return temp;
    }

    int maxSize()
    {
        lock_guard<mutex> lck(_mutex_block_queue);
        int temp = _max_queue_size;
        return temp;
    }

    bool push(const LogPair &recv)
    {
	
		int temp = size();
        lock_guard<mutex> lock(_mutex_block_queue);
           
        if(temp >= _max_queue_size)
		{
			_condition.notify_all();
			return false;
		}
	//	pair<string, string> ff = log;
        _block_queue.push_back(recv);
		//std::cout<<log.second<<std::endl;
		_condition.notify_all();
		return true;
    	
	}
	bool pop(LogPair &log) 
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
	
	bool pop(LogPair &log, int timeSecond) 
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
	
	~BlockQueue()
	{
		if(_block_queue.size())
		{
			_condition.notify_all();
		}
	}


private:
    mutable std::mutex _mutex_block_queue;
    std::list<pair<string, string>> _block_queue;
	condition_variable_any _condition;
    unsigned int _max_queue_size;
};

}
#endif
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
    explicit BlockQueue(unsigned int size=1000);
    void clear();
    bool full() const;

    bool empty() const;

    bool front(LogPair &log) const;

    bool back(LogPair & log) const;

    int size() const;

    int maxSize();

    bool push(const LogPair &recv);
	bool pop(LogPair &log);
	
	bool pop(LogPair &log, int timeSecond);
	
	~BlockQueue();


private:
    mutable std::mutex _mutex_block_queue;
    std::list<pair<string, string>> _block_queue;
	condition_variable_any _condition;
    unsigned int _max_queue_size;
};

}
#endif
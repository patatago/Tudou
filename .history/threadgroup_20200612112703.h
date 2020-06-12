#ifndef __TUDOU_THREADGROUP_H__
#define __TUDOU_THREADGROUP_H__

#include <mutex>
#include <unordered_map>
#include <thread>
#include <vector>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <vector>
#include <memory>
using std::mutex;
using std::condition_variable;
using namespace std;
namespace tudou
{
class ThreadGroup
{
public:
	ThreadGroup();
	~ThreadGroup();
	
	
	template <typename Type>
thread* createThread(Type && func)
{
    auto _temp_thread = std::make_shared<thread>(func);
    _thread_id = _temp_thread->get_id();
    //std::cout << "group" << _thread_id <<std::endl;
    _threads[_thread_id] = _temp_thread;
    return _temp_thread.get();
}
	
	bool isThisThreadIn();
	void joinAll();

private:
	unordered_map<thread::id, std::shared_ptr<thread>> _threads;
	thread::id _thread_id;

};
}
#endif
//#include "epoll.h"
//#include "channel.h"
//#include "event.h"
//#include "time.h"
#include <iostream>
//#include "util.h"
//#include "threadgroup.h"
#include <unordered_set>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <memory>
#include <condition_variable>
#include <list>
#include "event.h"
#include "time.h"
using namespace std;
using namespace tudou;



int main()
{
	//Timer c;
	Timer c;
	EventLoop::getInstance().run();
	c.add(2000, 
		[]()->bool
		{
			EventLoop::getInstance().runAsync
			(
				[](){	//sleep(1);
					std::cout << "hello" << std::endl;
					
					auto usec = rand() % 5000;	usleep(usec);	}
				, true
			);
			return true;
		
		}
	);
	while(1);
}
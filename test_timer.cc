#include "timer.h"
#include <iostream>
using namespace std;
using namespace tudou;


int main()
{
    Timer::Ptr ptr(new Timer(1000, [](){ cout <<"hello"<<endl; return true; }));
    TimerManager::getInstance().addTimer(ptr);
	//sleep(10);
	//ptr->delTimer();
}

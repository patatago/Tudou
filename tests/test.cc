#include "event.h"
#include <iostream>
#include <string>
using namespace std;
using namespace tudou;

int main()
{
    EventLoop::Ptr loop(new EventLoop());
	loop->runAsync([](){cout << "hello" << std::endl;}, false);
	loop->loop();
}

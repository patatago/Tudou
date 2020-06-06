#include "event.h"
#include <iostream>
#include <string>
using namespace std;
using namespace tudou;

int main()
{
    EventLoop::Ptr loop();
	loop->runAsync([](){cout <, "hello" << std::endl;}, false)
}

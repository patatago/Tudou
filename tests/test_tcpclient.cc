#include "../net/tcpconnection.h"
#include "../log/log.h"
#include <iostream>
using namespace std;
using namespace tudou;

int main()
{
    Loglevel(LogLevel::TRACE);
    EventLoop::Ptr loop(new EventLoop());
    // EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    auto client = TcpClient::start(loop, "127.0.0.1", 8080, 2000); //返回tcpconnecion对象
    // //connect返回后如果触发epollout然后调用handlehandshake等待接受
    // if(client == nullptr) throw runtime_error("client start error");
    //loop->loop();
}
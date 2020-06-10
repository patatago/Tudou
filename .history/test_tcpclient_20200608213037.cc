#include "tcpconnection.h"
#include <iostream>
using namespace std;
using namespace tudou;

int main()
{
    EventLoop::Ptr loop(new EventLoop());
    auto client = TcpClient::start(loop, "140.143.226.113", 8080, 2000)
    // client = TcpConnection::connection(loop, "140.143.226.113", 8080, 2000, nullptr);
    if(client == nullptr) throw runtime_error("client start error");
    loop->loop();
}
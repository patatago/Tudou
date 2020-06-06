#include "tcpconnection.h"
#include <iostream>
#include <string>
using namespace std;
using namespace tudou;

int main()
{
    make_shared<EventLoop> base = make_shared<EventLoop>();
    TcpServer::Ptr ptr = nullptr;
	ptr = TcpServer(&base).start(&base, "127.0.0.1", 8080);
    if(ptr)
	{
		cout << "ok" <<endl;
	}
	else
	
	cout << "not ok" << endl;
    ptr->setConnectionCb([](const TcpConnection::Ptr &con){
        std::cout << "welocme to tudou" <<std::endl;
    } );
	base.loop();
}

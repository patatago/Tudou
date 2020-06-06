#include "tcpconnection.h"
#include <unistd.h>
#include <thread>
using namespace tudou;
using namespace std;
/*
int main()
{
    auto base = new EventLoop();
    //auto base = make_shared<EventLoop>();
    TcpServer::Ptr ptr = nullptr;
	ptr = TcpServer(base).start(base, "127.0.0.1", 8080);
    if(ptr)
	{
		cout << "ok" <<endl;
	}
	else
	
	cout << "not ok" << endl;
    ptr->setConnectionCb([](const TcpConnection::Ptr &con){
        std::cout << "welocme to tudou" <<std::endl;
    } );
	base->loop();
}


*/
int main()
{
    auto base = new EventLoop();
    //EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    //TudouServer::Ptr serv(new TudouServer(base, "127.0.0.1", 8080, 1, 0)); //2个线程，使用——main——loop

    TcpServer::Ptr ptr = TcpServer(base).start(base, "127.0.0.1", 8080);
    //EventLoopPool::getInstance(base, 0);
    ptr->setConnectionCb([](const TcpConnection::Ptr &con){
			std::cout << "welocme to tudou" <<std::endl;
	     } );
    /*
    ptr->setReadCb([](const TcpConnection::Ptr &con){
        std::cout << "welocme to read" <<std::endl;
        } );
    */
    //serv->start();

    //auto _temp_thread = std::make_shared<thread>([&](){sleep(5); serv->send("hello world");});
    base->loop();  //开始监听客户端连接


}
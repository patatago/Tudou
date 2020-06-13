#include "../net/tcpconnection.h"
#include "../signal.h"
#include <unistd.h>
#include <thread>
using namespace tudou;
using namespace std;

int main()
{
    EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    TudouServer::Ptr serv(new TudouServer(_main_loop, "127.0.0.1", 8080, 4, 2)); //2个线程，使用——main——loop
    serv->setConnectionCb([](const TcpConnection::Ptr &con){
			std::cout << "welocme to tudou" <<std::endl;
	     } );

    // serv->setReadCb([](const TcpConnection::Ptr &con){
    //     std::cout << "welocme to read" <<std::endl;
    //     } );
    Signal::httpClose(serv); //设置关闭
    serv->start();
    _main_loop->loop();  //开始监听客户端连接
      
}
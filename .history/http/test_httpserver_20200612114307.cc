#include "httpserver.h"
#include <iostream>
#include <errno.h>
#include <signal.h>
#include <string.h>
using namespace std;
using namespace tudou;

int main()
{
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &act, NULL) != 0)
        exit(1);
    EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    HttpServer::Ptr serv(new HttpServer(_main_loop, "0.0.0.0", 8000, 2, 2)); //2个线程，使用——main——loop
    // serv->setConnectionCb([](const TcpConnection::Ptr &con){
	// 		std::cout << "welocme to tudou" <<std::endl;
	//      } );
    
    serv->start();

    _main_loop->loop();  //开始监听客户端连接
 
}
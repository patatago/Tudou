#include "tcpconnection.h"
#include <unistd.h>
#include <thread>
using namespace tudou;
using namespace std;


int main()
{
    EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    TudouServer::Ptr serv(new TudouServer(_main_loop, "127.0.0.1", 8081, 2, 0)); //2个线程，使用——main——loop
    serv->setConnectionCb([](const TcpConnection::Ptr &con){
			std::cout << "welocme to tudou" <<std::endl;
	     } );
    /*
    serv->setWriteCb([](const TcpConnection::Ptr &con){
        std::cout << "welocme to write" <<std::endl;
        } );
    */
    /*
    serv->setReadCb([](const TcpConnection::Ptr &con){
        std::cout << "welocme to read" <<std::endl;
        } );
    */
    serv->start();
    //thread _thread(func);

    //auto _temp_thread = std::make_shared<thread>([&](){sleep(5); serv->send("hello world");});
    _main_loop->loop();  //开始监听客户端连接
      
  
    while(1) ;
    

}
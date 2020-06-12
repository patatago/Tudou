#include "httpserver.h"
#include <iostream>
#include <errno.h>
#include "../log/log.h"
#include <signal.h>
#include <string.h>
using namespace std;
using namespace tudou;

int main()
{
    Loglevel(LogLevel::TRACE);
    ::signal(SIGPIPE, SIG_IGN);
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0)
    {
       printf("block sigpipe error\n");
    } 
    EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    HttpServer::Ptr serv(new HttpServer(_main_loop, "0.0.0.0", 8000, 2, 2)); //2个线程，使用——main——loop
    // serv->setConnectionCb([](const TcpConnection::Ptr &con){
	// 		std::cout << "welocme to tudou" <<std::endl;
	//      } );
    
    serv->start();

    _main_loop->loop();  //开始监听客户端连接

    // class Signal
    // : public noncopyable 
    // {
    //     static void IgnoreSigpipe()
    //     {
    //         sigset_t signal_mask;
    //         sigemptyset(&signal_mask);
    //         sigaddset(&signal_mask, SIGPIPE); //屏幕sigpipe
    //         int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    //         if (rc != 0)
    //         {
    //             printf("block sigpipe error\n");
    //         } 
    //     }

    //     static void addSignal(int signal, DefaultFunction & func)
    //     {
    //         //sigaction(signal, func);
    //     }

    //    // static void addSignal(TcpServer::ptr ptr)
    // }

 
}
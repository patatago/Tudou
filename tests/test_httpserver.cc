#include "httpserver.h"
#include <iostream>
#include <errno.h>
#include "../log/log.h"
#include <signal.h>
#include "../util/signal.h"
#include <string.h>
using namespace std;
using namespace tudou;

int main()
{
    Loglevel(LogLevel::FATAL);
    Signal::IgnoreSigpipe();
    EventLoop::Ptr _main_loop(new EventLoop()); //main_loop-->接受请求
    HttpServer::Ptr serv(new HttpServer(_main_loop, "0.0.0.0", 8000, 2, 2)); //2个线程，使用——main——loop
    serv->start();
    Signal::httpClose(serv); //设置关闭

    _main_loop->loop();  //开始监听客户端连接

    
   
 
}
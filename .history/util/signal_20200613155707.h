#ifndef __TUDOU_SIGNAL_H__
#define __TUDOU_SIGNAL_H__

#include <signal.h>
#include "util.h"
#include "../log/log.h"
#include "../http/httpserver.h"
namespace tudou
{
class Signal
: public noncopyable 
{
public: 
    //屏蔽sigpipe信号
    static void IgnoreSigpipe()
    {
        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGPIPE); //屏幕sigpipe
        int ret = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
        if(ret != 0)
        {
            Warn << "sigpipe mask error" << std::endl;
        } 
    }

    static void httpClose(HttpServer::Ptr &server)
    {
        _http_server = server;
        signal(SIGINT, httpHandler);
    }
private:
    static void httpHandler(int sig)
    {
        EventLoopPool::getInstance().shutdown();
        Signal::_http_server->shutdown();  //HttpServer::Ptr
        Log::getInstance().shutdown();
        std::cout << "GoodBye...." << std::endl;
        return ;
    }

    static void tcpHandler(int sig)
    {
        EventLoopPool::getInstance().shutdown();
        Signal::_tcp_server->shutdown();  //HttpServer::Ptr
        Log::getInstance().shutdown();
        std::cout << "GoodBye...." << std::endl;
        return ;
    }
private:
    static HttpServer::Ptr _http_server;
    static TudouServer::ptr _tcp_server;
};

HttpServer::Ptr Signal::_http_server = nullptr;
}
#endif




#include "httpserver.h"
using namespace tudou;

HttpServer::HttpServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum)
: _main_loop(loop)
, _thread_pool(threadnum)
, _event_pool(EventLoopPool::getInstance(loop, eventnum))
, _host(host)
, _port(port)
, _server(nullptr)
{}
void HttpServer::readCb(const TcpConnection::Ptr & ptr)
{
    auto _in = ptr->getInput();	//拿到http请求
    HttpParse _parse;
    auto flag = _parse.parse(*_in);
    Buffer out;
    HttpResponse to(_parse.getHttpMessage(), "tudou", out);
    to.append();
    ptr->send(out);
    if(!flag)
    {
        Debug << "HTTP parse error" << std::endl;
        _in->reset();
    }
    
}

void HttpServer::start() //最后一个参数是线程池所创建的线程
{
    TcpServer _temp_server(_main_loop);
    
    _temp_server.setReadCb([=](const TcpConnection::Ptr &con){readCb(con); });
    _server = _temp_server.start(_main_loop, _host, _port); //启动监听
}

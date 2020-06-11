#ifndef __TUDOU_HTTPSERVER_H__
#define __TUDOU_HTTPSERVER_H__

#include "httprequest.h"
#include "../tcpconnection.h"
#include "../buffer.h"

namespace tudou
{
class HttpServer    
{
public:
    using Ptr = shared_ptr<HttpServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
    HttpServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum)
	: _main_loop(loop)
	, _thread_pool(threadnum)
	, _event_pool(EventLoopPool::getInstance(loop, eventnum))
	, _host(host)
	, _port(port)
	, _server(nullptr)
	{}
	
void readCb(const TcpConnection::Ptr & ptr)
{
	//shared_ptr<TcpConnection> // _read_cb(ptr)
	auto _in = ptr->getInput();	//拿到http请求
	HttpParse _parse;
	// std::cout <<"------------------"<<std::endl;
	// std::cout << _in->begin() << std::endl;
	// std::cout << "------------------" << std::endl;
	auto flag = _parse.parse(*_in);
	
	auto out = ptr->getOutput();
	HttpResponse to(_parse.getHttpMessage(), "tudou", *out);
	//_parse.getHttpMessage().print();
	to.append(ptr);
	//
	ptr->send(*out);
	ptr->getChannel()->close();
	//to.append();
}

	void start() //最后一个参数是线程池所创建的线程
	{
		TcpServer _temp_server(_main_loop);
		
		_temp_server.setReadCb([=](const TcpConnection::Ptr &con){readCb(con); });
		_server = _temp_server.start(_main_loop, _host, _port); //启动监听
	}

private:
    const string _host;
	uint16_t _port;
	size_t _threadnum;
	size_t _eventnum;
	
	Buffer *_in, *_out;
	MessageCallback _read_cb, _write_cb, _close_cb, _connection_cb;
	EventLoop::Ptr _main_loop;
	ErrorCallback _error_cb;
	EventLoopPool _event_pool;
	TcpServer::Ptr _server;
	ThreadPool _thread_pool;
};
}
#endif
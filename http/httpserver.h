#ifndef __TUDOU_HTTPSERVER_H__
#define __TUDOU_HTTPSERVER_H__

#include "httprequest.h"
#include "../net/tcpconnection.h"
#include "../util/buffer.h"
#include "../log/log.h"

namespace tudou
{
class HttpServer    
{
public:
    using Ptr = shared_ptr<HttpServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
    HttpServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum);
	void readCb(const TcpConnection::Ptr & ptr);

	void start();
	

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
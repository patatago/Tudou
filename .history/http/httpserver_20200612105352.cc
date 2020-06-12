
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
		// if(!ptr->getOutput()->size())
		// 	ptr->getChannel()->close();
		if(!flag)
		{
			_in->reset();
		}
		
	}

	void HttpServer::start() //最后一个参数是线程池所创建的线程
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
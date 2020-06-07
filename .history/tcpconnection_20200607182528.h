#ifndef __TUDOU_TCPCONNECTION_H__
#define __TUDOU_TCPCONNECTION_H__

#include "event.h"
#include "error.h"
#include "util.h"
#include "socket.h"
#include "buffer.h"
#include "threadpool.h"
#include "eventlooppool.h"
#include <string.h>
#include "timer.h"
#include <iostream>
#include <memory>
#include <set>
#include <poll.h>
#include <functional>
using std::set;
using std::cout;
using std::endl;
using std::weak_ptr;
using std::shared_ptr;

namespace tudou
{

#define _MAX_READ_SIZE_ 1024
#define _CLOSE_TIME_ 8000

class Timer;

class TcpConnection
: public enable_shared_from_this<TcpConnection>
{
public:
	using Ptr = shared_ptr<TcpConnection>;
	using MessageCallback = function<void(const Ptr &)>;
	using CloseCallback = function<void(const Ptr &)>;
	using HandShakeCallback = function<void(const Ptr &)>;
	using ErrorCallback = function<void(const Ptr &, const Error &)>;
	enum TcpState
	{
		INVAILD, HANDSHAKING,
		CONNECTED, CLOSED, FAILED
	};
	TcpConnection(EventLoop::Ptr loop, int fd);
	
	
	//connect封装
	Ptr connection(EventLoop::Ptr loop, const string &host, uint16_t port, int timeout = 10, const char *localIp = "0.0.0.0");
	//设置回调函数
	void setReadCb(const MessageCallback &cb);
	
	void setWriteCb(const MessageCallback &cb);

	void setCloseCb(const CloseCallback &cb);
	
	void setErrorCb(const ErrorCallback &cb);

	
	//发送消息，首先把消息存到缓冲区
	inline
	int send(const string &msg); 
	/*
		1. 缓冲区没有数据尝试直接::write
		2. 若有数据就append到缓冲区
		3. 没全部发完把剩下的append到缓冲区
	*/
	
	int send(const char *msg, size_t len);
	

	EventLoop::Ptr getEventLoop();
	Channel::Ptr getChannel();
	TcpState getState();
		
	Buffer* getInput();
	Buffer* getOutput();
	SocketAddress getAddress();
	
	void setState(TcpState s);
	
	//主动断开链接
	void shutdown();
	

	//处理读事件,如果使用lt模式，有数据就会一直触发
	void handleRead(const Ptr &ptr);
	
	void handleWrite(const Ptr &ptr);
	
	
	void handleError(const Ptr &ptr);
	
	int handleHandshake(const Ptr &ptr); //握手,connect函数在第二次握手返回,缓冲区是可写的
	
	//处理关闭,被动断开链接
	void handleClose(const Ptr &ptr);
	
	
	//超时还没连上就断开
	void handleTimeout(const Ptr &ptr, int timeout);

	void set(EventLoop::Ptr loop, int _fd);
	
	weak_ptr<Timer>& getTimer();
	void setTimer(shared_ptr<Timer> &timer);
	
private:
	Channel::Ptr _channel; 
	EventLoop::Ptr _loop;
	
	Buffer _input, _output;
	int _sockfd;
	int _timeout;
	weak_ptr<Timer> _timer;
	TcpState _state; //tcp状态
	// 读写 - 链接 - 断开 回调函数 
	
	MessageCallback _read_cb = nullptr, _write_cb = nullptr;
	ErrorCallback _error_cb = nullptr;
	CloseCallback _close_cb = nullptr;
	SocketAddress _address;
	
};

class TcpServer
: public enable_shared_from_this<TcpServer>
{
	friend class TudouServer;
public:
	using Ptr = shared_ptr<TcpServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
	TcpServer(EventLoop::Ptr loop)
	: _loop(loop)
	, _channel(nullptr)
	, _read_cb(nullptr)
	, _write_cb(nullptr)
	, _connection_cb(nullptr)
	, _error_cb(nullptr)
	{}
	int listen(const string &host, uint16_t port)
	{
		auto ret = SocketTool::listenTcp(host.c_str(), port); //pair<Error, fd>
		if(ret.second < 0)
		{
			return -1;
		}

		_channel.reset(new Channel(_loop.get(), ret.second)); //监听socket加入epoll
		_channel->enableWrite(false);
		_channel->enableRead(true);
		auto conn = shared_from_this();
		_channel->setReadCb([&, conn, this](){ handleAccept(conn); });
		return 0;
	}
	
	Ptr start(EventLoop::Ptr loop, const string &host, uint16_t port)
	{
		Ptr ptr(new TcpServer(loop));  //在这里要设置回调函数
		ptr->setReadCb(_read_cb);
		ptr->setWriteCb(_write_cb);	
		ptr->setConnectionCb(_connection_cb);
		ptr->setErrorCb(_error_cb);
		auto ret = ptr->listen(host, port);
		if(ret)
		{
			return nullptr;
		}
		
		return ptr;
	}
	
	void setReadCb(const MessageCallback& cb) { _read_cb = cb; }
	void setWriteCb(const MessageCallback& cb) { _write_cb = cb; }
	void setErrorCb(const ErrorCallback &cb) { _error_cb = cb; }
	void setConnectionCb(const MessageCallback &cb) { _connection_cb = cb; }
	Channel::Ptr getChannel() { return _channel; }
	void handleAccept(shared_ptr<TcpServer> _conn) //listen readcb
	{
		auto _channel = _conn->getChannel();
		struct sockaddr_in client;
		memset(&client, 0, sizeof(client));
		socklen_t len = sizeof(client);
		//链接客户端
		int connfd = ::accept(_channel->getFd(), (struct sockaddr *)&client, &len);
		if(connfd < 0)
		{
			std::cout<< strerror(errno);
			return ;
		}
		auto _choose_loop = EventLoopPool::getInstance().choose(); //选取一个eventloop
		//if(_choose_loop == _loop)	std::cout << "great" << std::endl;
		SocketTool::setNonBlock(connfd, true); //设置非阻塞
		TcpConnection::Ptr con(new TcpConnection(_choose_loop, connfd)); //新建一个tcpconnection
		con->setState(TcpConnection::CONNECTED); //已链接
		con->getAddress().set(SocketTool::inetNtop(client), SocketTool::ntoh(client.sin_port)); //链接客户端的信息
		
		_conn->_connections.emplace(con); //有一个链接加入
		//std::cout << "true size" << _connections.size() << std::endl;
		auto fun = [&, con, this]()
					{
						
						con->setReadCb([&, this](const TcpConnection::Ptr &s) 
										{
											if(_read_cb)	{ _read_cb(s); }
										} );	

						con->setWriteCb([&, this](const TcpConnection::Ptr &s) 
										{
											if(_write_cb)	{ _write_cb(s); }
										} );	
						con->setCloseCb([&, this](const TcpConnection::Ptr &s) 
										{
											if(_close_cb)	{ _close_cb(s); }
										} );						
										
						con->setErrorCb([&, this](const TcpConnection::Ptr &x, const Error &s)
										{
											if(_error_cb) { _error_cb(x, s); }
										} );
						
						if(_connection_cb) { _connection_cb(con); }
						
						con->set(_choose_loop, connfd); //新建channel并加入监听
						//加入定时器，超时断开连接
						auto _time = make_shared<Timer>(_CLOSE_TIME_, [con, this]()
							{
								con->shutdown();
								con->getTimer().lock()->delTimer();
								return false;
							}
						, con);	
						TimerManager::getInstance().addTimer(_time);

					};

		_loop->runAsync(fun, false);


	
		
		
	}
	
	//关闭连接
	void shutdown(TcpConnection::Ptr con)
	{
		auto it = _connections.find(con);
		if(it != _connections.end())
		{
			con->shutdown(); //关闭connection
		}
	}
	
	void shutdown()
	{
		for(auto & it : _connections)
		{
			it->shutdown();
		}
	}
	
	size_t size() { return _connections.size(); } //链接个数

	void send(const string &msg)
	{
		
		for(auto &it : _connections)
		{
			it->send(msg);
		}
	}
	
private:
	EventLoop::Ptr _loop; //loop
	SocketAddress _address;
	Channel::Ptr _channel;
	MessageCallback _read_cb, _write_cb, _close_cb, _connection_cb;
	
	ErrorCallback _error_cb;
	set<TcpConnection::Ptr> _connections; //管理链接
	int _error_num;
	int _reconnect_num;
};


class TudouServer
{
public:
	using Ptr = shared_ptr<TudouServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
	TudouServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum)
	: _main_loop(loop)
	, _thread_pool(threadnum)
	, _event_pool(EventLoopPool::getInstance(loop, eventnum))
	, _host(host)
	, _port(port)
	, _server(nullptr)
	{}
	void setReadCb(const MessageCallback& cb) { _read_cb = cb; }
	void setWriteCb(const MessageCallback& cb) {_write_cb = cb; }
	void setErrorCb(const ErrorCallback &cb) { _error_cb = cb; }
	void setConnectionCb(const MessageCallback &cb) { _connection_cb = cb; }

	void start() //最后一个参数是线程池所创建的线程
	{
		TcpServer _temp_server(_main_loop);
		_temp_server.setReadCb(_read_cb);
		_temp_server.setWriteCb(_write_cb);
		_temp_server.setErrorCb(_error_cb);
		_temp_server.setConnectionCb(_connection_cb);
		_server = _temp_server.start(_main_loop, _host, _port); //启动监听
		
		
	}
	
	void shutdown()
	{
		_server->shutdown();
		_thread_pool.shutdown();
	}
	//发送数据到缓冲区
	
	void send(const string &message)
	{
		//将任务加入线程池
		
		bool flag = _thread_pool.addTask(
			[&, this, message]()
			{
				_server->send(message + "\r\n");
			}
		);
		
	}
	
	//TcpServer::Ptr & getServer() { return _server; }
private:
	const string _host;
	uint16_t _port;
	size_t _threadnum;
	size_t _eventnum;
	MessageCallback _read_cb, _write_cb, _close_cb, _connection_cb;
	EventLoop::Ptr _main_loop;
	ErrorCallback _error_cb;
	EventLoopPool _event_pool;
	TcpServer::Ptr _server;
	ThreadPool _thread_pool;
};

}
#endif
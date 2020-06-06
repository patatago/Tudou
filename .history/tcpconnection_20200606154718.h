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
using std::shared_ptr;

namespace tudou
{

#define _MAX_READ_SIZE_ 1024

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
	TcpConnection(EventLoop* loop, int fd)
	: _loop(loop)
	, _sockfd(fd)
	, _timeout(10)
	, _input()
	, _output()
	{
		//_read_cb = _write_cb = _error_cb = _close_cb = nullptr;
		_channel = make_shared<Channel>(_loop, _sockfd); //初始化一个channel
	}
	
	
	//connect封装
	Ptr connection(EventLoop* loop, const string &host, uint16_t port, int timeout = 10, const char *localIp = "0.0.0.0")
	{
		Ptr _conn(new TcpConnection(loop, 0));
		_address.set(host, port);
		_timeout = timeout;
		auto nowTime = getMsec(); //当前时间
		auto ret = SocketTool::connectTcp(host.c_str(), port); //socket - bind - connect
		switch(ret.first.getStatus())
		{
		case Error::OK:
			{
				_state = HANDSHAKING;
				set(_loop, ret.second);
				break;
			}
		default:
			break;
		}
		
		if(ret.second < 0)
		{
			return nullptr;
		}
		
		handleTimeout(shared_from_this(), timeout);
		return _conn; //返回一个Tcpconnection
	}

	//设置回调函数
	void setReadCb(const MessageCallback &cb)
	{
		_read_cb = cb;
		_channel->enableRead(true); //设置可读	
	}
	void setWriteCb(const MessageCallback &cb)
	{
		_write_cb = cb;
		_channel->enableWrite(true); //设置可写
	}
	void setCloseCb(const CloseCallback &cb)
	{
		_close_cb = cb;
	}
	void setErrorCb(const ErrorCallback &cb)
	{
		_error_cb = cb;
	}
	
	//发送消息，首先把消息存到缓冲区
	inline
	int send(const string &msg) { send(msg.c_str(), msg.size()); }
	
	/*
		1. 缓冲区没有数据尝试直接::write
		2. 若有数据就append到缓冲区
		3. 没全部发完把剩下的append到缓冲区
	*/
	int send(const char *msg, size_t len)
	{
		if(_channel)
		{
			if(!_output.empty()) //输出缓冲区有数据还没有发送
			{
				_output.append(msg, len);
				return 0;
			}
			//尝试直接发送
			pair<Error, int>
			ret = SocketTool::sendPart(_channel->getFd(), msg, len);
			auto &_err = ret.first;
			if(ret.first.getStatus() != Error::OK)
			{
				return -1; //发送不成功
			}
			if(ret.second == len)
			{
				return len; //全部发送成功
			}
			_output.append(msg+ret.second, len-ret.second); //剩下的加入缓冲区
			return 1;
		}
		return -1;
	}

	EventLoop* getEventLoop() { return _loop; }
	Channel::Ptr getChannel() { return _channel; }
	TcpState getState() { return _state; }
		
	Buffer* getInput() { return &_input; }
	Buffer* getOutput() { return &_output; }
	SocketAddress getAddress() { return _address; }
	
	void setState(TcpState s) { _state = s; }
	
	//主动断开链接
	void shutdown()
	{
		//缓冲区还有数据没有发完
		if(!_output.empty())
		{
			//把缓冲区所有信息发出去
			
		}
		auto ptr = shared_from_this();
		handleClose(ptr);
		
	}
private:
	//处理读事件,如果使用lt模式，有数据就会一直触发
	void handleRead(const Ptr &ptr)
	{
		
		char _buffer[_MAX_READ_SIZE_];
		
		//std::cout << "head read" << std::endl;
		if(ptr->_state == HANDSHAKING && !handleHandshake(ptr)) //判断是否链接
		{
			return ;
		}
		//std::cout << "this is handle" << std::endl;
		
		if(ptr->_state == CONNECTED && _channel->getFd() > 0)
		{
			
			auto ret = SocketTool::readPart(_channel->getFd(), _buffer, _MAX_READ_SIZE_);	
			
			
			if(ret.second == -2) //没有读到数据
			{
				handleClose(ptr);
				return ;
			}
			else if(ret.second == -1)
			{
				//std::cout << "no" << std::endl;
				return ;
			}

			_input.append(_buffer, ret.second); //存入输入缓冲区

			//读到了数据
			
			if(_read_cb)
			{	
				_read_cb(ptr);
			}
		}
		
		//std::cout << "zuile" << std::endl;
	}
	void handleWrite(const Ptr &ptr)
	{
		
		if(ptr->_state == HANDSHAKING)
		{
			handleHandshake(ptr);
		}
		else if(ptr->_state == CONNECTED)
		{
			//没有数据可以发送
			//std::cout << "this is " <<std::endl;
			if(_output.empty() && _write_cb)
			{
				_write_cb(ptr);
				return ;
			}
			auto ret = SocketTool::sendPart(_channel->getFd(), _output.begin(), _output.size());
			if(ret.first.getStatus() != Error::OK) //发送失败
			{
				handleClose(shared_from_this());
				return ;
			}
			if(ret.second >= 0)
			{
				_output.consume(ret.second); //发送了second
			}
			if(_write_cb)
			{
				_write_cb(ptr);
			}

			
		}
		
	
	}
	void handleError(const Ptr &ptr)
	{
		Error _error;
		_channel->enableRead(false);
		_channel->enableWrite(false);
		_state = FAILED;
		if(_error_cb)
		{
			_error_cb(ptr, _error);
		}
	}
	int handleHandshake(const Ptr &ptr) //握手,connect函数在第二次握手返回,缓冲区是可写的
	{ 										//判断是否链接成功，成功之后触发pollout
		struct pollfd pf;
		memset(&pf, 0, sizeof(pf));
		pf.fd = _channel->getFd();
		pf.events = POLLOUT | POLLERR;
		int ret = poll(&pf, 1, 0);
		if(ret == 1)
		{
			if(pf.events & POLLOUT)
			{
				//监听到读事件连接成功
				_state = CONNECTED;
				return 0;
			}
		}
		
		return -1;
	}
	//处理关闭,被动断开链接
	void handleClose(const Ptr &ptr)
	{

		_state = CLOSED;  //设置状态为close
		_channel->enableWrite(false);
		_channel->enableRead(false);
		//把channel从eventloop中删除
		_loop->delChannel(_channel.get());
		if(_close_cb)
		{
			_close_cb(ptr);
		}
	}
	
	//超时还没连上就断开
	void handleTimeout(const Ptr &ptr, int timeout)
	{
		if(timeout > 0)
		{
			auto conn = shared_from_this();
			auto _time = make_shared<Timer>(timeout, [conn]()
								{
									if(conn->getState() == HANDSHAKING)
									{
										conn->setState(TcpConnection::CLOSED);
										auto ch = conn->getChannel();
										ch->close(); //关闭channel
										ch = nullptr; 
									}
									return false;
								}
						, ptr);	
			TimerManager::getInstance().addTimer(_time);
		}
	}
public:
	void set(EventLoop* loop, int _fd)
	{
		
		Ptr _point = shared_from_this(); //获取一个指针,设置channel相关回调函数
		_point->getChannel()->setReadCb( [&, this](){ _point->handleRead(_point); } );
		_point->getChannel()->setWriteCb( [&, this](){ _point->handleWrite(_point); } );
		//////////////////////
		_point->getChannel()->setErrorCb( [&, this](){ _point->handleError(_point); } );
		_point->getChannel()->setCloseCb( [&, this](){ _point->handleClose(_point); } );
	}
	
	
private:
	Channel::Ptr _channel; 
	EventLoop* _loop;
	
	Buffer _input, _output;
	int _sockfd;
	int _timeout;
	
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
	TcpServer(EventLoop* loop)
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

		_channel.reset(new Channel(_loop, ret.second)); //监听socket加入epoll
		_channel->enableWrite(false);
		_channel->enableRead(true);
		auto conn = shared_from_this();
		_channel->setReadCb([&, conn, this](){ handleAccept(conn); });
		return 0;
	}
	
	Ptr start(EventLoop* loop, const string &host, uint16_t port)
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
		std::cout << "this is epoll wait3" << std::endl;
		//链接客户端
		int connfd = ::accept(_channel->getFd(), (struct sockaddr *)&client, &len);
		if(connfd < 0)
		{
			std::cout<< strerror(errno);
			return ;
		}
		std::cout << "this is epoll wait5" << std::endl;
		auto &_choose_loop = _loop;//EventLoopPool::getInstance().choose(); //选取一个eventloop
		SocketTool::setNonBlock(connfd, true); //设置非阻塞
		std::cout << "this is epoll wait6" << std::endl;
		TcpConnection::Ptr con(new TcpConnection(_choose_loop, connfd)); //新建一个tcpconnection
		std::cout << "this is epoll wait1" << std::endl;
		con->setState(TcpConnection::CONNECTED); //已链接
		con->getAddress().set(SocketTool::inetNtop(client), SocketTool::ntoh(client.sin_port)); //链接客户端的信息
		_conn->_connections.emplace(con); //有一个链接加入
		std::cout << "true size" << _connections.size() << std::endl;
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
	EventLoop* _loop; //loop
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
		TcpServer _temp_server(_main_loop.get());
		_temp_server.setReadCb(_read_cb);
		_temp_server.setWriteCb(_write_cb);
		_temp_server.setErrorCb(_error_cb);
		_temp_server.setConnectionCb(_connection_cb);
		_server = _temp_server.start(_main_loop.get(), _host, _port); //启动监听
		
		
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
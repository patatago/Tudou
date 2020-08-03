#ifndef __TUDOU_TCPCONNECTION_H__
#define __TUDOU_TCPCONNECTION_H__

#include "../poller/event.h"
#include "../util/error.h"
#include "../util/util.h"
#include "../log/log.h"
#include "socket.h"
#include "../util/buffer.h"
#include "../thread/threadpool.h"
#include "../poller/eventlooppool.h"
#include <string.h>
#include "../util/timer.h"
#include <iostream>
#include <memory>
#include <set>
#include <poll.h>
#include <mutex>
#include <functional>
using std::set;
using std::cout;
using std::endl;
using std::weak_ptr;
using std::shared_ptr;
using std::mutex;

namespace tudou
{

#define _MAX_READ_SIZE_ 1024
#define _CLOSE_TIME_ 8000

class Timer;

class TcpConnection
: public enable_shared_from_this<TcpConnection>
{
	friend class TcpServer;
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
	Ptr connection(EventLoop::Ptr loop, const string &host, uint16_t port, int timeout = 10);
	//设置回调函数
	void setReadCb(const MessageCallback &cb);
	
	void setWriteCb(const MessageCallback &cb);

	void setCloseCb(const CloseCallback &cb);
	
	void setErrorCb(const ErrorCallback &cb);

	
	//发送消息，首先把消息存到缓冲区
	int send(Buffer &buffer);
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
	uint64_t _name;
	mutex _mutex;
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

class TcpClient
: public enable_shared_from_this<TcpClient>
{
public:
	using Ptr = shared_ptr<TcpClient>;
	TcpClient(EventLoop::Ptr loop, TcpConnection::Ptr conn);
	static TcpClient::Ptr start(EventLoop::Ptr loop, const string &host, uint16_t port, int timeout = 10);

private:
	TcpConnection::Ptr _connection;
	EventLoop::Ptr _loop; //loop
};


class TcpServer
: public enable_shared_from_this<TcpServer>
{
	friend class TudouServer;
public:
	using Ptr = shared_ptr<TcpServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
	TcpServer(EventLoop::Ptr loop);
	int listen(const string &host, uint16_t port);
	
	Ptr start(EventLoop::Ptr loop, const string &host, uint16_t port);
	
	void setReadCb(const MessageCallback& cb);
	void setWriteCb(const MessageCallback& cb);
	void setErrorCb(const ErrorCallback &cb);
	void setConnectionCb(const MessageCallback &cb);
	Channel::Ptr getChannel();
	void handleAccept(shared_ptr<TcpServer> _conn); //listen readcb

		
	
	//关闭连接
	void shutdown(TcpConnection::Ptr con);
	
	void shutdown();
	
	size_t size();

	void send(const string &msg);


	
private:
	EventLoop::Ptr _loop; //loop
	SocketAddress _address;
	Channel::Ptr _channel;
	MessageCallback _read_cb, _write_cb, _close_cb, _connection_cb;
	
	ErrorCallback _error_cb;
	map<uint64_t ,TcpConnection::Ptr> _connections; //管理链接
	int _error_num;
	int _reconnect_num;
};


class TudouServer
{
public:
	using Ptr = shared_ptr<TudouServer>;
	using MessageCallback = TcpConnection::MessageCallback;
	using ErrorCallback = TcpConnection::ErrorCallback;
	TudouServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum);

	void setReadCb(const MessageCallback& cb); 
	void setWriteCb(const MessageCallback& cb);
	void setErrorCb(const ErrorCallback &cb); 
	void setConnectionCb(const MessageCallback &cb);

	void start(); //最后一个参数是线程池所创建的线程
	
	void shutdown();
	
	//发送数据到缓冲区
	
	void send(const string &message);
	
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
#ifndef __TUDOU_SOCKET_H__
#define __TUDOU_SOCKET_H__
#include "error.h"
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <endian.h>
#include "buffer.h"
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <memory>
#include <map>
#include <utility>
#include "util.h"
//#include "error.h"

using std::string;
using std::pair;
using std::shared_ptr;
using std::make_pair;
namespace tudou
{

class SocketTool
{
public:
	template <typename Type>
	static Type hton(Type v)//转网络序
	{
		return typeHton(v);
	}
	
	
	template <typename Type>
	static Type ntoh(Type v) //转字节序
	{
		return typeNtoh(v);
	}
	
	static void inetPton(const char *s, struct sockaddr_in &serv)
	{
		inet_pton(AF_INET, s, &serv.sin_addr.s_addr);
		//std::cout << ref << std::endl;
	}
	static string inetNtop(struct sockaddr_in &serv)
	{
		char buf[1024]={};
		inet_ntop(AF_INET, &serv.sin_addr.s_addr,buf,static_cast<socklen_t>(sizeof(buf)));
		return buf;
	}
	
	//设置接受缓冲区大小
	static int setRecvBuf(int sockfd, int size) 
	{
		int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char *)&size, static_cast<socklen_t>(sizeof(size)));
		return ret;
	}
	//设置发送缓冲区大小
	static int setSendBuf(int sockfd, int size) 
	{
		int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char *)&size, static_cast<socklen_t>(sizeof(size)));
		return ret;
	}
	
	static bool checkIp(struct sockaddr_in &s)
	{
		return s.sin_addr.s_addr != INADDR_NONE; //判断ip地址合法性
	}
	
	static int reuseAddr(int sockfd, bool flag)
	{
	    int opt = flag;
    	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, static_cast<socklen_t>(sizeof(opt)));
		return ret;
	}
	
	static int reusePort(int sockfd, bool flag)
	{
		int opt = flag;
    	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&opt, static_cast<socklen_t>(sizeof(opt)));
		return ret;
	}
	static int noDelay(int sockfd, bool flag) //禁用Nagle算法
	{
		int opt = flag;
		int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,(const char *)&opt,static_cast<socklen_t>(sizeof(opt)));
		return ret;
	}
	
	static int keepAlive(int sockFd, bool flag) 
	{
		int opt = flag;
		int ret = setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&opt,static_cast<socklen_t>(sizeof(opt)));
		return ret;	
	}
	
	static int setCloseOnExec(int sockfd)
	{
			// close-on-exec
		auto flag = ::fcntl(sockfd, F_GETFD, 0);
		flag |= FD_CLOEXEC;
		return ::fcntl(sockfd, F_SETFD, flag);


	}
	static int setNonBlock(int sockfd, bool flag)
	{
    	int old_opt = fcntl(sockfd, F_GETFL);
		if(flag)
		{
			return fcntl(sockfd, old_opt | O_NONBLOCK);
		}
		else
		{
			return fcntl(sockfd, old_opt & ~O_NONBLOCK);
		}
	}

	static int socketTcp()
	{
		int ret = socket(AF_INET, SOCK_STREAM, 0); 
		return ret;
	}

	static pair<Error, int> 
	connectTcp(const char *ip, uint16_t port, bool isasync = false) //客户端使用
	{
		struct sockaddr_in cli;
		Error _error(Error::OK, "connect ok");
		memset(&cli, 0, sizeof(cli));
		cli.sin_family = AF_INET;
		cli.sin_port = hton(port);
		inetPton(ip, cli); //存入服务器ip
		
		int fd = socketTcp();
		if(fd < 0)
		{
			_error.setType(Error::SOCKET_ERROR);
			return make_pair(_error, -1);
		}
		reuseAddr(fd, true);
		reusePort(fd, true);
		
		setNonBlock(fd, isasync); //设置是否非阻塞链接
		setCloseOnExec(fd);
		noDelay(fd, true);
		int opt = ::connect(fd, (struct sockaddr*)&cli, sizeof(cli));
		if(opt > 0)
		{
			return make_pair(_error, fd);
		}
		else if(isasync && errno == EINPROGRESS) //非阻塞链接
		{
			return make_pair(_error, fd);
		}
		_error.setType(Error::CONNECT_ERROR);
		return make_pair(_error, -1);
	}
	
	
	static pair<Error, int> 
	listenTcp(const char *ip, uint16_t port)
	{
		auto fd = socketTcp();
		if(fd < 0)
		{
			std::cout << "socket err" << std::endl;
			return make_pair(Error(Error::FAILED, "socket get error"), -1);
		}
		reuseAddr(fd, true);
		reusePort(fd, true);
		setNonBlock(fd, true);
		setCloseOnExec(fd);
		auto flag = bindTcp(fd, ip, port);
		if(flag < 0)
		{
			//std::cout << strerror(errno) << std::endl;
			std::cout << "bind err" << std::endl;
			return make_pair(Error(Error::FAILED, "bind error"), -1);
		}
		flag = ::listen(fd, 20);
		//std::cout << strerror(errno) << std::endl;;
		if(flag)
		{
			std::cout << "listen err" << std::endl;
			return make_pair(Error(Error::FAILED, "bind error"), -1);
		}
		else
		{
			return make_pair(Error(Error::OK, "listen ok"), fd);
		}

	}	
	static int bindTcp(int &sockfd, const char *ip, uint16_t port)
	{
		//std::cout << sockfd << std::endl;
		struct sockaddr_in serv;
		memset(&serv, 0 , sizeof(serv));
		serv.sin_family = AF_INET;
		serv.sin_port = hton(port);
		serv.sin_addr.s_addr = INADDR_ANY;
		inetPton(ip, serv);
		
		int ret = ::bind(sockfd, (struct sockaddr*)&serv, sizeof(serv));
		return ret;
	}
	
	static int acceptTcp(const char *ip, uint16_t port)
	{
		auto ret = listenTcp(ip, port);
		if(ret.second < 0)
		{
			return -1;
		}
		struct sockaddr_in serv;
		memset(&serv, 0 , sizeof(serv));
		socklen_t len = sizeof(serv);
		return ::connect(ret.second, (struct sockaddr*)&serv, len);
	}
	static pair<Error, int>
	sendPart(int sockfd, const char *msg, size_t len)
	{
				std::cout << "----------+++**%%%%***+++-------" << std::endl;
		size_t ret = 0;
		Error _error(Error::OK, "send ok");
		while(1)
		{
			ret = ::write(sockfd, msg, len);
			if(ret <= 0)
			{
				if(errno == EINTR)
				{
					continue;
				}
				if(errno == EAGAIN)
				{
					break;
				}
				else
				{
					_error.setType(Error::ERROR);
					return make_pair(_error, 0);
				}
				
			}
			
		}
		return make_pair(_error, len);
	}


	static pair<Error, int>
	sendPart(int sockfd, Buffer &_buffer)
	{
		//尽可能全部发送
		int count = 0;
		Error _error(Error::OK, "send ok");
		int ret = 0, part = 0; //ret表示写成功的字符数， part表示总共发送了多少

		while(1)
		{
			auto pos = _buffer.find(); //找到一条数据
			ret = ::write(sockfd, _buffer.begin(), pos - _buffer.getReadPos() + 2);
			if(ret <= 0)
			{
				if(errno == EINTR)
				{
					continue;
				}
				else if(errno == EAGAIN)
				{
					_error.setErrno(errno);
					return make_pair(_error, part);
				}
				else
				{
					_error.setType(Error::ERROR);
					return make_pair(_error, -1);
				}
				
			}
			else
			{
				_buffer.consume(ret);
				part += ret;
			}
		}
		
	}
	
	
	static pair<Error, int>
	readPart(int sockfd, Buffer &_buffer)
	{
		
		int ret = 0;
		int part = 0;
		Error _error(Error::OK, "read Error::OK");
		while(1)
		{
			char _temp_buffer[65536];
			
			std::cout << ":ha connfd = " << sockfd << std::endl;
			ret = ::recv(sockfd, _temp_buffer, sizeof(_temp_buffer), 0);
			std::cout << _temp_buffer << std::endl;
			if(ret <= 0)
			{
				if(errno == EAGAIN) 
				{
					return make_pair(_error, part);
				}
				else if(errno == EINTR)
				{
					continue;
				}
				else
				{
					_error.setType(Error::CONNECT_ERROR);
					_error.setMsg("signal is Error::CONNECT_ERROR");
					std::cout << strerror(errno) << "0--------" << std::endl;	
					return make_pair(_error, -1);
				}
			}
			else
			{
				_buffer.append(_temp_buffer, ret); //存到缓冲区
				part += ret;
			//	if(ret != 65536) { break; }
			}
		}
			
	}


};


class SocketAddress 
{
public:
    SocketAddress(const std::string& ip, uint16_t port = 0) 
	: _ip(ip)
	, _port(port) 
	{}

    SocketAddress() 
	: _ip("")
	, _port(0) 
	{}
	void set(const std::string& ip, uint64_t port)
	{
		_ip = ip;
		_port = port;
	}
    string _ip;
    uint64_t _port;
};

}
#endif

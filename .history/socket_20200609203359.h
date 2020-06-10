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
		socklen_t len = sizeof(serv);
		return ::connect(ret.second, (struct sockaddr*)&serv, len);
	}
	static pair<Error, int>
	sendPart(int sockfd, const char *msg, size_t len)
	{
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
				else if(errno == EAGAIN)
				{
					return make_pair(_error, 0);
				}
				else
				{
					_error.setType(Error::ERROR);
					return make_pair(_error, -1);
				}
				
			}
			
		}
		return make_pair(_error, len);
	}


	static pair<Error, int>
	sendPart(int sockfd, Buffer &_buffer)
	{
		//尽可能全部发送
		Error _error(Error::OK, "send ok");
		int ret = 0, part = 0, len = _buffer.size(); //ret表示写成功的字符数， part表示总共发送了多少
		//std::cout << _buffer.begin() << std::endl;
		while(1)
		{
			
			auto pos = _buffer.find(); //找到一条数据
			string _message_line = _buffer.read(_buffer.size()); 
			//std::cout << "_message_line: " <<_message_line<< std::endl;
			ret = ::write(sockfd, _message_line.c_str(), _message_line.size());
			if(ret <= 0)
			{
				if(errno == EINTR)
				{
					continue;
				}
				else if(errno == EAGAIN)
				{
					//_error.setType(Error::ERROR);
					_error.setErrno(errno);
					return make_pair(_error, 0);
				}
				else
				{
					_error.setType(Error::ERROR);
					return make_pair(_error, -1);
				}
				
			}
			else
			{
				//_buffer.consume(ret);
			}
		}
	}
	


	//偷窥看一下是不是可以读了
	static 	
	size_t recvPeek(int sockfd, char *buf, size_t len)
	{
		while(1)
		{
			int ret = ::recv(sockfd, buf, len, MSG_PEEK);//从sockfd读取内容到buf（len是buf的长度）,但不去清空sockfd,偷窥
	
			if(ret == -1 && errno == EINTR)
			{
				continue;//信号中断
			}
			if(errno == EAGAIN)
			{
				std::cout << "EAGAIN = " << ret << std::endl;
			}
			std::cout << "ret = " << ret << std::endl;
			return ret;
		}
		
	}

	static 
	size_t readn(int sockfd, char *buf, size_t count)
	{
		auto nread = 0;
		auto nleft = count;
		while(nleft)
		{
			if((nread = ::read(sockfd, buf, nleft)) < 0)
			{
				if(errno == EINTR)
				{
					continue;
				}
				return -1; //读取失败
			}
			else if(nread == 0)
			{
				return  count - nleft; //读取了这么多字节
			}
			buf += nread;
			nleft -= nread;
		}
		return count;
	}
	static pair<Error, int>
	readPart(int sockfd, Buffer &_buffer)
	{
		char _temp_buffer[1024] = {};
		int ret = 0;
		int part = 0;
		Error _error(Error::OK, "read Error::OK");
		while(1)
		{
			ret = recvPeek(sockfd, _temp_buffer, 1024); //看一下sockfd有没有数据	
			if(ret < 0) //读完了，errno=eagain
			{
				//_error.setType(Error::ERROR);
				//_error.setErrno(errno);
				return make_pair(_error, 0);
			}
			else if(ret == 0) //断开连接
			{
				//此时客户端断开链接
				_error.setType(Error::CONNECT_ERROR);
				_error.setMsg("signal is Error::CONNECT_ERROR");	
				return make_pair(_error, -1);
			}
			else //已经读取了一部分，要将数据从sockfd中删去
			{
				readn(sockfd, _temp_buffer, ret);  //从sockfd读出
				_buffer.append(_temp_buffer, ret); //存到缓冲区
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

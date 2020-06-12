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
	static Type hton(Type v);//转网络序

	
	
	template <typename Type>
	static Type ntoh(Type v); //转字节序
	
	static void inetPton(const char *s, struct sockaddr_in &serv);

	static string inetNtop(struct sockaddr_in &serv);
	
	//设置接受缓冲区大小
	static int setRecvBuf(int sockfd, int size);
	//设置发送缓冲区大小
	static int setSendBuf(int sockfd, int size);
	
	static bool checkIp(struct sockaddr_in &s);
	
	static int reuseAddr(int sockfd, bool flag);
	static int reusePort(int sockfd, bool flag);
	static int noDelay(int sockfd, bool flag); //禁用Nagle算法
	
	
	static int keepAlive(int sockFd, bool flag);
	
	static int setCloseOnExec(int sockfd);
	static int setNonBlock(int sockfd, bool flag);

	static int setCloseWait(int sockFd, int second);
	static int socketTcp();
	static pair<Error, int> 
	connectTcp(const char *ip, uint16_t port, bool isasync = false); //客户端使用
	
	
	
	static pair<Error, int> 
	listenTcp(const char *ip, uint16_t port);
	static int bindTcp(int &sockfd, const char *ip, uint16_t port);
	
	static int acceptTcp(const char *ip, uint16_t port);
	static pair<Error, int>
	sendPart(int sockfd, const char *msg, size_t len);

	static pair<Error, int>
	sendPart(int sockfd, Buffer &_buffer);

	static pair<Error, int>
	readPart(int sockfd, Buffer &_buffer);

};


class SocketAddress 
{
public:
    SocketAddress(const std::string& ip, uint16_t port = 0);

    SocketAddress();
	void set(const std::string& ip, uint64_t port);
    string _ip;
    uint64_t _port;
};

}
#endif

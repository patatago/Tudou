#ifndef __TUDOU_UTIL_H__
#define __TUDOU_UTIL_H__
#include <functional>
#include <sys/time.h>
#include <fcntl.h>
#include <endian.h>
#include <string>

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
using std::string;
using std::function;

namespace tudou
{
const int _PATH_MAX_ = 1024;
//禁止拷贝基类
typedef enum {
	Event_Read = 1 << 0, //读事件
	Event_Write = 1 << 1, //写事件
	Event_Error = 1 << 2, //错误事件
	Event_LT    = 1 << 3,//水平触发
} Poll_Event;
#define toEpoll(event)    (((event) & Event_Read) ? EPOLLIN : 0) \
						| (((event) & Event_Write) ? EPOLLOUT : 0) \
						| (((event) & Event_Error) ? (EPOLLHUP | EPOLLERR) : 0) \
						| (((event) & Event_LT) ?  0 : EPOLLET)
#define toPoller(epoll_event) (((epoll_event) & EPOLLIN) ? Event_Read : 0) \
							| (((epoll_event) & EPOLLOUT) ? Event_Write : 0) \
							| (((epoll_event) & EPOLLHUP) ? Event_Error : 0) \
							| (((epoll_event) & EPOLLERR) ? Event_Error : 0)

using DefaultFunction = function<void()>;
class noncopyable
{
protected:
    noncopyable(){}
    ~noncopyable(){}
    noncopyable &operator=(const noncopyable&) = delete;
    noncopyable(const noncopyable&) = delete;    
};


string getPath();
void setNonBlock(int socketfd);


uint64_t getMsec();
uint16_t typeHton(uint16_t v);

uint32_t typeHton(uint32_t v);

uint64_t typeHton(uint64_t v);

uint16_t typeNtoh(uint16_t v);

uint32_t typeNtoh(uint32_t v);

uint64_t typeNtoh(uint64_t v);
//int typeNtop(int v);

}
#endif
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

using DefaultFunction = function<void()>;

//禁止拷贝基类
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
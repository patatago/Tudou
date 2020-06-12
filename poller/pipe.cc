#include "pipe.h"
using namespace tudou;

PipeBase::PipeBase()
{
    if(pipe(_pipe_fd) == -1)
    {
        Fatal << "pipe create error" << std::endl;
        throw std::runtime_error("pipe create error");
    }
    setNonBlock(_pipe_fd[0]);
    setNonBlock(_pipe_fd[1]); //设置非阻塞
    
}
PipeBase::~PipeBase()
{
    close(_pipe_fd[0]);
    close(_pipe_fd[1]);
}
int PipeBase::write(const void *buf, int n)
{
    int ret = 0;
    do
    {
        ret = ::write(_pipe_fd[1], (char *)buf, n);
    }
    while(ret == -1 && errno == EAGAIN);
    return ret;
}
int PipeBase::read(void *buf, int n)
{
    int ret = 0;
    do
    {
        ret = ::read(_pipe_fd[0], (char *)buf, n);
    }
    while(ret == -1 && errno == EAGAIN);
    return ret;
}
int PipeBase::getReadFd() const { return _pipe_fd[0]; }
int PipeBase::getWriteFd() const { return _pipe_fd[1]; }

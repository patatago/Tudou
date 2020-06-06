#ifndef __TUDOU_PIPE_H__
#define __TUDOU_PIPE_H__

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "util.h"
#include <unistd.h>

namespace tudou
{

class PipeBase
{
public:
	PipeBase()
	{
		if(pipe(_pipe_fd) == -1)
		{
			throw std::runtime_error("pipe create error");
		}
		setNonBlock(_pipe_fd[0]);
		setNonBlock(_pipe_fd[1]); //设置非阻塞
		
	}
	~PipeBase()
	{
		close(_pipe_fd[0]);
		close(_pipe_fd[1]);
	}
	int write(const void *buf, int n)
	{
		int ret = 0;
		do
		{
			ret = ::write(_pipe_fd[1], (char *)buf, n);
		}
		while(ret == -1 && errno == EAGAIN);
		return ret;
	}
	int read(void *buf, int n)
	{
		int ret = 0;
		do
		{
			ret = ::read(_pipe_fd[0], (char *)buf, n);
		}
		while(ret == -1 && errno == EAGAIN);
		return ret;
	}
	int getReadFd() const { return _pipe_fd[0]; }
	int getWriteFd() const { return _pipe_fd[1]; }
private:
	int _pipe_fd[2] = {-1, -1};
};

}
#endif
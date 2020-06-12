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
	PipeBase();
	~PipeBase();
	int write(const void *buf, int n);
	int read(void *buf, int n);
	int getReadFd() const;
	int getWriteFd() const;
private:
	int _pipe_fd[2] = {-1, -1};
};

}
#endif
#include "util.h"
using namespace tudou;

void tudou::setNonBlock(int socketfd)
{
    int old_opt = fcntl(socketfd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(socketfd, F_SETFL, new_opt);
}

uint64_t tudou::getMsec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000ul + tv.tv_usec/1000;
}

uint16_t tudou::typeHton(uint16_t v)
{
	return htobe16(v);
}
uint32_t tudou::typeHton(uint32_t v)
{
	return htobe32(v);
}
uint64_t tudou::typeHton(uint64_t v)
{
	return htobe64(v);
}
uint16_t tudou::typeNtoh(uint16_t v)
{
	return be16toh(v);
}
uint32_t tudou::typeNtoh(uint32_t v)
{
	return be32toh(v);
}
uint64_t tudou::typeNtoh(uint64_t v)
{
	return be64toh(v);
}

string tudou::getPath()
{
	char _buf[_PATH_MAX_] = {};
	string _file_path="";
	int idx = -1;
	idx = readlink("/proc/self/exe", _buf, sizeof(_buf));
	if(idx>0 && idx<sizeof(_buf))
	{
		char *p = strrchr(_buf, '/');
		for(auto i=_buf; i!=p; ++i)
			_file_path += *i;
		_file_path += '\0';
	}
	else 
	{
		_file_path = "./";
	}
	return _file_path;
}

#ifndef __TUDOU_ERROR_H__
#define __TUDOU_ERROR_H__
#include <string>
#include <errno.h>
#include <string.h>
using std::string;

namespace tudou
{
class Error
{
public:
	enum ErrorType
	{
		OK, ERROR, FAILED,
		UNKNOW, SOCKET_ERROR, CONNECT_ERROR, 
		INET_ERROR, OTHER_ERROR
	};

	Error();
	Error(ErrorType code, const string &msg);
	
	Error(int errno);
	
	
	void setType(ErrorType type);
	void setMsg(const string &msg);
	void setErrno(int err);
	ErrorType getStatus();
private:
	ErrorType _error_code; //错误代码
	int _errno;
	string _msg;
};
}

#endif
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

	Error()
	: _error_code(UNKNOW)
	, _msg("unknow")
	{}
	
	Error(ErrorType code, const string &msg)
	: _error_code(code)
	, _msg(msg)
	{}
	
	Error(int errno)
	: _errno(errno)
	, _error_code(UNKNOW)
	{
		if(_errno < 0)
		{
			_msg = "unknow";
		}
		else
		{
			_msg = ::strerror(_errno);
		}
	}
	
	
	void setType(ErrorType type) { _error_code = type; }
	void setMsg(const string &msg) { _msg = msg; }
	void setErrno(int err) 
	{ 
		_errno = err;
		_msg = ::strerror(err);
	}
	ErrorType getStatus() { return _error_code; }
private:
	ErrorType _error_code; //错误代码
	int _errno;
	string _msg;
};
}

#endif
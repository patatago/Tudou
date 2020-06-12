#include "error.h"
using namespace tudou;
namespace tudou


Error::Error()
: _error_code(UNKNOW)
, _msg("unknow")
{}

Error::Error(ErrorType code, const string &msg)
: _error_code(code)
, _msg(msg)
{}

Error::Error(int errno)
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


void Error::setType(ErrorType type) { _error_code = type; }
void Error::setMsg(const string &msg) { _msg = msg; }
void Error::setErrno(int err) 
{ 
    _errno = err;
    _msg = ::strerror(err);
}
ErrorType Error::getStatus() { return _error_code; }


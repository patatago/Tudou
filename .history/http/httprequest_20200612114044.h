#ifndef __HTTP_message_H__
#define __HTTP_message_H__
#include <mutex>
#include <string>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <sys/mman.h>
#include <utility>
#include "../tcpconnection.h"
#include "../buffer.h"
using std::map;
using std::string;
using std::stringstream;
using std::pair;
using std::make_pair;
using std::equal;
namespace tudou
{
// http请求	

class HttpMessage
{
	friend class HttpResponse;
public:
	enum Method
	{
		INVAILD, GET, POST, HEAD, PUT, DELETE
	};
	enum Version
	{
		UNKNOW = -1, HTTP10 = 0, HTTP11 = 1
	};
public:
	HttpMessage();
	void setVersion(Version ver);
	Version getVersion() const;
	
	bool setMethod(const char *x, size_t n);
	Method getMethod() const;
	
	void setFilenameCgi(const pair<string, string> &p);
	const pair<string, string> &getFilenameCgi() const;
	
	void addHeader(const string &key, const string &value);
	string getHeader(const string &key);
	
	const map<string, string> & getHeader() const;
	void setBody(const string &body);
	string &getBody();
	void reset(); //重置
	
	inline
	const string & getFilename();
	void print();
	HttpMessage operator=(const HttpMessage &s);
private:
	Method _method; //请求方法
	Version _version; //协议
	bool _is_static;
	pair<string, string> _filename_cgiargs;
	map<string, string> _head; //请求头部
	string _body; //请求体

};

class HttpParse
{
public:
	enum ParseState
	{
		PARSE_REQUESTLINE, PARSE_HEAD, PARSE_BODY, PARSE_OK
		//解析请求行----------解析请求头--解析请求体----解析ok
	};
	void print();
	HttpMessage &getHttpMessage();
	HttpParse();
	
	bool parseUrl(const string &path);
	
	bool parseRequestLine(int len, Buffer &buffer);
	
	bool parseRequestHead(int len, Buffer &buffer);
	//解析头部
	bool parse(Buffer &buffer);
	
private:
	ParseState _state;
	HttpMessage _message;
	
};

class HttpResponse
{
public:
	//定义状态码
	enum HttpState
	{
		INVALID, HTTP200 = 200, HTTP301 = 301,
		HTTP400 = 400, HTTP404 = 404, HTTP501 = 501
	};
	//定义协议版本
	enum Version
	{
		UNKNOW = -1, HTTP10 = 0, HTTP11 = 1
	};
public:
	void getFileType();
	void clientError(const string &cause, const string &errnum, const string &shortmsg, const string &longmsg);
	
	void append();
	HttpResponse(HttpMessage &message, const string &str, Buffer &buffer);
private:
	struct stat sbuf;
	HttpMessage _message;
	HttpState _state; //响应状态码
	string _message_return; //返回信息
	string _filetype;
	mutex _mutex;
	Buffer &_buffer;
};



}


#endif
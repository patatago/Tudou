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
class Buffer;
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
	Method getMethod() const { return _method; }
	
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
	void getFileType()
	{
		if(_message._filename_cgiargs.first.find(".html") != string::npos)
		{
			_filetype = "text/html";
		}
		else if(_message._filename_cgiargs.first.find(".gif") != string::npos)
		{
			_filetype = "image/gif";
		}
		else if(_message._filename_cgiargs.first.find(".png") != string::npos)
		{
			_filetype = "image/png";
		}
		else if(_message._filename_cgiargs.first.find(".jpg") != string::npos)
		{
			_filetype = "image/jpeg";
		}
		else if(_message._filename_cgiargs.first.find(".avi") != string::npos)
		{
			_filetype = "vedio/x-msvideo";
		}
		else
		{
			_filetype = "text/plain";
		}
	}
	void clientError(const string &cause, const string &errnum, const string &shortmsg, const string &longmsg)
	{
		stringstream ss, aa;

		//build http response body
		ss << "<html><title>Tudou Error</title>";
		ss << "<body bgcolor=""ffffff"">\r\n";
		ss << errnum << ": " << shortmsg << "\r\n";
		ss << "<p>" << longmsg << ": " << cause << "\r\n";
		ss << "<hr><em>The Tuduo Web server</em>\r\n";
		//打印信息 
		if(_message._version == 0)
		{
			aa << "HTTP/1.0" << " " << errnum << " " << shortmsg << "\r\n";
		}
		else
		{
			aa << "HTTP/1.1" << " " << errnum << " " << shortmsg << "\r\n";
		}
		aa << "Content-type: text/html\r\n";
		
		aa << "Content-length: " << ss.str().size() << "\r\n\r\n";
		_buffer.append(aa.str());
		_buffer.append(ss.str());
		//std::cout << aa.str() << ss.str() << std::endl;
	}
	
	void append()
	{
		char buf[1024] = {};
		int nread = 0;
		if(_message._method != (HttpMessage::POST || HttpMessage::GET))
		{
			clientError(_message.getFilename(), "501", "Not Implemented", "Tudou cannot implement this method");
			return ;
		}
		if(stat(_message.getFilename().c_str(), &sbuf) < 0)
		{
			clientError(_message.getFilename(), "404", "Not Found", "Tudou cannot find this file");
			return ;
		}
		if(!(S_ISREG(sbuf.st_mode)) ||!(sbuf.st_mode & S_IRUSR))
        {
            clientError(_message.getFilename(), "403", "Not Found", "Tiny couldn't read this file");  
            return ;    
        }
		getFileType();
		stringstream ss;
		//
		//	such as:
		//		HTTP/1.1 200 OK
		//
		ss << "HTTP/1." << _message._version << " " << _state << " OK " << "\r\nServer: " << _message_return << "\r\n";
		if(_message._head.find("Connection") != _message._head.end()
			&& (_message._head["Connection"] == "Keep-Alive"
			|| _message._head["Connection"] == "keep-alive"))
		{
			ss << "Connection: keep-alive" << "\r\n";
		}
		else
		{
			ss << "Connection: close" << "\r\n";
		}
		ss << "Content-length: " << sbuf.st_size << "\r\n";
		ss << "Content-type: " << _filetype << "\r\n\r\n";
		
		_buffer.append(ss.str());
		int srcfd = open(_message.getFilename().c_str(), O_RDONLY, 0777); 
		auto len = sbuf.st_size;
		while(len)
		{
			nread = ::read(srcfd, buf, 1024);
			_buffer.append(buf, nread);
			len -= nread;
		}
		//std::cout <<"----------++-------"<<_buffer.size() << std::endl;
    	close(srcfd);
 	}
	


	HttpResponse(HttpMessage &message, const string &str, Buffer &buffer)
	: _message(message)
	, _message_return(str)
	, _buffer(buffer)
	, _state(HTTP200)
	{}
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
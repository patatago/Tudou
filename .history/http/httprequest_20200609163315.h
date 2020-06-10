#ifndef __HTTP_message_H__
#define __HTTP_message_H__

#include <string>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <sys/mman.h>
#include <utility>
#include "../buffer.h"
using std::map;
using std::string;
using std::stringstream;
using std::pair;
using std::make_pair;
using std::equal;
namespace tudou
{
static char HttpStr[2] = {'\r', '\n'};
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
	HttpMessage()
	: _method(INVAILD)
	, _version(UNKNOW)
	, _body("")
	{}
	
	void setVersion(Version ver) { _version = ver; }
	Version getVersion() const { return _version; }
	
	bool setMethod(const char *x, size_t n)
	{
		const string &method = string(x, n);
		if(method == "GET") { _method = GET; }
		else if(method == "POST") { _method = POST; }
		else if(method == "HEAD") { _method = HEAD; }
		else if(method == "PUT") { _method = PUT; }
		else if(method == "DELETE") { _method == DELETE; }
		else { _method = INVAILD; }
		return _method != INVAILD;
	}
	Method getMethod() const { return _method; }
	
	void setFilenameCgi(const pair<string, string> &p) { _filename_cgiargs = p; }
	const pair<string, string> &getFilenameCgi() const { return _filename_cgiargs; }
	
	void addHeader(const string &key, const string &value)
	{
		_head[key] = value;
	}
	string getHeader(const string &key) 
	{
		if(_head.find(key) == _head.end())
		{
			return "";
		}
		return _head[key];
	}
	
	const map<string, string> & getHeader() const { return _head; }
	void setBody(const string &body) { _body = body; }
	string &getBody() { return _body; }
	void reset() //重置
	{
		_method = INVAILD, _version = UNKNOW;
		_body = "";
		_filename_cgiargs = make_pair("", "");
		_head.clear();
	}
	inline
	const string & getFilename() { return _filename_cgiargs.first; }
	void print()
	{
		std::cout << "method = " << _method 
				<< "version = " << _version 
				<< "filename = " << _filename_cgiargs.first <<" "<<_filename_cgiargs.second
				<< std::endl;
			for(auto it : _head)
			{
				std::cout << it.first <<": " << it.second << std::endl;
			}
			std::cout << _body << std::endl;
	}
	HttpMessage operator=(const HttpMessage &s)
	{
		_method = s._method, _version = s._version, _is_static = s._is_static;
		_filename_cgiargs = s._filename_cgiargs, _head = s._head;
		_body = s._body;
	}
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
	void print() { _message.print();}
	HttpMessage &getHttpMessage() { return _message; }
	HttpParse()
	: _state(PARSE_REQUESTLINE)
	{}
	
	bool parseUrl(const string &path)
	{
		if(path.find("cgi-bin") == string::npos)
		{
			pair<string, string> url("." + path, "");
			if(path[path.size() - 1] == '/')
			{
				url.first += "home.html";
			}
			_message.setFilenameCgi(url);
			return true;
		}
		return false; //不支持动态
	}
	
	bool parseRequestLine(int len, Buffer &buffer)
	{
		string _message_line = buffer.read(len+2); //读出请求行
		std::cout << "_message: " << _message_line << std::endl; 
		auto pos1 = _message_line.find(' ');
		_message.setMethod(&_message_line[0], pos1); //请求方法
		++pos1; //路径
		auto pos2 = _message_line.find(' ', pos1); //找下一个空格
	
		parseUrl(string(&_message_line[pos1], pos2-pos1));	//路径
	
		//开始解析http版本
		if(!equal(&_message_line[pos2+1], &_message_line[pos2 + 7], "HTTP/1."))
		{
			return false;
		}
		if(_message_line[pos2 + 8] == '1')
		{
			_message.setVersion(HttpMessage::HTTP11);
		}
		else if(_message_line[pos2 + 8] == '0')
		{
			_message.setVersion(HttpMessage::HTTP10);
		}
		else
		{
			return false;
		}
		return true;
	}
	
	bool parseRequestHead(int len, Buffer &buffer)
	{
		string _message_line = buffer.read(len+2); //读出请求头
		auto pos = _message_line.find(':'); //找到：
		
		if(pos == string::npos)
		{
			return false;
		}
		_message.addHeader(string(&_message_line[0], pos), 
						string(&_message_line[pos+2], _message_line.size()-pos-4));
		return true;
	}
	//解析头部
	bool parse(Buffer &buffer)
	{
		bool flag;
	    std::cout << buffer.begin() << std::endl;
		int findPos = -1;
		stringstream ss;

		while(true)
		{
			if(_state == PARSE_REQUESTLINE) //从请求行开始解析
			{
				findPos = buffer.find(); //找到"\r\n"
				std::cout << "pos = " << findPos << std::endl;
				if(findPos == -1)
				{
					return false; //解析失败
				}
				
				flag = parseRequestLine(findPos, buffer);
				if(flag)
				{
					_state = PARSE_HEAD; //读请求头
				}
				else
				{
					break; //不是http请求
				}
				

				
			}
			else if(_state == PARSE_HEAD) //请求头处理
			{
				findPos = buffer.find();
				if(findPos == -1)
				{
					return false; //解析失败
				}
				flag = parseRequestHead(findPos, buffer);
				if(!flag)
				{
					//遇到空行，下面是请求体
					_state = PARSE_BODY;
				}
			}
			else if(_state == PARSE_BODY)
			{
				findPos = buffer.find();
				if(findPos == -1)
				{
					return false; //解析失败
				}

				_message.setBody(string(buffer.begin(), findPos));
				buffer.consume(findPos + 2);
				break;	
			}
		}
		return true;
	}
	
	
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
	}
	
	void append()
	{
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
		ss << "HTTP/1." << _message._version << " " << _state << " OK " << _message_return << "\r\n";
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
		ss << "Content-type: " << _filetype << "\r\n";
		ss << "Content-length: " << sbuf.st_size << "\r\n\r\n";
		
		int srcfd = open(_message.getFilename().c_str(), O_RDONLY, 0777); 
    	char *srcp = (char *)mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    	close(srcfd);
		_buffer.append(ss.str());
		_buffer.append(string(srcp, sbuf.st_size) + "\r\n\r\n");
		munmap(srcp, sbuf.st_size);
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
	Buffer &_buffer;
};



}


#endif
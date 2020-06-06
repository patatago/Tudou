// #ifndef __TUDOU_HTTP_RESPONSE_H__
// #define __TUDOU_HTTP_RESPONSE_H__

// namespace tudou
// {
// //http返回类
// class HttpResponse
// {
// public:
// 	//定义状态码
// 	enum HttpState
// 	{
// 		INVALID, HTTP200 = 200, HTTP301 = 301,
// 		HTTP400 = 400, HTTP404 = 404
// 	};
// 	//定义协议版本
// 	enum Version
// 	{
// 		UNKNOW = -1, HTTP10 = 0, HTTP11 = 1
// 	};
// public:
// 	bool isLongConnection() const { return _is_long_connection; }
// 	void setIsLongConnection(bool flag) { _is_long_connection = flag; }
// 	Version getVersion() { return _version; }
// 	void setVersion(Version version) { _version = version; }
// 	void setState(HttpState state) { _state = state; }
// 	void setBody(const string &str) { _body = str; }
// 	void setHeader(const map<string, string> &header) { _header = header; }
// 	void append(Buffer &buffer)
// 	{
// 		stringstream ss;
// 		/*
// 			such as:
// 				HTTP/1.1 200 OK
// 		*/
// 		ss << "HTTP/1." << _version << " " << _state << _message;
// 		buffer.append(ss.str());
// 		if(_is_long_connection)
// 		{
// 			ss.clear();
// 			ss << "Content-Length: " << _body.size();
// 			buffer.append(ss.str());
// 			buffer.append("Connection: Keep-Alive");
// 		}
// 		else
// 		{
// 			buffer.append("Connection: close");
// 		}
// 		for(auto it = _header.begin(); it != _header.end(); ++it)
// 		{
// 			ss.clear();
// 			ss << it->first << ": " << it->second;
// 			buffer.append(ss.str());
// 		}
// 		buffer.append(""); //插入空行
// 		buffer.append(_body);
		
		
//  	}
// private:
// 	bool _is_long_connection; //是否是长连接
// 	Version _version;
// 	HttpState _state; //响应状态码
// 	string _message; //返回信息
// 	map<string, string> _header; //报文头列表
// 	string _body; //报文体
// };
// }


// #endif
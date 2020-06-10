#ifndef __TUDOU_RINGBUFFER_H__
#define __TUDOU_RINGBUFFER_H__
//#include <iostream>
#include <vector>
#include <string.h>
#include <memory>
#include <cmath>

using std::size_t;
using std::vector;
using std::string;
using std::max;
using std::shared_ptr;
namespace tudou
{
class Buffer
{
public:
	using Ptr = shared_ptr<Buffer>;
	Buffer(size_t size = 1024)
	: _buffer(size)
	, _read_pos(0)
	, _write_pos(0)
	{}

	size_t getReadPos() const { return _read_pos; }
	size_t length(int len) const { return len - _read_pos; }
	size_t capacity() const { return _buffer.size(); }
	size_t size() const { 	return _write_pos - _read_pos; }
	size_t writeSize() const { return _buffer.size() - _write_pos; }
	bool empty() const { return size() == 0; }


	void reset()  //重置read write位置
	{
		_read_pos = _write_pos = 0;
	}
	
	const char * begin() const { return &(*_buffer.begin()) + _read_pos; }
	const char * end() const { return &(*_buffer.begin()) + _read_pos + size(); }
	
	void append(const char *str, size_t len)
	{
		//std::cout << "recv buf:" <<str<<std::endl;
		if(size() == 0) //缓冲区全部读完了就重置
		{
			reset();
		}
		if(writeSize() < len) 
		{
			_buffer.resize(2 * max(_buffer.size(), len) ); 
		}
		for(size_t i=0; i<len; ++i)
		{
			_buffer[_write_pos++] = str[i];
		}
	}
	inline
	void append(const string &str)
	{
		append(str.c_str(), str.size());
	}
	
	int find(const string &str)
	{
		std::cout << "size = "<< size() << std::endl;
		std::cout << begin() << std::endl;
		if(!str.size() || !_buffer.size())
		{
		 	return -1;
		}
		string S = (begin(), end()-400);
		S+="\0";
		auto pos = S.find(str);
		std::cout << "third = " <<S<< std::endl;
		if(pos == string::npos) { return -1; }
		return pos + _read_pos;   //返回相对位置
		
	}
	size_t consume(size_t len) //取出字符
	{
		auto temp = size();
		len = len > temp ? temp : len;
		_read_pos += len;
		return len;
	}
	
	string read(size_t len)
	{
		auto temp = size();
		len = len > temp ? temp : len;
		string str(&(*begin()), len);
		consume(len); 
		return str;
	}
	const char &operator[](size_t i)
	{
		return _buffer[_read_pos + i]; 
	}
private:
	vector<char> _buffer;
	size_t _read_pos; //表示buffer中开始位置
	size_t _write_pos; //表示buffer中字符串结束的下一个位置
	
};
}

#endif
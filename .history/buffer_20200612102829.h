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
	Buffer(size_t size = 1024);
	size_t getReadPos();
	size_t length(int len);
	size_t capacity() const;
	size_t size() const;
	size_t writeSize();
	bool empty() const;


	void reset()  //重置read write位置
	{
		_read_pos = _write_pos = 0;
	}
	
	const char * begin() const;
	const char * end() const; 
	
	void append(const char *str, size_t len);
	
	inline
	void append(const string &str);
	
	int find();
	size_t consume(size_t len); //取出字符
	
	
	string read(size_t len);
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
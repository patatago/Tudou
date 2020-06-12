#include "buffer.h"
using namespace tudou;
Buffer::Buffer(size_t size)
: _buffer(size)
, _read_pos(0)
, _write_pos(0)
{}

size_t Buffer::getReadPos() const { return _read_pos; }
size_t Buffer::length(int len) const { return len - _read_pos; }
size_t Buffer::capacity() const { return _buffer.size(); }
size_t Buffer::size() const { 	return _write_pos - _read_pos; }
size_t Buffer::writeSize() const { return _buffer.size() - _write_pos; }
bool Buffer::empty() const { return size() == 0; }


void Buffer::reset()  //重置read write位置
{
    _read_pos = _write_pos = 0;
}

const char * Buffer::begin() const { return &(*_buffer.begin()) + _read_pos; }
const char * Buffer::end() const { return &(*_buffer.begin()) + _read_pos + size(); }

void Buffer::append(const char *str, size_t len)
{
    
    if(size() == 0) //缓冲区全部读完了就重置
    {
        reset();
    }
    if(writeSize() + _read_pos < len) 
    {
        auto buffersize = size();
        _buffer.resize(2 * max(buffersize, len) ); 
        
    }
    std::copy(begin(), end(), &(*_buffer.begin()));  //将原来的数据拷贝
    _write_pos = size();
    _read_pos = 0; //更新后的位置
    for(size_t i=0; i<len; ++i)
    {
        _buffer[_write_pos++] = str[i];
    }
}
inline
void Buffer::append(const string &str)
{
    append(str.c_str(), str.size());
}

int Buffer::find()
{
    if(!_buffer.size())
    {
        return -1;
    }
    for(int i = _read_pos; i < _write_pos; ++i)
    {
        if(_buffer[i] == '\r' && _buffer[i+1] == '\n' && i+1 < _write_pos)
        {
            return i;
        }
    }
    return -1;

    // auto pos = S.find(str);
    // std::cout << "third = " <<S<< std::endl<<"----------"<<std::endl;;
    // if(pos == string::npos) { return -1; }
    // return pos + _read_pos;   //返回相对位置
    
}
size_t Buffer::consume(size_t len) //取出字符
{
    auto temp = size();
    len = len > temp ? temp : len;
    _read_pos += len;
    return len;
}

string Buffer::read(size_t len)
{
    auto temp = size();
    len = len > temp ? temp : len;
    string str(&(*begin()), len);
    consume(len); 
    return str;
}

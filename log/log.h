#ifndef __TUDOU_LOG_H__
#define __TUDOU_LOG_H__

#include <string>
#include "blockqueue.h"
#include <thread>
#include "../util.h"
#include "../threadgroup.h"
#include <memory>
#include <iostream>
#include <string>
#include <utility>
#include <thread>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <list>
#include <fstream>

using std::list;
using std::string;
using std::shared_ptr;
using std::ofstream;
namespace tudou
{
//class LogFromat;
//class FileLogAppender;
class Log;


//日志级别
class LogLevel
{
public:
	enum Level
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};
	string toString(LogLevel::Level level)
	{
		switch(level)
		{
		case DEBUG:
			return "[DEBUG]: ";
		case INFO:
			return "[INFO]: ";
		case WARN:
			return "[WARN]: ";
		case ERROR:
			return "[ERROR]: ";
		case FATAL:
			return "[FATAL]: ";
		default:
			return "[UNKNOW]: ";
		}
		return "[UNKNOW]: ";
	}
};


/*日志事件*/
class LogEvent
{
public:
	using Ptr = shared_ptr<LogEvent>;
	LogEvent(LogLevel::Level level, const string &file, const string &function, const int &line);
	pair<string, struct tm> printTime();
	void format();
	
	LogEvent &operator<< (const char * data);
	//endl 是一个函数，pf为函数指针
	LogEvent& operator<<(ostream& (*pf)(ostream &)); //改变endl行为

	~LogEvent();
	const string getFullText();
	LogLevel::Level getLevel();
private:
	int _line = 0; //行号
	stringstream _full_text;
	stringstream _message; //日志信息
	string _function; //函数名
	LogLevel::Level _level;
	string _file; //文件名
	uint64_t _time; //时间戳
	thread::id _thread_id; //线程id
	//FileLogAppender::Ptr _appender;
	
};


/*输出到文件*/
class FileLogAppender
{
	friend class Log;
public:
	using Ptr = shared_ptr<FileLogAppender>;
	FileLogAppender(const string filename, const string path);
	
	void openFile();
	
	void close();
	void set(const string &filename, const string &path);
	~FileLogAppender();
private:
	string _path;
	string _filename;
	ofstream _file_stream;
};



class Log 
: noncopyable
{

public:	
	//~Log(){};
	static Log &getInstance(); //单例
	
	~Log();
private:
	Log();
public:
	void setLineCount(int count);
	void log(LogEvent &event);
	void writeLog();
	void setLogLevel(LogLevel::Level level);
private:
	mutex _mutex_set;
	BlockQueue _log_list;
	ThreadGroup _log_thread_group;
	LogLevel::Level _level;
	static unique_ptr<Log> _pInstance;
	FileLogAppender::Ptr _appender; //appender集合
	string _filename;
	size_t _line_count = 0;  //log行数
	size_t _split_line = 10000;  //满split则分割
	int64_t _log_day = -1; //上次的日志是在哪一天
	int64_t _log_month = -1;
	int64_t _log_year = -1;
};

//unique_ptr<Log> Log::_pInstance = nullptr;
#define Trace LogEvent(LogLevel::Trace, __FILE__,__FUNCTION__, __LINE__)
#define Debug LogEvent(LogLevel::DEBUG, __FILE__,__FUNCTION__, __LINE__)
#define Info LogEvent(LogLevel::INFO, __FILE__,__FUNCTION__, __LINE__)
#define Warn LogEvent(LogLevel::WARN, __FILE__,__FUNCTION__, __LINE__)
#define Error LogEvent(LogLevel::ERROR, __FILE__,__FUNCTION__, __LINE__)
#define Fatal LogEvent(LogLevel::FATAL, __FILE__,__FUNCTION__, __LINE__)
#define Loglevel(level) Log::getInstance().setLogLevel(level)

}

#endif
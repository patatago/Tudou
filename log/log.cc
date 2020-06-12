#include "log.h"

using namespace tudou;


//	using Ptr = shared_ptr<LogEvent>;
LogEvent::LogEvent(LogLevel::Level level, const string &file, const string &function, const int &line)
: _file(file)
, _level(level)
, _function(function)
, _line(line)
, _thread_id(this_thread::get_id())
{
	_full_text.clear();
	_message.clear();
}

pair<string, struct tm> LogEvent::printTime()
{
	struct timeval _tv;
	gettimeofday(&_tv, NULL); //时间信息
	struct tm _my_tm;
	localtime_r(&_tv.tv_sec, &_my_tm);
	char _time_buf[1024]={};
	gettimeofday(&_tv, NULL); //时间信息
	//年-月-日-天-时-分
	snprintf(_time_buf, 1024, "%04d-%02d-%02d_%02d:%02d:%02d"
			, _my_tm.tm_year + 1900
			, _my_tm.tm_mon + 1, _my_tm.tm_mday, _my_tm.tm_hour
			, _my_tm.tm_min, _my_tm.tm_sec);
	return make_pair(_time_buf, _my_tm);
}
void LogEvent::format()
{
	if(_message.str().size() == 0)
	{
		return ;
	}
	auto path = getPath();
	_full_text << LogLevel().toString(_level);
	string name = path.substr(path.rfind('/') + 1);
	_full_text << name << "(" << _thread_id << ")" << " " << _file << " " 
		<< _function << " " << _line << " " << std::endl;
	//cout << endl << _full_text.str() << endl;
	_full_text << _message.str() << std::endl;
}

LogEvent::~LogEvent() {}
const string LogEvent::getFullText()
{
	format();
	return _full_text.str();
}

LogEvent &LogEvent::operator<<(const char * data)
{
	if(data)
	{
		_message << data;
	}
	return *this;
}

LogLevel::Level LogEvent::getLevel() { return _level; }

LogEvent& LogEvent::operator<<(ostream& (*pf)(ostream &)) //改变endl行为
{
	Log::getInstance().log(*this);
	return *this;
}




FileLogAppender::FileLogAppender(const string filename = "", const string path = "")
: _filename(filename)
, _path(path)
{}


void FileLogAppender::openFile()
{
	if(_file_stream.good()) //如果流打开就关闭
	{
		_file_stream.close();
	}
	_file_stream.open(_filename, ios::out | ios::app); //打开文件	
}


//const ofstream & getFileStream() { return _file_stream; }

void FileLogAppender::close()
{
	if(_file_stream.good())
	{
		_file_stream.close();
	}
}

void FileLogAppender::set(const string & filename, const string & path)
{
	_filename = filename;
	_path = path;
}

FileLogAppender::~FileLogAppender() 
{
	close();
}


Log & Log::getInstance() //单例
{
	mutex _mutex;
	if(_pInstance == nullptr)
	{
		lock_guard<mutex> lock(_mutex);
		if(_pInstance == nullptr)
		{
			_pInstance.reset(new Log());
		}
	}
	return *_pInstance;
}

Log::~Log()
{
	_log_thread_group.joinAll();
}
Log::Log()
: _appender(new FileLogAppender())
{
	
	_log_thread_group.createThread(bind(&Log::writeLog, this));
}

void Log::setLineCount(int count)
{
	_line_count = count;
}
//virtual ~Log(){}
void Log::log(LogEvent &event)
{
	if(event.getLevel() < _level)
	{
		return ;
	}
	auto ret = event.printTime();
	if(_log_day != ret.second.tm_mday || _log_month != ret.second.tm_mon+1 || _log_year != ret.second.tm_year+1900)
	{
		_log_day = ret.second.tm_mday;
		_log_month = ret.second.tm_mon + 1;
		_log_year = ret.second.tm_year + 1900;
		_filename = "TudouLog" + to_string(_log_year) + to_string(_log_month) + to_string(_log_day) + "_0.log";
		setLineCount(0); //新一天
	}
	else
	{
		if(_line_count % _split_line == 0)
		{
			_filename = "TudouLog" + to_string(_log_year) + to_string(_log_month) + to_string(_log_day) + "_" + to_string(_line_count/_split_line) + ".log";
		}
	}
	++_line_count; 
	_log_list.push(make_pair(event.getFullText(), _filename)); //存入队列
}

void Log::setLogLevel(LogLevel::Level level)
{
	_level = level;
}
void Log::writeLog()
{
	while(1)
	{
		pair<string, string> ret;
		auto flag = _log_list.pop(ret, 5);

		if(flag)
		{	
			lock_guard<mutex> lock(_mutex_set);
			_appender->set(ret.second, getPath());
			_appender->openFile();
			_appender->_file_stream << ret.first;
			_appender->_file_stream.flush();
		}
	}
}

unique_ptr<Log> Log::_pInstance = nullptr;
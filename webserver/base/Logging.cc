// Author Fangping Liu
// 2019-08-29

#include "CurrentThread.h"
#include "Logging.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

namespace lfp
{
namespace detail
{
	//默认输出到标准输出
	void defaultOutput(const char* msg, int len){
		size_t n = fwrite(msg, 1, len, stdout);
		(void)n;
	}
	void defaultFlush() {
		fflush(stdout);
	}

	Logger::OutputFunc g_output = defaultOutput;
	Logger::FlushFunc g_flush = defaultFlush;

} //end of namespace detail

} //end of namespace lfp


using namespace lfp;


Logger::Impl::Impl(const char* file, int line)
  : stream_(),
  	line_(line),
	basename_()
{
	const char* slash = strrchr(file, '/');  //找到file中最后一个/字符
	if (slash) {
		++slash;
		basename_ = std::string(slash, strlen(slash));
	}
	else {
		basename_ = std::string(file, strlen(file));
	}

	formatTime();  //格式化时间到缓冲区
	CurrentThread::tid();
	stream_ << CurrentThread::tidString(); //格式化线程ID到缓冲区
}

void Logger::Impl::formatTime()
{
	char buf[32];
	memset(buf, 0, sizeof buf);

	struct timeval tv;
	gettimeofday(&tv, nullptr);
	time_t time = tv.tv_sec;
	int microseconds = static_cast<int>(tv.tv_usec);

	struct tm* ptm = localtime(&time);
	strftime(buf, sizeof buf, "%Y.%m.%d-%H:%M:%S", ptm);
    stream_ << buf;
	snprintf(buf, sizeof buf, ".%06d ", microseconds);
	stream_ << buf;	
}

//输出文件名和行号
void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}


Logger::Logger(const char* file, int line, bool isAsync)
  : impl_(file, line),
  	isAsync_(isAsync)
{
	//异步日志和同步日志的以上步骤均相同，仅写入文件时的操作不同
}


Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());

	if (isAsync_) {  //异步写入
		Singleton<AsyncLogging>::instance().append(buf.data(), buf.length());
	}
	else {  //同步写入
		detail::g_output(buf.data(), buf.length());
		detail::g_flush();
	}
}

void Logger::setOutput(OutputFunc func) { detail::g_output = func; }

void Logger::setFlush(FlushFunc func) { detail::g_flush = func; }
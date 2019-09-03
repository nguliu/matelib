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

__thread char t_time[32];			//保存当前线程格式化的时间（年.月.日-时.分.秒）
__thread time_t t_lastCache = 0;	//上一次格式化时间的秒

//这里有一个技巧：在一秒内写入两条日志时，秒及以上的时间都不用再次格式化，因为在一秒之内
//它们肯定都没有变，在前端线程日志量巨大是对性能有极大地提升。（亲自体会过，血淋淋的教训）

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
	stream_ << CurrentThread::tidString() << ">>>"; //格式化线程ID到缓冲区
}

void Logger::Impl::formatTime()
{
	struct timeval tv;
	::gettimeofday(&tv, nullptr);
	time_t sec = tv.tv_sec;
	int usec = static_cast<int>(tv.tv_usec);

	//如果是新的1秒才格式化秒及以上时间
	if (t_lastCache != sec) {
		t_lastCache = sec;
		struct tm* ptm = localtime(&sec);
		strftime(t_time, sizeof t_time, "%Y.%m.%d-%H:%M:%S", ptm);
	}
    stream_ << t_time;
	
	char buf[16];
	snprintf(buf, sizeof buf, ".%06d ", usec);
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
	//在以上格式化的过程中，所有的数据都会在调用FixedBuffer::append时被拷贝一次，
	//此时是拷贝到前端线程的格式化缓冲区中，这一次拷贝无法避免。
}


Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());

	if (isAsync_) {  //异步写入
		Singleton<AsyncLogging>::instance().append(buf.data(), buf.length());

		//这里又会有一次内存拷贝，这里是从前端线程的格式化缓冲区拷贝到后端线程的应用层缓冲区
		//这一次拷贝可以避免，留做优化。
	}
	else {  //同步写入
		detail::g_output(buf.data(), buf.length());
		detail::g_flush();
	}
}

void Logger::setOutput(OutputFunc func) { detail::g_output = func; }

void Logger::setFlush(FlushFunc func) { detail::g_flush = func; }
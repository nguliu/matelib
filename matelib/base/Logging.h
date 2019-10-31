// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_LOGGING_H
#define MATELIB_BASE_LOGGING_H

#include "AsyncLogging.h"
#include "LogStream.h"
#include "noncopyable.h"
#include "Singleton.h"
#include <string>

namespace lfp
{
	class Logger : noncopyable
	{
	public:
		enum LogLevel {
			DEBUG,
			INFO,
			WARN,
			ERROR,
			NONLOG
		};

		Logger(const char* file, int line, LogLevel level, bool isAsync);
		~Logger();

		LogStream& stream() { return impl_.stream_; }

		static LogLevel logLevel() {
			return logLevel_;
		}
		static void setLogLevel(LogLevel level) {
			logLevel_ = level;
		}

		//设置同步日志输出方法
		typedef void (*OutputFunc)(const char* msg, int len);
		typedef void (*FlushFunc)();
		static void setOutput(OutputFunc);
		static void setFlush(FlushFunc);

	private:
		class Impl
		{
		public:
			Impl(const char* file, int line, LogLevel level);
			void formatTime();
			void finish();

			LogStream stream_;
			int line_;
			std::string basename_; //这里保存的只是文件名，不含路径
		};
	private:
		Impl impl_;
		bool isAsync_;  //是否为异步日志
		static LogLevel logLevel_;
	};

//根据savedErrno返回错误描述
const char* strerror_tl(int savedErrno);


//日志输出接口。STDLOG_*为同步日志，默认输出到标准输出，其余为异步日志
#define STDLOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) Logger(__FILE__, __LINE__, Logger::DEBUG, false).stream()
#define STDLOG_INFO  if (Logger::logLevel() <= Logger::INFO)  Logger(__FILE__, __LINE__, Logger::INFO, false).stream()
#define STDLOG_WARN  if (Logger::logLevel() <= Logger::WARN)  Logger(__FILE__, __LINE__, Logger::WARN, false).stream()
#define STDLOG_ERROR if (Logger::logLevel() <= Logger::ERROR) Logger(__FILE__, __LINE__, Logger::ERROR, false).stream()

#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) Logger(__FILE__, __LINE__, Logger::DEBUG, true).stream()
#define LOG_INFO  if (Logger::logLevel() <= Logger::INFO)  Logger(__FILE__, __LINE__, Logger::INFO, true).stream()
#define LOG_WARN  if (Logger::logLevel() <= Logger::WARN)  Logger(__FILE__, __LINE__, Logger::WARN, true).stream()
#define LOG_ERROR if (Logger::logLevel() <= Logger::ERROR) Logger(__FILE__, __LINE__, Logger::ERROR, true).stream()


//设置异步日志属性/停止异步日志线程
#define SET_ASYNCLOG_BASENAME(name)	  Singleton<AsyncLogging>::instance().setBaseName(name)
#define SET_ASYNCLOG_ROLLSIZE(size)   Singleton<AsyncLogging>::instance().setRollSize(size)
#define ASYNCLOG_STOP()				  Singleton<AsyncLogging>::instance().stop()


} // namespace lfp

#endif // !MATELIB_BASE_LOGGING_H
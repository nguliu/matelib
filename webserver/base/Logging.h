// Author Fangping Liu
// 2019-08-29

#ifndef WEBSERVER_BASE_LOGGING_H
#define WEBSERVER_BASE_LOGGING_H

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
		Logger(const char* file, int line, bool isAsync);
		~Logger();

		LogStream& stream() { return impl_.stream_; }

		typedef void (*OutputFunc)(const char* msg, int len);
		typedef void (*FlushFunc)();
		static void setOutput(OutputFunc);
		static void setFlush(FlushFunc);

	private:
		class Impl
		{
		public:
			Impl(const char* file, int line);
			void formatTime();
			void finish();

			LogStream stream_;
		private:
			int line_;
			std::string basename_; //这里保存的只是文件名，不含路径
		};
	private:
		Impl impl_;
		bool isAsync_;  //是否为异步日志
	};


//日志输出接口。注意这里__FILE__是文件的绝对路径
#define SYNC_LOG  Logger(__FILE__, __LINE__, false).stream() //同步日志
#define ASYNC_LOG Logger(__FILE__, __LINE__, true).stream()  //异步日志


//设置异步日志属性
#define SET_ASYNCLOG_BASENAME(name) Singleton<AsyncLogging>::instance().setBaseName(name)
#define SET_ASYNCLOG_ROLLSIZE(size) Singleton<AsyncLogging>::instance().setRollSize(size)
//停止异步日志线程
#define ASYNCLOG_STOP Singleton<AsyncLogging>::instance().stop()

} //end of namespace lfp

#endif //end of WEBSERVER_BASE_LOGGING_H
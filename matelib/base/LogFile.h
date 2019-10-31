// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_LOGFILE_H
#define MATELIB_BASE_LOGFILE_H

#include "MutexLock.h"
#include "noncopyable.h"
#include <memory>
#include <string>

namespace lfp
{
	// 日志滚动实现，通过时间驱动(每天0点)或写满规定大小驱动，
	// 用户选择性的设置是否需要保证线程安全，兼顾了效率和灵活性
	class File;  //前向声明

	class LogFile : noncopyable
	{
	public:
		LogFile(const std::string& basename,
				size_t rollSize,		  //日志文件滚动大小
				int flushInterval = 3,	  //日志冲刷时间默认为3秒
				bool threadSafe = true);  //默认设置为线程安全的
		~LogFile();

		void append(const char* data, int len);
		void flush();

	private:
		void append_unlocked(const char* data, int len); //无锁的追加到文件
		void rollFile();	//滚动日志文件

		static std::string getLogFileName(const std::string& basename, time_t* now);

		const std::string basename_;
		const size_t rollSize_;
		const int flushInterval_;
		std::unique_ptr<MutexLock> pmutex_;

		time_t startOfPeriod_;	//开始记录日志的时间，自动对齐到当天零点
		time_t lastRoll_;		//上一次滚动日志的时间
		time_t lastFlush_;		//上一次冲刷日志缓冲区的时间
		int count_;				//写日志计数器

		std::unique_ptr<File> file_; //指向当前日志文件

		const static int checkEveryN_ = 1000;
		const static int kRollPerSeconds_ = 24 * 60 * 60;  //驱动日志滚动的时间长度(24h * 60m * 60s)
	};

} // namespace lfp

#endif // !MATELIB_BASE_LOGFILE_H
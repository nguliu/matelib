// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include "CurrentThread.h"
#include "LogFile.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

namespace lfp
{
	// File使用RAII机制对日志文件进行封装，它操作具体的日志文件
	// File类不保证线程安全，线程安全由上层的LogFile保证

	class File : public noncopyable
	{
	public:
		explicit File(const std::string& filename)
		  : fp_(::fopen(filename.data(), "ae")),
			writtenBytes_(0)
		{
			//用户为fp_提供系统输入缓冲区
			setbuffer(fp_, buf_, sizeof buf_);
		}

		~File() {
			::fclose(fp_);
		}

		void append(const char* data, int len)
		{
			size_t n = write_unlocked(data, len);
			size_t remain = len - n;  //剩余字节数

			while (remain > 0) {
				size_t x = write_unlocked(data + n, remain);
				if (x == 0) {
					int err = ferror(fp_);
					if (err) {
						fprintf(stderr, "LogFile::File::append() failed:%s\n", ::strerror(err));
					}
					break;
				}
				n += x;
				remain -= x;
			}

			writtenBytes_ += n;
		}

		void flush() {
			::fflush(fp_);
		}

		int filefd() {
			return ::fileno(fp_);
		}

		size_t writtenBytes() const { return writtenBytes_; }

	private:
		size_t write_unlocked(const char* data, size_t len)
		{
			return ::fwrite_unlocked(data, 1, len, fp_);  //写入len个单位长度为1的单元，返回写入的单元数

			//这里没有采用线程安全的fwrite，而是采用了效率更高的非线程安全的写入方式，因为线程
			//安全性已由LogFile的threadSafe参数保证，这里没必要在使用线程安全的方法
		}

		FILE* fp_;			  //日志文件
		char buf_[64*1024];	  //提供给fp_作为系统输入缓冲区
		size_t writtenBytes_; //已写入字节数
	};

}


using namespace lfp;


LogFile::LogFile(const std::string& basename,
				 size_t rollSize,	  //日志文件滚动大小
				 int flushInterval,	  //日志冲刷时间默认为3秒
				 bool threadSafe)	  //默认设置为线程安全的
  : basename_(basename),
  	rollSize_(rollSize),
	flushInterval_(flushInterval),
	pmutex_(threadSafe ? new MutexLock : nullptr),  //是否为线程安全
	startOfPeriod_(0),  //开始记录日志的时间（对齐到当天零点）
	lastRoll_(0),
	lastFlush_(0),
	count_(0)
{
	rollFile();	//这里滚动一下日志，产生第一个日志文件
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* data, int len)
{
	//根据pmutex_决定是否需要保证线程安全
	if (pmutex_) {
		MutexLockGuard lock(*pmutex_);
		append_unlocked(data, len);
	}
	else {
		append_unlocked(data, len);
	}
}

void LogFile::flush()
{
	//根据pmutex_决定是否需要保证线程安全
	if (pmutex_) {
		MutexLockGuard lock(*pmutex_);
		file_->flush();
	}
	else {
		file_->flush();
	}
}

void LogFile::append_unlocked(const char* data, int len)
{
	file_->append(data, len);

	if (file_->writtenBytes() > rollSize_) {
		rollFile();
		count_ = 0;
	}
	else {
		++count_;

		if (count_ >= checkEveryN_)	//达到checkEveryN_次才检查是否需要滚动或冲刷
		{
			count_ = 0;
			time_t now = ::time(NULL);
			time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;	//将写入的时间对齐到当天零点

			if (thisPeriod != startOfPeriod_) {	//如果是新的一天则滚动一下日志
				rollFile();
			}
			else if (now - lastFlush_ > flushInterval_) {  //如果冲刷时间间隔超过3s则进行冲刷
				lastFlush_ = now;
				file_->flush();  //注意这里不能调用flush()，否则在线程安全的情况下会造成死锁
			}
		}
	}
}

void LogFile::rollFile()  //滚动日志
{
	time_t now = 0;
	std::string filename = getLogFileName(basename_, &now);
	/*这里请注意，如果rollsize较小导致在1s内写满了一个文件，那么这里生成的日志文件名会和上一个文件
	  同名（因为文件名只精确到秒），导致实际写入的还是上一个文件（这是正是File中要用"ae"打开的原因）
	 */

	lastRoll_ = now;
	lastFlush_ = now;
	//开始记录日志的时间对齐到当天零点
	startOfPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;

	file_.reset(new File(filename));
	//这里使file_指向一个新的File对象, 上一个File对象被销毁且保存日志文件

	/*write a flag at file start*/
	char buf[64];
	snprintf(buf, sizeof buf, "Writed by thread %d\nlogfile fd = %d\n", CurrentThread::tid(), file_->filefd());
	file_->append(buf, strlen(buf));
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
	std::string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char buf[32];

	struct tm* ptm;
	*now = time(NULL);   // UTC时间戳
	ptm = localtime(now);
	strftime(buf, sizeof buf, ".%Y%m%d-%H%M%S.", ptm);

	filename += buf;
	::gethostname(buf, sizeof buf);
	filename += buf;

	snprintf(buf, sizeof buf, ".p%d", ::getpid());
	filename += buf;
	filename += ".log";

	return filename;
}
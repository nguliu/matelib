// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_ASYNCLOGGING_H
#define MATELIB_BASE_ASYNCLOGGING_H

#include "Condition.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "noncopyable.h"
#include "Thread.h"
#include <assert.h>
#include <memory> //unique_ptr
#include <string>
#include <vector>

namespace lfp
{
	class AsyncLogging : noncopyable
	{
	public:
		AsyncLogging(std::string basename = "asynclog",
					 size_t rollSize = 500 * 1024 * 1024,	//日志文件滚动大小默认为500M
					 int flushInterval = 3);	//日志冲刷时间默认为3秒
		~AsyncLogging()
		{
			if (running_) {
				stop();
			}
		}

		//供前端生产者线程调用（将日志数据写到应用层缓冲区中）
		void append(const char* data, int len);

		void stop()
		{
			running_ = false;
			cond_.notify();		//消费者线程可能处于阻塞等待状态，需要唤醒
			thread_.join();
		}

		void setBaseName(const std::string& name) { basename_ = name; }
		void setRollSize(size_t size) { rollSize_ = size; }

	private:
		void start()  //启动后台消费者线程
		{
			if (!running_) {
				running_ = true;
				thread_.start();  //在这里面前台线程会等待后台线程启动完毕才能继续执行
			}
		}

		// 供后端消费者线程调用（将数据从应用层缓冲区写到日志文件）
		void threadFunc();
		
		typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;  //4M的应用层缓冲区
		typedef std::unique_ptr<Buffer> BufferPtr;	//只具备移动语义，不能进行复制操作只能进行移动操作
		typedef std::vector<BufferPtr> BufferVector;

		const int flushInterval_; // 超时时间，在flushInterval_秒后，缓冲区没写满，仍将缓冲区中的数据写到文件中
		bool running_;
		std::string basename_;	//日志文件basename
		size_t rollSize_;		//文件滚动大小
		Thread thread_;			//后台消费者线程
		MutexLock mutex_;
		Condition cond_;
		BufferPtr currentBuffer_;	// 当前缓冲区
		BufferPtr nextBuffer_;		// 预备缓冲区
		BufferVector fullBuffers_;	// 待写入文件的已填满的缓冲区
	};

} // namespace lfp

#endif // !MATELIB_BASE_ASYNCLOGGING_H
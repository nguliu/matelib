// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"
#include <functional>

using namespace lfp;


AsyncLogging::AsyncLogging(std::string basename,
							size_t rollSize,	 //日志文件滚动大小默认为500M
							int flushInterval)	 //日志冲刷时间默认为3秒
  : flushInterval_(flushInterval),
  	running_(false),
	basename_(basename),
	rollSize_(rollSize),
	thread_(std::bind(&AsyncLogging::threadFunc, this), "AsyncLogThread"),
	mutex_(),
	cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    fullBuffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	fullBuffers_.reserve(16);
	
	start();
}

//这里会有一次内存拷贝
//供前端生产者线程调用（将数据从前端线程格式化缓冲区拷贝到后端线程应用层缓冲区）
void AsyncLogging::append(const char* data, int len)
{
	MutexLockGuard lock(mutex_);  //生产者线程与消费者线程互斥访问缓冲区

	if (currentBuffer_->avail() > len) {
		//当前缓冲区未满，直接追加数据到当前缓冲区
		currentBuffer_->append(data, len);
	}
	else
	{
		//当前缓冲区已满，将当前缓冲区移动到待写入文件的已满缓冲区列表
		//move语意，原来的unique_ptr被移动到buffers_中，currentBuffer_被置空
		fullBuffers_.push_back(std::move(currentBuffer_));

		if (nextBuffer_) {
			//如果预备缓冲区还没使用则将其设为当前缓冲区
			//move语意，nextBuffer_被置空
			currentBuffer_ = std::move(nextBuffer_);
		}
		else {
			//极少遇到的情况：前端写入速度太快，一下把两块缓冲区都写完，只好分配一块新的缓冲区。
			currentBuffer_.reset(new Buffer);
		}

		currentBuffer_->append(data, len);
		cond_.notify();	 //通知后台线程将日志写入文件
	}
}

// 供后端消费者线程调用（将数据从应用层缓冲区写到日志文件）
void AsyncLogging::threadFunc()
{
	// 准备两块空闲的预备缓冲区
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite; //用于写入文件时暂存fullBuffers_的内容，减小临界区的长度
	buffersToWrite.reserve(16);

	//写入到日志文件时只有一个后台线程进行写入，因此不用保证线程安全
	LogFile logfile(basename_, rollSize_, flushInterval_, false);

	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{ //临界区
			MutexLockGuard lock(mutex_);
			while (fullBuffers_.empty() && running_)
			{
				//等待前端写满了一个或多个buffer，或者一个冲刷时间到来
				cond_.waitForSeconds(flushInterval_);
				//如果在一个flushInterval_时间内有线程写入一些数据到缓冲区但又未写满也要及时写入文件
				if (currentBuffer_->length() > 0) {
					break;
				}
			}

			fullBuffers_.push_back(std::move(currentBuffer_)); //move语意
			//fullBuffers_与buffersToWrite交换，这样后面的代码可以在临界区之外安全地访问buffersToWrite
			buffersToWrite.swap(fullBuffers_);
			
			//保证前端有可用的缓冲区
			currentBuffer_ = std::move(newBuffer1);
			if (!nextBuffer_) {
				nextBuffer_ = std::move(newBuffer2);
			}
		}

		// 消息堆积
		// 前端陷入死循环，不断的写入日志消息，超过后端的处理能力，这就是典型的生产速度
		// 超过消费速度问题，会造成数据在内存中堆积，严重时引发性能问题（可用内存不足）
		// 或程序崩溃（分配内存失败）。此时应该扔掉多余日志并输出一条错误消息
		if (buffersToWrite.size() > 25) {
			char buf[128];
			snprintf(buf, sizeof buf, "Discard log messages at %s, %zd larger buffers\n",
					 Timestamp::now().toFormattedString().c_str(), buffersToWrite.size() - 2);

			fputs(buf, stderr);
			logfile.append(buf, static_cast<int>(strlen(buf)));
			
			// 丢掉多余日志，以腾出内存，仅保留两块缓冲区
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}

		for (size_t i = 0; i < buffersToWrite.size(); ++i) {
			logfile.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
		}

		if (buffersToWrite.size() > 2) {
			buffersToWrite.resize(2);  //仅保留两个buffer用于填充newBuffer1和newBuffer2
		}

		if (!newBuffer1) {
			newBuffer1 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer1->reset(); //(*newBuffer1).reset();
		}
		if (!newBuffer2) {
			newBuffer2 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer2->reset(); //(*newBuffer2).reset();
		}

		buffersToWrite.clear();
		logfile.flush();
	}
	logfile.flush();
}
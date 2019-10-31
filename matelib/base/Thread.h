// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_THREAD_H
#define MATELIB_BASE_THREAD_H

#include "CountDownLatch.h"
#include "CurrentThread.h"
#include <functional>
#include <string>
#include <pthread.h>

namespace lfp
{
	class Thread : noncopyable
	{
	public:
		typedef std::function<void ()> ThreadFunc;

		explicit Thread(const ThreadFunc& func, const std::string& name = std::string());
		~Thread();

		void start();
		int join();	//return pthread_join()

		bool started() { return started_; }
		pthread_t pthreadId() const { return pthreadId_; }
		pid_t tid() const { return tid_; }
		const std::string& name() { return name_; }

	private:
		//线程入口函数，只有在入口函数之后的调用栈中调用的函数才是在新线程中执行
		static void* startThread(void* thread);
		void runInThread(); //执行具体任务

		bool			started_;
		pthread_t		pthreadId_;
		pid_t			tid_;	//线程全局真实ID
		ThreadFunc		func_;	//线程任务
		std::string		name_;	//线程名
		CountDownLatch	latch_;
	};

} // namespace lfp

#endif // !MATELIB_BASE_THREAD_H
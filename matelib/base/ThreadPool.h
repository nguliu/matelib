// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_THREADPOOL_H
#define MATELIB_BASE_THREADPOOL_H

#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"
#include "Thread.h"
#include <deque>
#include <functional> //std::function
#include <memory>
#include <string>
#include <vector>

namespace lfp
{
//线程池实现，本质也是生产者-消费者问题
	class ThreadPool : noncopyable
	{
	public:
		typedef std::function<void ()> Task;

		explicit ThreadPool(int t_num, const std::string& name = std::string("ThreadPool"));
		~ThreadPool();

		void start();
		void stop();
		void run(const Task& task);
	private:
		void runInThread();	//线程入口函数
		Task take();		//取任务

		MutexLock			mutex_;
		Condition			cond_;
		int					threadNum_;
		std::string			name_;
		bool				running_;
		std::deque<Task>	tasks_;	//任务队列
		std::vector<std::unique_ptr<Thread>> threads_;
	};

} // namespace lfp

#endif // !MATELIB_BASE_THREADPOOL_H
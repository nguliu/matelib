// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_EVENTLOOPTHREAD_H
#define MATELIB_EVENTLOOPTHREAD_H

#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"
#include "base/Thread.h"
#include "callbacks.h"
#include "EventLoop.h"
#include <functional>

namespace lfp
{
	class EventLoop;

	class EventLoopThread : noncopyable
	{
	public:
		EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
		~EventLoopThread();

		EventLoop* start();
	
	private:
		void threadFunc();

		bool quit_;
		EventLoop* loop_;
		Thread thread_;
		MutexLock mutex_;
		Condition cond_;
		ThreadInitCallback initCallBack_;	//线程初始化回调函数
	};

} // namespace lfp

#endif // !MATELIB_EVENTLOOPTHREAD_H
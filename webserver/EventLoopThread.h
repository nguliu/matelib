// Author Fangping Liu
// 2019-09-03

#ifndef WEBSERVER_EVENTLOOPTHREAD_H
#define WEBSERVER_EVENTLOOPTHREAD_H

#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"
#include "base/Thread.h"
#include "EventLoop.h"
#include <functional>

namespace lfp
{
	class EventLoop;

	class EventLoopThread : noncopyable
	{
	public:
		typedef std::function<void(EventLoop*)> ThreadInitCallback;
		
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
		ThreadInitCallback callBack_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_EVENTLOOPTHREAD_H
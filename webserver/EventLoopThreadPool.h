// Author Fangping Liu
// 2019-09-03

#ifndef WEBSERVER_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_EVENTLOOPTHREADPOOL_H

#include "base/noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace lfp
{
	class EventLoop;
	class EventLoopThread;

	class EventLoopThreadPool : noncopyable
	{
	public:
		typedef std::function<void(EventLoop*)> ThreadInitCallback;
		
		EventLoopThreadPool(EventLoop* baseloop, int threadNum);
		~EventLoopThreadPool();

		void start(const ThreadInitCallback& cb = ThreadInitCallback());

		EventLoop* getNextLoop();
	
	private:
		bool started_;
		int threadNum_;		//线程数
		int next_;			//新连接到来时选择EventLoop对象的下标
		EventLoop* baseloop_;
		std::vector<EventLoop*> loops_;	 //EventLoop列表
		std::vector<std::unique_ptr<EventLoopThread>> loopThreads_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_EVENTLOOPTHREADPOOL_H
// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_EVENTLOOP_H
#define MATELIB_EVENTLOOP_H

#include "base/CurrentThread.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"
#include "callbacks.h"
#include <memory>
#include <vector>


namespace lfp
{
	class Channel;	//前向声明
	class EpollPoller;
	class TimerQueue;
	class TimerId;

	class EventLoop : noncopyable
	{
	public:
		typedef std::function<void ()> Functor;

		EventLoop();
		~EventLoop();

		void loop();
		void quit();
		void runInLoop(const Functor& func);
		void queueInLoop(const Functor& func);

		//timer
		TimerId runAfter(double timeInSec, const TimerCallback& cb);
		TimerId runEvery(double intervalInSec, const TimerCallback& cb);
		void cancelTimer(TimerId timerId);
		
		//唤醒当前线程
		void wakeup();

		void updateChannel(Channel* channel);
		void removeChannel(Channel* channel);
		void assertInLoopThread();

		static EventLoop* getEventLoopOfCurrentThread();
		bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

	private:
		void handleWakeup();	//处理唤醒当前线程事件
		void doPendingFunctors();

		typedef std::vector<Channel*> ChannelList;

		bool looping_;
		bool quit_;
		bool eventHandling_;
		bool callingPendingFunctors_;
		MutexLock mutex_;
		const pid_t threadId_;	//当前对象所属的线程
		std::unique_ptr<EpollPoller> poller_;
		std::unique_ptr<TimerQueue> timerQueue_;
		int wakeupFd_;	//用于eventfd
		std::unique_ptr<Channel> wakeupChannel_;
		ChannelList activeChannels_;	  //当前活动的通道
		Channel* currentHandingChannel_;  // 当前正在处理的活动通道
		std::vector<Functor> pendingFunctors_;
	};

} // namespace lfp

#endif // !MATELIB_EVENTLOOP_H
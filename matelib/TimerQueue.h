// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_TIMERQUEUE_H
#define MATELIB_TIMERQUEUE_H

#include "base/copyable.h"
#include "base/noncopyable.h"
#include "base/Timestamp.h"
#include "callbacks.h"
#include "Channel.h"
#include <set>
#include <vector>


namespace lfp
{
	class EventLoop;
	class Timer;
	class TimerId;

	class TimerQueue : noncopyable
	{
	public:
		TimerQueue(EventLoop* loop);
		~TimerQueue();

		TimerId addTimer(const TimerCallback& cb,
						 Timestamp when,
						 double intervalInSec);

		void cancelTimer(TimerId timerId);

	private:
		typedef std::pair<Timestamp, Timer*> Entry;
		typedef std::set<Entry> TimerSet;

		void handleRead();		//定时器描述符可读的处理函数
		void addTimerInLoop(Timer* timer);
		void cancelTimerInLoop(TimerId timerId);

		std::vector<Entry> getExpired(Timestamp now);
		void reset(const std::vector<Entry>& expired, Timestamp now);

		EventLoop* loop_;
		const int timerfd_;
		Channel timerfdChannel_;
		TimerSet timers_;			//注册的定时器集合
		bool callingExpiredTimers_;	//是否正在处理到期定时器
		TimerSet cancelingTimers_;	//被取消的定时器
	};

} // namespace lfp

#endif // !MATELIB_TIMERQUEUE_H
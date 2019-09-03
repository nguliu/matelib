// Author Fangping Liu
// 2019-09-02

#ifndef WEBSERVER_TIMERQUEUE_H
#define WEBSERVER_TIMERQUEUE_H

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
		typedef std::set<Entry> TimerList;

		void addTimerInLoop(Timer* timer);
		void cancelTimerInLoop(TimerId timerId);
		void handleRead();	//定时器描述符可读的处理函数

		std::vector<Entry> getExpired(Timestamp now);
		void reset(const std::vector<Entry>& expired, Timestamp now);

		EventLoop* loop_;
		const int timerfd_;
		Channel timerfdChannel_;
		TimerList timers_;
		bool callingExpiredTimers_;	//是否正在处理到期定时器
		TimerList cancelingTimers_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_TIMERQUEUE_H
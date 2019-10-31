// Author Fangping Liu
// 2019-09-02

#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include "base/noncopyable.h"
#include "base/Timestamp.h"
#include "callbacks.h"


namespace lfp
{
	class Timer : noncopyable
	{
	public:
		Timer(const TimerCallback& cb, Timestamp when, double intervalInSec)
		  : callback_(cb),
			expiration_(when),
			intervalInSec_(intervalInSec),
			repeat_(intervalInSec_ > 0.0)
		{
		}

		void run() {
			callback_();
		}

		bool repeat() const { return repeat_; }

		Timestamp expiration() const  { return expiration_; }
		
		void restart(Timestamp now)
		{
			//如果是重复定时器，将当前时间加上时间间隔得到下一个超时时刻
			if (repeat_) {
				expiration_ = addTime(now, intervalInSec_);
			}
			else {	//否则将下一个超时时刻设置为一个非法时刻
				expiration_ = Timestamp::invalid();
			}
		}
	private:
		const TimerCallback callback_;		// 定时器回调函数
		Timestamp expiration_;				// 定时器超时时刻
		const double intervalInSec_;		// 超时时间间隔，如果是一次性定时器，该值为0
		const bool repeat_;					// 是否重复
	};

	
	//TimerId用于唯一标识一个定时器
	class TimerId : copyable
	{
		friend class TimerQueue;
	public:
		TimerId()
		  : when_(Timestamp::invalid()),
			timer_(nullptr)
		{
		}

		TimerId(Timestamp when, Timer* timer)
		  : when_(when),
			timer_(timer)
		{
		}

	private:
		Timestamp when_;	//定时器超时时刻
		Timer* timer_;		//定时器
	};


} //end of namespace lfp

#endif //end of WEBSERVER_TIMER_H
// Author Fangping Liu
// 2019-09-02

#include "base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerQueue.h"
#include <functional>
#include <sys/timerfd.h>
#include <unistd.h>

namespace lfp::detail
{
    // 创建定时器文件描述符
    int createTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            ASYNC_LOG << "::timerfd_create() error";
        }
        return timerfd;
    }

    // 计算when与当前时间的时间差
    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100)	 //最小定时100微秒
			microseconds = 100;

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);

        return ts;
    }

    //读取定时器文件描述符，避免一直触发
    void readTimerfd(int timerfd)
    {
        uint64_t howmany;
        ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
        ASYNC_LOG << "TimerQueue::handleRead(), readed " << howmany;

        if (n != sizeof howmany) {
            ASYNC_LOG << "TimerQueue::handleRead() error, reads " << n << "bytes instead of 8";
        }
    }

    // 注册定时器文件描述符超时时刻，这里注册的是一次性定时器
    void resetTimerfd(int timerfd, Timestamp expiration)
    {
        struct itimerspec newValue;
        bzero(&newValue, sizeof newValue);

        newValue.it_value = howMuchTimeFromNow(expiration);		//这里设置的是一次性定时器
        int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);

        if (ret) {
            ASYNC_LOG << "::timerfd_settime() error";
        }
    }

} //end of namespace lfp::detail


using namespace lfp;


TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
  	timerfd_(detail::createTimerfd()),
	timerfdChannel_(loop_, timerfd_),
	timers_(),
	callingExpiredTimers_(false),
	cancelingTimers_()
{
	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	::close(timerfd_);
	for (auto it = timers_.begin(); it != timers_.end(); ++it)
	{
		delete it->second;
	}
}

//线程安全的添加
TimerId TimerQueue::addTimer(const TimerCallback& cb,
							 Timestamp when,
							 double intervalInSec)
{
	Timer* timer = new Timer(cb, when, intervalInSec);
	loop_->runInLoop(
		std::bind(&TimerQueue::addTimerInLoop, this, timer));

	return TimerId(when, timer);
}

//线程安全的取消
void TimerQueue::cancelTimer(TimerId timerId)
{
	loop_->runInLoop(
		std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();
	bool earliestChanged = false;

	Timestamp when = timer->expiration();
	TimerList::iterator it = timers_.begin();
	if (timers_.empty() || when < it->first) {
		earliestChanged = true;
	}
	
	// 插入到timers_中
	std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
    assert(result.second); (void)result;

	if (earliestChanged) {
		//如果定时器列表最早到期时间改变，重置定时器的超时时刻
		detail::resetTimerfd(timerfd_, timer->expiration());
	}
}

void TimerQueue::cancelTimerInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	
	Entry entry(timerId.when_, timerId.timer_);
	TimerList::iterator it = timers_.find(entry);
	if (it != timers_.end()) {
		delete it->second;
		size_t n = timers_.erase(entry);
		assert(n == 1); (void)n;
	}
	else if (callingExpiredTimers_) //如果不在timers_列表中，可能是因为已经到期被移动到了expired中
	{								//此时需要加入取消列表，避免被重新加入到定时器列表
		cancelingTimers_.insert(entry);
	}
}

//定时器描述符可读回调函数，有定时器到期时被调用
void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();

	detail::readTimerfd(timerfd_);	 // 清除该事件，避免一直触发
	cancelingTimers_.clear();		 //清空被取消的定时器列表

	Timestamp now(Timestamp::now());

	// 在getExpired之前调用cancelInLoop会将定时器直接删除，在getExpired之后
	// 调用cancelInLoop会将定时器加入cancelingTimers_列表
	std::vector<Entry> expired = getExpired(now);

	callingExpiredTimers_ = true;

	for (auto it = expired.begin();
		 it != expired.end(); ++it)
	{
		it->second->run();
	}
	callingExpiredTimers_ = false;

	// 对于到期的非一次性定时器，需要重新加入定时器列表
	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));

	// 返回第一个未超时的Timer的迭代器
	// lower_bound的含义是返回第一个值>=sentry的元素的iterator
	// 即*end >= sentry，从而end->first > now
	TimerList::iterator end = timers_.lower_bound(sentry);
	assert(now < end->first || end == timers_.end());

	std::copy(timers_.begin(), end, back_inserter(expired));
	// 从timers_中移除到期的定时器
	timers_.erase(timers_.begin(), end);

	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;

	for (std::vector<Entry>::const_iterator it = expired.begin();
		 it != expired.end(); ++it)
	{
		Entry entry(it->first, it->second);

		// 如果是重复的定时器并且是未取消定时器，则重启该定时器
		if (it->second->repeat() &&
			cancelingTimers_.find(entry) == cancelingTimers_.end())
		{
			it->second->restart(now);
			timers_.insert(entry);
		}
		else {
			// 一次性定时器或者已被取消的定时器是不能重置的，因此删除该定时器
			delete it->second;
		}
	}

	if (!timers_.empty()) {
		// 获取最早到期的定时器超时时间
		nextExpire = timers_.begin()->second->expiration();
		// 重置定时器的超时时刻(timerfd_settime)
		detail::resetTimerfd(timerfd_, nextExpire);
	}
}
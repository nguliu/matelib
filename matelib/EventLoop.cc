// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include "base/Logging.h"
#include "base/Timestamp.h"
#include "Channel.h"
#include "EpollPoller.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerQueue.h"
#include <algorithm>  //std::find
#include <functional>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>


namespace lfp::detail
{
	//线程私有数据，对于IO线程指向其EventLoop对象
	__thread EventLoop* t_loopInThisThread = nullptr;

	const int kEpollWaitTimeMS = 20 * 1000;	 //epoll查询阻塞时间

	int createEventfd() {
		int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (evfd < 0) {
			int savedError = errno;
			LOG_ERROR << "syscall eventfd error: " << strerror_tl(savedError);
			abort();
		}
		return evfd;
	}

	class IgnoreSig
	{
	public:
		IgnoreSig()
		{
			::signal(SIGPIPE, SIG_IGN);
			::signal(SIGCHLD, SIG_IGN);

			LOG_INFO << "Ignore signal SIGPIPE and SIGCHLD";
		}
	};

	IgnoreSig initObj;
}


using namespace lfp;

EventLoop::EventLoop()
  : looping_(false),
  	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	mutex_(),
	threadId_(CurrentThread::tid()),
	poller_(new EpollPoller(this)),			//use a fd
	timerQueue_(new TimerQueue(this)),		//use a fd
	wakeupFd_(detail::createEventfd()),		//use a fd
	wakeupChannel_(new Channel(this, wakeupFd_)),
	activeChannels_(),
	currentHandingChannel_(nullptr),
	pendingFunctors_()
{
	//一个线程最多有一个EventLoop对象
	assert(detail::t_loopInThisThread == nullptr);
	detail::t_loopInThisThread = this;

	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
	wakeupChannel_->enableReading();

	LOG_INFO << "EventLoop: wakeupfd = " << wakeupFd_;
}

EventLoop::~EventLoop()
{
	::close(wakeupFd_);
	detail::t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();

	looping_ = true;
	while (!quit_)
	{
		activeChannels_.clear();
		//阻塞查询
		int numEvents = poller_->poll(detail::kEpollWaitTimeMS, &activeChannels_);
		if (numEvents > 0)		//进行事件分发处理
		{
//			STD_LOG << numEvents << " events happended in EventLoop::loop()";
			eventHandling_ = true;
			for (auto it = activeChannels_.begin();
				 it != activeChannels_.end();
				 ++it)
			{
				 currentHandingChannel_ = *it;
				 currentHandingChannel_->handleEvent();
			}
			currentHandingChannel_ = nullptr;
			eventHandling_ = false;
		}
		else {
//			STD_LOG << "Nothing happended in EventLoop::loop()";
		}

		doPendingFunctors(); //执行其他线程向当前线程添加的任务
	}
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::runInLoop(const Functor& func)
{
	if (isInLoopThread()) {
		func();
	}
	else {
		queueInLoop(func);
	}
}

void EventLoop::queueInLoop(const Functor& func)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(func);
	}

	//如果不是在当前线程放入或者正在处理任务需要唤醒，原因与doPendingFunctors的实现有关
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

TimerId EventLoop::runAfter(double timeInSec, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), timeInSec));
	return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runEvery(double intervalInSec, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), intervalInSec));
	return timerQueue_->addTimer(cb, time, intervalInSec);
}

void EventLoop::cancelTimer(TimerId timerId)
{
	timerQueue_->cancelTimer(timerId);
}

//唤醒当前线程
void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR << "EventLoop::wakeup() error";
	}
}

void EventLoop::handleWakeup()
{
	uint64_t one = 1;
	ssize_t n = ::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR << "EventLoop::handleWakeup() error";
	}
}

void EventLoop::updateChannel(Channel* channel)
{
	assertInLoopThread();
	assert(channel->ownerLoop() == this);
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	assertInLoopThread();
	assert(channel->ownerLoop() == this);
	if (eventHandling_)
	{
		assert(currentHandingChannel_ == channel ||
			   std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
		//这里如果在处理活动通道，要确保处理的正是当前通道（因对等方关闭、处理事件出错等原因需要关闭）
		//或当前channel处于不活跃状态才能移除当前通道
	}
	poller_->removeChannel(channel);
}

void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread()) {
		LOG_ERROR << "EventLoop[" << this << "] created by thread " << threadId_
				  << ",current thread is " <<  CurrentThread::tid();
		abort();
	}
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return detail::t_loopInThisThread;
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	//先swap再处理，为了缩小临界区
	//另一方面能避免死锁（如果直接处理pendingFunctors_并且其中某个函数又调用了queueInLoop(func)会造成死锁）
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	//在这之后可能又有线程放入任务到pendingFunctors_中，在此次doPendingFunctors调用中不会被执行，
	//因此queueInLoop中callingPendingFunctors_时需要唤醒

	for (size_t i = 0; i < functors.size(); ++i) {
		functors[i]();
	}

	callingPendingFunctors_ = false;
}
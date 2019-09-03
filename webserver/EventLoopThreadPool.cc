// Author Fangping Liu
// 2019-09-03

#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include <assert.h>

using namespace lfp;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, int threadNum)
  : started_(false),
  	threadNum_(threadNum),
	next_(0),
	baseloop_(baseloop)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
	assert(!started_);
	baseloop_->assertInLoopThread();

	for (int i = 0; i < threadNum_; ++i) {
		EventLoopThread* t = new EventLoopThread(cb);
		loopThreads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->start());
	}
	if (threadNum_ == 0) {
		cb(baseloop_);
	}

	started_ = true;
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseloop_->assertInLoopThread();
	EventLoop* loop = baseloop_;

	if (threadNum_ > 0) {
		loop = loops_[next_];
		next_ = (next_ + 1) % threadNum_;
	}

	return loop;
}
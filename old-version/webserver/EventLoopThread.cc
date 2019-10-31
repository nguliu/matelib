// Author Fangping Liu
// 2019-09-03

#include "EventLoopThread.h"

using namespace lfp;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : quit_(false),
  	loop_(nullptr),
	thread_(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
	mutex_(),
	cond_(mutex_),
	callBack_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
	quit_ = true;
	loop_->quit();
	thread_.join();  //当前线程等待loop线程结束
}

EventLoop* EventLoopThread::start()
{
	assert(!thread_.started());

	thread_.start();

	{	//启动线程必须等待新线程启动完成才能继续执行
		MutexLockGuard lock(mutex_);
		while (loop_ == nullptr) {
			cond_.wait();
		}
	}

	return loop_;
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;
	
	if (callBack_) {
		callBack_(&loop);
	}

	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();	//通知启动线程继续执行
	}

	loop.loop();
}
// Author Fangping Liu
// 2019-08-29

#include "ThreadPool.h"
#include <assert.h>
#include <stdio.h>

using namespace lfp;


ThreadPool::ThreadPool(int t_num, const std::string& name)
  : mutex_(),
  	cond_(mutex_),
	threadNum_(t_num),
	name_(name),
	running_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (running_) {
		stop();
	}
}

void ThreadPool::start()
{
	assert(!running_);
	running_ = true;

	//创建threadNum_个Thread对象保存到threads_
	for (int i = 0; i < threadNum_; ++i) {
		char buf[32];
		snprintf(buf, sizeof buf, "-thread%d", i);

		threads_.push_back(std::unique_ptr<Thread>(
			new Thread(std::bind(&ThreadPool::runInThread, this), name_ + buf)));
	}

	//启动线程
	for (const std::unique_ptr<Thread>& t : threads_) {
		t->start();
	}
}

void ThreadPool::stop()
{
	{
		MutexLockGuard lock(mutex_); //running_为共享读互斥写
		running_ = false;
		cond_.notifyAll();  //唤醒所有等待中的线程
	}

	//等待所有线程结束
	for (const std::unique_ptr<Thread>& t : threads_) {
		t->join();
	}

	//因为是unique_ptr管理，以下不做也不会有内存泄漏
	for (std::unique_ptr<Thread>& t : threads_) {
		t.reset();
	}
}

void ThreadPool::run(const Task& task)
{
	if (threads_.empty()) { //如果线程池中没有线程，则由调用线程直接执行
		task();
	}
	else {  //否则将其放入任务队列
		MutexLockGuard lock(mutex_);
		tasks_.push_back(task);
		cond_.notify();
	}
}

void ThreadPool::runInThread()	//线程入口函数
{
	while (running_)
	{
		Task task(take());
		if (task) {
			task();
		}
	}
}

ThreadPool::Task ThreadPool::take()	//取任务
{
	MutexLockGuard lock(mutex_);

	//使用while避免虚假唤醒
	while (tasks_.empty() && running_) {
		cond_.wait();
	}

	Task task;
	if (!tasks_.empty()) {
		task = tasks_.front();
		tasks_.pop_front();
	}

	return task;
}
// Author Fangping Liu
// 2019-08-28

#include "CurrentThread.h"
#include "Thread.h"
#include <assert.h>
#include <stdio.h>
#include <sys/syscall.h> //::syscall(SYS_gettid)
#include <unistd.h>

namespace lfp
{

namespace CurrentThread
{
	__thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	__thread const char* t_threadName = "defaultName";
} //namespace CurrentThread

namespace detail
{
	pid_t gettid()
	{
		return static_cast<pid_t>(::syscall(SYS_gettid));
		//函数::gettid可以得到线程的真实tid，但glibc没有实现该函数，
		//只能通过linux系统调用syscall(SYS_gettid)来获取
	}
}  //namespace detail

} //namespace lfp


using namespace lfp;

void CurrentThread::cachedId()
{
	if (t_cachedTid == 0) {
		t_cachedTid = detail::gettid();
		int ret = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
		assert(ret == 6); (void)ret;
	}
}


Thread::Thread(const ThreadFunc& func, const std::string& name)
  : started_(false),
  	pthreadId_(0),
	tid_(0),
	func_(func),
	name_(name),
	latch_(1)  //门栓值为1
{
	setDefaultName();
}

Thread::~Thread()
{
}

void Thread::start()
{
	assert(!started_);
	started_ = true;

	int ret = pthread_create(&pthreadId_, nullptr, &startThread, this);
	assert(ret == 0); (void)ret;

	latch_.wait();	//启动线程等待新线程创建完成才继续执行
}

int Thread::join()
{
	assert(started_);
	return pthread_join(pthreadId_, nullptr);
}

void Thread::setDefaultName()
{
	//some init
}

//线程入口函数，只有在入口函数之后的调用栈中调用的函数才是在新线程中执行
void* Thread::startThread(void* thread)
{
	Thread* t = static_cast<Thread*>(thread);
	t->runInThread();
	return nullptr;
}

void Thread::runInThread() //执行具体任务
{
	latch_.countDown();  //新线程已启动，通知启动线程继续执行

	tid_ = CurrentThread::tid();  //获取线程真实ID的同时缓存ID
	if (name_.empty()) {
		char buf[128];
		snprintf(buf, sizeof buf, "Thread%d", tid_);
		name_ = buf;
	}
	CurrentThread::t_threadName = name_.c_str();

	func_();  //执行任务
	CurrentThread::t_threadName = "finished";
}

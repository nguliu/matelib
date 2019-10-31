#include <matelib/base/CurrentThread.h>
#include <matelib/base/Thread.h>

#include <functional> //std::bind
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

using namespace lfp;
using namespace std;

void threadFunc1()
{
	printf("tid=%d\n", CurrentThread::tid());
	printf("--CurrentThread::t_threadName = %s\n", CurrentThread::name());
}

void threadFunc2(int x, Thread* thread)
{
	printf("tid=%d, thread.name=%s, x=%d\n",
			CurrentThread::tid(), thread->name().c_str(), x);
	printf("--CurrentThread::t_threadName = %s\n", CurrentThread::name());
}

class Foo
{
public:
	explicit Foo(double x) : x_(x) { }
	
	void memberFunc(const string& text, Thread* thread) {
		printf("tid=%d, thread.name=%s, Foo.x=%f, text=%s\n",
				CurrentThread::tid(), thread->name().c_str(), x_, text.c_str());
		printf("--CurrentThread::t_threadName = %s\n", CurrentThread::name());
	}
private:
	double x_;
};


int main()
{
	printf("pid=%d, tid=%d\n\n", ::getpid(), CurrentThread::tid());

	Thread t1(threadFunc1);
	t1.start();
	t1.join();

	Thread t2(std::bind(threadFunc2, 2, &t2), "thread2");
	t2.start();
	t2.join();

	Foo foo(87.5);
	Thread t3(std::bind(&Foo::memberFunc, &foo, "this is a text", &t3), "thread3");
	t3.start();
	t3.join();
}
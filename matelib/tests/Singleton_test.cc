#include <matelib/base/Singleton.h>
#include <matelib/base/CurrentThread.h>
#include <matelib/base/Thread.h>

#include <string>
#include <stdio.h>

using namespace lfp;
using namespace std;

class Test
{
public:
	Test() {
		printf("tid=%d, constructing %p\n", CurrentThread::tid(), this);
	}

	~Test() {
		printf("tid=%d, destructing %p %s\n", CurrentThread::tid(), this, name_.c_str());
	}

	const string& name() const { return name_; }
	void setName(const string& n) { name_ = n; }

private:
	string name_;
};

void threadFunc() {
    printf("tid=%d, %p name=%s\n",
            CurrentThread::tid(),
            &Singleton<Test>::instance(),
            Singleton<Test>::instance().name().c_str());

    Singleton<Test>::instance().setName("only one, changed");
}

int main()
{
    Singleton<Test>::instance().setName("only one");

    Thread t1(threadFunc);
    t1.start();
    t1.join();

    printf("tid=%d, %p name=%s\n",
            CurrentThread::tid(),
            &Singleton<Test>::instance(),
            Singleton<Test>::instance().name().c_str());
}

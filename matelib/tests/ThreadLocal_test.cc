#include <matelib/base/CurrentThread.h>
#include <matelib/base/noncopyable.h>
#include <matelib/base/ThreadLocal.h>
#include <matelib/base/Thread.h>
#include <stdio.h>


using namespace lfp;

class Test : noncopyable
{
    public:
        Test() {
            printf("tid = %d, constructing Test %p\n", CurrentThread::tid(), this);
        }

        ~Test() {
            printf("tid = %d, destructing  Test %p %s\n", CurrentThread::tid(), this, name_.c_str());
        }

        const std::string& name() const { return name_; }
        void setName(const std::string& n) { name_ = n; }

    private:
        std::string name_;
};


ThreadLocal<Test> testObj1;	//这里会生成两个键
ThreadLocal<Test> testObj2;


void print() {
    printf("\ntid = %d, obj1 value:%p name = %s\n",
            CurrentThread::tid(),
            &testObj1.value(), testObj1.value().name().c_str());

    printf("tid = %d, obj2 value:%p name = %s\n\n",
            CurrentThread::tid(),
            &testObj2.value(), testObj2.value().name().c_str());
}

void threadFunc() {
    testObj1.initValue();	 //子线程通过两个键值关联两个对象
    testObj2.initValue();

    print();
    testObj1.value().setName("child thread changed 1");
    testObj2.value().setName("child thread changed 2");
    print();
	
//	exit(0);	//如果是这样退出将没有child thread changed 1/2的析构
}

int main() {
    testObj1.initValue();	 //主线程通过键值关联两个Test对象
    testObj2.initValue();

    testObj1.value().setName("main thread one");
    print();

    Thread t1(threadFunc);
    t1.start();
    t1.join();

    testObj2.value().setName("main thread two");
    print();

//	exit(0);	//如果是这样退出将没有main thread的析构
    pthread_exit(0);
}

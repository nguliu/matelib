#include "../base/ThreadPool.h"
#include "../base/CountDownLatch.h"
#include "../base/CurrentThread.h"
#include <functional>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

using namespace lfp;

void print() {
    printf("I am %s, tid = %d, get a empty task\n", CurrentThread::name(), CurrentThread::tid());
}

void printString(const std::string& text) {
    printf("I am %s, tid = %d, text = %s\n",
            CurrentThread::name(), CurrentThread::tid(), text.c_str());
}

void countDown(ThreadPool* pool, CountDownLatch* latch) {
    printf("there are task countDown, I am %s, will sleep 2s.......\n", CurrentThread::name());
    sleep(2);
	latch->countDown();
}

int main()
{
    printf("main: pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());

    ThreadPool pool(5, "ThreadPool");
	pool.run(print);  //now pool.threads_ is empry
    pool.start();
    pool.run(print);  //now pool.threads_ is not empry

    for (int i = 0; i < 20; ++i) {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d come from thread %d", i, CurrentThread::tid());
        pool.run(std::bind(printString, std::string(buf)));
    }

    CountDownLatch latch(1);	//传入一个countDown任务
    pool.run(std::bind(&countDown, &pool, &latch));

    latch.wait();
    pool.stop();

	printf("over\n");
}

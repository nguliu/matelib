#include "../base/Logging.h"
#include "../base/CountDownLatch.h"
#include "../EventLoopThread.h"
#include "../Timer.h"
#include <functional>
#include <stdio.h>
#include <unistd.h>

using namespace lfp;
using namespace std;

int cnt = 0;
EventLoop* g_loop = nullptr;
CountDownLatch latch(1);


void printTid()
{
	SYNC_LOG << "printTid: pit=" << ::getpid() << ", tid=" << CurrentThread::tid();
}

void printMsg(const char* msg)
{
	SYNC_LOG << "printMsg: message=" << msg;
    if (++cnt == 20)
    {
        g_loop->quit();
		latch.countDown();
    }
}

void cancel(TimerId timer)
{
    g_loop->cancelTimer(timer);
	SYNC_LOG << "cancel";
}

int main()
{
	SET_ASYNCLOG_BASENAME("timer_test_log");
	EventLoopThread loopThread;

	g_loop = loopThread.start();
	printTid();

	g_loop->runAfter(1, std::bind(printMsg, "once1"));
	g_loop->runAfter(1.5, std::bind(printMsg, "once1.5"));
	g_loop->runAfter(2.5, std::bind(printMsg, "once2.5"));
	g_loop->runAfter(3.5, std::bind(printMsg, "once3.5"));
	TimerId t45 = g_loop->runAfter(4.5, std::bind(printMsg, "once4.5"));
	g_loop->runAfter(4.2, std::bind(cancel, t45));
	g_loop->runAfter(4.8, std::bind(cancel, t45));
	g_loop->runEvery(2, std::bind(printMsg, "every2"));
	TimerId t3 = g_loop->runEvery(3, std::bind(printMsg, "every3"));
	g_loop->runAfter(9.001, std::bind(cancel, t3));

	printf("main loop start\n");
	latch.wait();
	printf("main loop exits\n");
}

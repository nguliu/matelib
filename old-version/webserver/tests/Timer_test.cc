#include "../base/Logging.h"
#include "../EventLoop.h"
#include "../Timer.h"
#include <functional>
#include <stdio.h>
#include <unistd.h>

using namespace lfp;
using namespace std;

int cnt = 0;
EventLoop* g_loop = nullptr;


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
	EventLoop loop;
	g_loop = &loop;

	printTid();

	loop.runAfter(1, std::bind(printMsg, "once1"));
	loop.runAfter(1.5, std::bind(printMsg, "once1.5"));
	loop.runAfter(2.5, std::bind(printMsg, "once2.5"));
	loop.runAfter(3.5, std::bind(printMsg, "once3.5"));
	TimerId t45 = loop.runAfter(4.5, std::bind(printMsg, "once4.5"));
	loop.runAfter(4.2, std::bind(cancel, t45));
	loop.runAfter(4.8, std::bind(cancel, t45));
	loop.runEvery(2, std::bind(printMsg, "every2"));
	TimerId t3 = loop.runEvery(3, std::bind(printMsg, "every3"));
	loop.runAfter(9.001, std::bind(cancel, t3));

	printf("main loop start\n");
	loop.loop();
	printf("main loop exits\n");
}

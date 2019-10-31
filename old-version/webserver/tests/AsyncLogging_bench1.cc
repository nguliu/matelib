#include "../base/AsyncLogging.h"
#include "../base/CountDownLatch.h"
#include "../base/Logging.h"
#include "../base/ThreadPool.h"
#include "../base/Timestamp.h"
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace lfp;


off_t kRollSize = 100 * 1024 * 1024;  //日志文件滚动大小为100M
AsyncLogging* g_asyncLog = NULL;

CountDownLatch latch(5);


void asyncOutput(const char* msg, int len) {
    g_asyncLog->append(msg, len);
}

void bench() {
    const int time = 10 * 10000;
	int cnt = 0;

	for (int i = 0; i < 30; ++i)
	{
		Timestamp start = Timestamp::now();
		for (int k = 0; k < time; ++k) {
			++cnt;
			//注意这里是通过修改同步日志的输出函数使用了异步日志，并不是直接使用异步日志
			SYNC_LOG << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " << cnt;
		}
		Timestamp end = Timestamp::now();

		printf("%f us / 次\n", timeDifference(end, start) * 1000000 / time);
	}

	latch.countDown();
}

int main(int argc, char* argv[])
{
	{
		// set max virtual memory to 2GB.
		size_t kOneGB = 1000*1024*1024;
		rlimit rl = { 2 * kOneGB, 2 * kOneGB };
		setrlimit(RLIMIT_AS, &rl);
	}

	Logger::setOutput(asyncOutput);
    
	AsyncLogging asyncLog("asynclog_bench1", kRollSize);
    g_asyncLog = &asyncLog;
	asyncLog.start();

	ThreadPool tPool(5);  //5个生产者线程
	tPool.start();

	Timestamp start = Timestamp::now();
	tPool.run(bench);
	tPool.run(bench);
	tPool.run(bench);
	tPool.run(bench);
	tPool.run(bench);
    latch.wait();
	Timestamp end = Timestamp::now();

	tPool.stop();
    printf("Make over, all use %fs\n", timeDifference(end, start));
	
	sleep(1);  //等待后台线程写完
    asyncLog.stop();

    printf("All over!\n");
}
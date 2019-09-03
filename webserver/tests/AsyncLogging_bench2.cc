#include "../base/AsyncLogging.h"
#include "../base/CountDownLatch.h"
#include "../base/Logging.h"
#include "../base/ThreadPool.h"
#include "../base/Timestamp.h"
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace lfp;


off_t kRollSize = 300 * 1024 * 1024;  //日志文件滚动大小为300M
CountDownLatch latch(5);


void bench() {
    const int time = 10 * 10000;
	int cnt = 0;

	for (int i = 0; i < 30; ++i)
	{
		Timestamp start = Timestamp::now();
		for (int k = 0; k < time; ++k) {
			++cnt;
			//这里直接使用异步日志
			ASYNC_LOG << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " << cnt;
		}
		Timestamp end = Timestamp::now();

		//printf("%f us / 次\n", timeDifference(end, start) * 1000000 / time);
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

	SET_ASYNCLOG_BASENAME("asynclog_bench2");
	SET_ASYNCLOG_ROLLSIZE(kRollSize);  //设置日志文件滚动大小为300M

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
	ASYNCLOG_STOP;

    printf("All over!\n");
}
#include <matelib/base/AsyncLogging.h>
#include <matelib/base/CountDownLatch.h>
#include <matelib/base/Logging.h>
#include <matelib/base/ThreadPool.h>
#include <matelib/base/Timestamp.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace lfp;


off_t kRollSize = 500 * 1024 * 1024;  //日志文件滚动大小为500M
AsyncLogging* g_asyncLog = NULL;

CountDownLatch latch(5);


void asyncOutput(const char* msg, int len) {
    g_asyncLog->append(msg, len);
}

void bench() {
    const int time = 10 * 10000;
	int cnt = 0;

	Timestamp start = Timestamp::now();
	for (int i = 0; i < 30; ++i)
	{
		for (int k = 0; k < time; ++k) {
			STDLOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " << ++cnt;
		}	
	}
	Timestamp end = Timestamp::now();
	printf("%f s\n", timeDifference(end, start));

	latch.countDown();
}

int main(int argc, char* argv[])
{
	Logger::setOutput(asyncOutput);//注意这里是通过修改同步日志的输出函数使用了异步日志，并不是直接使用异步日志
    
	AsyncLogging asyncLog("asynclog_bench1", kRollSize);
    g_asyncLog = &asyncLog;

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
    printf("Make over, all use %fs\n", timeDifference(end, start));
	
	tPool.stop();
	
	sleep(1);  //等待后台线程写完
    asyncLog.stop();

    printf("All over!\n");
}
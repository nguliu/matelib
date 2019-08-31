#include "../base/Logging.h"
#include <string>
#include <unistd.h>

using namespace lfp;


off_t kRollSize = 100 * 1024 * 1024;  //日志文件滚动大小为100M

int main() {
	bool					bool_ = true;
	char					char_ = 'C';
	short					short_ = 10;
	unsigned short			ushort_ = 100;
	int						int_ = 1000;
	unsigned int			uint_ = 10000;
	long					long_ = 100000;
	unsigned long			ulong_ = 1000000;
	long long				llong_ = 10000000;
	unsigned long long		ullong_ = 100000000;
	float					float_ = 88.755;
	double					double_ = 9999999.777755555;
	unsigned long*			pulong_ = &ulong_;
	const char				pchar_[] = "this is char*";
	const unsigned char		puchar_[] = "this is unsigned char*";
	std::string				str_ = "this is std::string";

	SYNC_LOG << bool_;
	SYNC_LOG << char_;
	SYNC_LOG << short_;
	SYNC_LOG << ushort_;
	SYNC_LOG << int_;
	SYNC_LOG << uint_;
	SYNC_LOG << long_;
	SYNC_LOG << ulong_;
	SYNC_LOG << llong_;
	SYNC_LOG << ullong_;
	SYNC_LOG << float_;
	SYNC_LOG << double_;
	SYNC_LOG << pulong_;
	SYNC_LOG << pchar_;
	SYNC_LOG << puchar_;
	SYNC_LOG << str_;


	SET_ASYNCLOG_BASENAME("logging_test_async");
	SET_ASYNCLOG_ROLLSIZE(kRollSize);  //设置日志文件滚动大小为100M
	ASYNC_LOG << bool_;
	ASYNC_LOG << char_;
	ASYNC_LOG << short_;
	ASYNC_LOG << ushort_;
	ASYNC_LOG << int_;
	ASYNC_LOG << uint_;
	ASYNC_LOG << long_;
	ASYNC_LOG << ulong_;
	ASYNC_LOG << llong_;
	ASYNC_LOG << ullong_;
	ASYNC_LOG << float_;
	ASYNC_LOG << double_;
	ASYNC_LOG << pulong_;
	ASYNC_LOG << pchar_;
	ASYNC_LOG << puchar_;
	ASYNC_LOG << str_;
	
	const int times = 100 * 10000;
	std::string str(1000, 'X');
	for (int i = 0; i < times; ++i) {
		ASYNC_LOG << str << " Hello 0123456789 ~!@#$%^&*()_+-=,.<>/? abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " << i;
	}

	printf("Make over!\n");
	sleep(2);  //等待后台线程写完
	ASYNCLOG_STOP;

    printf("All over!\n");
}
#include <matelib/base/Logging.h>
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

	STDLOG_INFO << bool_;
	STDLOG_INFO << char_;
	STDLOG_INFO << short_;
	STDLOG_INFO << ushort_;
	STDLOG_INFO << int_;
	STDLOG_INFO << uint_;
	STDLOG_INFO << long_;
	STDLOG_INFO << ulong_;
	STDLOG_INFO << llong_;
	STDLOG_INFO << ullong_;
	STDLOG_INFO << float_;
	STDLOG_INFO << double_;
	STDLOG_INFO << pulong_;
	STDLOG_INFO << pchar_;
	STDLOG_INFO << puchar_;
	STDLOG_INFO << str_;


	SET_ASYNCLOG_BASENAME("logging_test_async");
	SET_ASYNCLOG_ROLLSIZE(kRollSize);  //设置日志文件滚动大小为100M
	LOG_INFO << bool_;
	LOG_INFO << char_;
	LOG_INFO << short_;
	LOG_INFO << ushort_;
	LOG_INFO << int_;
	LOG_INFO << uint_;
	LOG_INFO << long_;
	LOG_INFO << ulong_;
	LOG_INFO << llong_;
	LOG_INFO << ullong_;
	LOG_INFO << float_;
	LOG_INFO << double_;
	LOG_INFO << pulong_;
	LOG_INFO << pchar_;
	LOG_INFO << puchar_;
	LOG_INFO << str_;
	
	std::string str(1000, 'X');
	const int times = 100 * 10000;

//	Logger::setLogLevel(Logger::DEBUG);
//	Logger::setLogLevel(Logger::NONLOG);
	for (int i = 0; i < times; ++i) {
		LOG_INFO << "No." << i+1 << " Hello 0123456789 ~!@#$%^&*()_+-=,.<>/? abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		LOG_DEBUG << str;
	}
	printf("Make over!\n");
	
	sleep(1);  //等待后台线程写完
	ASYNCLOG_STOP();

    printf("All over!\n");
}
#include <matelib/base/Logging.h>
#include <matelib/Buffer.h>
#include <matelib/Channel.h>
#include <matelib/EventLoop.h>
#include <matelib/InetAddress.h>
#include <matelib/TcpClient.h>
#include <signal.h>
#include <unistd.h>


using namespace lfp;
using namespace std::placeholders;  //std::bind(_1)

class TestTcpClient
{
public:
	TestTcpClient(EventLoop* loop, const InetAddress& addr, int retryDelayMs)
	  : buf_(),
		loop_(loop),
		tcpClient_(new TcpClient(loop_, addr, retryDelayMs)),
		stdinChannel_(new Channel(loop_, STDIN_FILENO))
	{
		tcpClient_->setConnectionCallback(std::bind(&TestTcpClient::onConnection, this, _1));
		tcpClient_->setMessageCallback(std::bind(&TestTcpClient::onMessage, this, _1, _2));
		tcpClient_->setCleanupCallback(std::bind(&TestTcpClient::onCleanup, this, _1));

		stdinChannel_->setReadCallback(std::bind(&TestTcpClient::readStdin, this));
	}

	void connect() { tcpClient_->connect(); }

	void disconnect() { tcpClient_->shutdown(); }

private:
	void onConnection(const TcpClientShptr& client) {
		if (client->connectError()) {
			printf("connect to %s failed\n", client->serverAddress().toIpPort().c_str());
			loop_->quit();
		}
		else if (client->connected()) {
			STDLOG_INFO << client->name() << " is UP";
			stdinChannel_->enableReading();
		}
		else {
			STDLOG_INFO << client->name() << " is DOWN";
		}
	}

	void onMessage(const TcpClientShptr& conn, Buffer* buffer) {
		std::string str(buffer->retrieveAllAsString());
		
		printf("On [%s] receved: %s\n", tcpClient_->serverAddress().toIpPort().c_str(),
										str.c_str());
	}

	void onCleanup(const TcpClientShptr& conn) {
		printf("Clean up %s\n", tcpClient_->serverAddress().toIpPort().c_str());
		loop_->quit();
	}

	void readStdin() {
		if (buf_.readFd(STDIN_FILENO, nullptr) > 0)
		{
			const char* crlf1 = buf_.findStr("\r", 1);
			if (crlf1)
				buf_.erase(crlf1, 1);

			const char* crlf2 = buf_.findStr("\n", 1);
			if (crlf2)
				buf_.erase(crlf2, 1);
			
			tcpClient_->send(&buf_);
		}
		buf_.retrieveAll();
	}

	Buffer buf_;
	EventLoop* loop_;
	TcpClientShptr tcpClient_;
	std::unique_ptr<Channel> stdinChannel_;
};

TestTcpClient* g_client = nullptr;

void sig_int(int) {
	printf("\nCapture to 'Ctrl+C'\n\n");
	g_client->disconnect();
}

int main(int argc, char* argv[]) {
	printf("use 'Ctrl+C' to quit!\n");

	const char* ip = (argc > 1 ? argv[1] : "127.0.0.1");
	int port = (argc > 2 ? atoi(argv[1]) : 8888);
	InetAddress serverAddr(ip, port);

	EventLoop loop;
	TestTcpClient client(&loop, serverAddr, 20);
	g_client = &client;

	::signal(SIGINT, sig_int);

	client.connect();
	loop.loop();

	return 0;
}
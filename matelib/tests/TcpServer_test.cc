#include <matelib/base/CurrentThread.h>
#include <matelib/TcpServer.h>
#include <matelib/EventLoop.h>
#include <matelib/InetAddress.h>
#include <functional>
#include <stdio.h>
#include <unistd.h>

using namespace lfp;
using namespace std;
using namespace std::placeholders;	//std::bind(_1)

class TestServer
{
public:
	TestServer(EventLoop* loop, const InetAddress& listenAddr, int threadNum)
	  : loop_(loop),
		server_(loop, listenAddr, threadNum)
	{
		server_.setConnectionCallback(
				std::bind(&TestServer::onConnection, this, _1));
		server_.setMessageCallback(
				std::bind(&TestServer::onMessage, this, _1, _2));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionShptr& conn)
	{
//		printf("onConnection: connection %s is %s\n",
//				conn->name().c_str(),
//				(conn->connected() ? "UP" : "DOWN"));
	}

	void onMessage(const TcpConnectionShptr& conn,
			Buffer* buf)
	{
		string msg(buf->retrieveAllAsString());
//		printf("onMessage(): received %zd bytes from connection [%s]\n",
//				msg.size(),
//				conn->name().c_str());
		printf("receved: %s \n", msg.c_str());

		conn->send(msg);
	}

	EventLoop* loop_;
	TcpServer server_;
};


int main()
{
    printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

    InetAddress listenAddr(8888);
    EventLoop loop;

    TestServer testServer(&loop, listenAddr, 1);
    testServer.start();

    loop.loop();
}
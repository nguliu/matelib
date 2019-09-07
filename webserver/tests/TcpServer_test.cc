#include "../base/CurrentThread.h"
#include "../TcpServer.h"
#include "../EventLoop.h"
#include "../InetAddress.h"
#include <functional>
#include <stdio.h>
#include <unistd.h>

using namespace lfp;
using namespace std;
using namespace std::placeholders;

class TestServer
{
public:
	TestServer(EventLoop* loop, const InetAddress& listenAddr)
	  : loop_(loop),
		server_(loop, listenAddr, 0, "TestTcpServer")	//这里没有开sub io thread
	{
		server_.setConnectionCallback(
				std::bind(&TestServer::onConnection, this, _1));
		server_.setMessageCallback(
				std::bind(&TestServer::onMessage, this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionShptr& conn)
	{
		if (conn->isConnected())
		{
			printf("onConnection(): new connection [%s] from %s is UP\n",
					conn->name().c_str(),
					conn->peerAddress().toIpPort().c_str());
		}
		else
		{
			printf("onConnection(): connection [%s] is DOWN, conn.use_count=%ld\n",
					conn->name().c_str(), conn.use_count());
		}
	}

	void onMessage(const TcpConnectionShptr& conn,
			Buffer* buf,
			Timestamp receiveTime)
	{
		string msg(buf->retrieveAllAsString());
		printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
				msg.size(),
				conn->name().c_str(),
				receiveTime.toFormattedString().c_str());
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

    TestServer testServer(&loop, listenAddr);
    testServer.start();

    loop.loop();
}

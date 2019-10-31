// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include "base/Logging.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "TcpClient.h"
#include "Timer.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace lfp;
using namespace std::placeholders;  //std::bind(_1)

//默认连接失败的时延为0.5s，断开后不需要重连
TcpClient::TcpClient(EventLoop* loop, const InetAddress& addr, int retryDelayMs)
  : state_(kDisconnected),
  	connectTimes_(0),
  	sockfd_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
	loop_(loop),
	serverAddr_(addr),
	retryDelayMs_(retryDelayMs),
	conn_(new TcpConnection(loop_, sockfd_, std::string(), serverAddr_, serverAddr_)),	 //初始化时把地址都设为服务端地址
	connectionCallback_(),
	messageCallback_(),
	cleanupCallback_()
{
	if (sockfd_ < 0) {
		int savedError = errno;
		LOG_ERROR << "syscall socket error: " << strerror_tl(savedError);
		abort();
	}
	SET_ASYNCLOG_BASENAME("tcpclient");
}

int TcpClient::connect()
{
	loop_->assertInLoopThread();

	if (connectTimes_++ < maxRetryTimes)
	{
		int ret = ::connect(sockfd_, (sockaddr*)&serverAddr_.getSockAddrInet(), sizeof(struct sockaddr));
		int savedError = (ret == 0) ? 0 : errno;
		switch (savedError)
		{
			case 0:
			case EISCONN:		// 连接成功
				ret = initConnection();
				return ret;
			case EINPROGRESS:	// 非阻塞套接字，未连接成功返回码是EINPROGRESS表示正在连接
			case EINTR:			// 被中断
			case EAGAIN:
			case EADDRINUSE:
			case EADDRNOTAVAIL:
			case ECONNREFUSED:
			case ENETUNREACH:	// 采用二次退避重连
				printf("The #%d reconnect will take place in %dms\n", connectTimes_, retryDelayMs_);
				loop_->runAfter(retryDelayMs_/1000.0, std::bind(&TcpClient::connect, this));
				if (retryDelayMs_ > 0 && retryDelayMs_ < maxRetryDelayMs_) {
					retryDelayMs_ *= 2;
				}
				return 0;
			case EACCES:
			case EPERM:
			case EAFNOSUPPORT:
			case EALREADY:
			case EBADF:
			case EFAULT:
			case ENOTSOCK:
				STDLOG_ERROR << "syscall socket error: " << strerror_tl(savedError);
				break;
			default:
				STDLOG_ERROR << "Unexpected error in TcpClient::connect: " << strerror_tl(savedError);
				break;
		}
	}

	::close(sockfd_);	// 连接失败，关闭sockfd
	setState(kConnectError);
	if (connectionCallback_) {
		loop_->runInLoop(std::bind(connectionCallback_, shared_from_this()));
	}
	return -1;
}

int TcpClient::initConnection()
{
//	int err;
//	socklen_t len = sizeof err;
//	::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len);
//	if (err) {
//		STDLOG_ERROR << "connection " << serverAddr_.toIpPort() << "error, SO_ERROR=" << err << ": " << strerror_tl(err);
//		printf("The #%d reconnect will take place in %dms\n", connectTimes_, retryDelayMs_);
//		loop_->runAfter(retryDelayMs_/1000.0, std::bind(&TcpClient::connect, this));
//		return 0;
//	}
	
	connectTimes_ = 0;
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	socklen_t addrlen = sizeof addr;
	::getsockname(sockfd_, (struct sockaddr*)&addr, &addrlen);
	InetAddress localAddr(addr);

	conn_->name() = ("c:" + localAddr.toPort() + "." + std::to_string(sockfd_) \
						  + "-" + serverAddr_.toIpPort());
	conn_->localAddress() = localAddr;

	conn_->setConnectionCallback(std::bind(&TcpClient::onConnection, this, _1));
	conn_->setMessageCallback(std::bind(&TcpClient::onMessage, this, _1, _2));
	conn_->setCleanupCallback(std::bind(&TcpClient::onCleanup, this, _1));
	conn_->setNoDelay();

	//要将connectionEstablished放到conn所属的线程中去执行
	loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn_));
	return sockfd_;
}

void TcpClient::onConnection(const TcpConnectionShptr& conn)
{
	setState(conn->connected() ? kConnected : kDisconnected);

	if (connectionCallback_) {
		connectionCallback_(shared_from_this());
	}
	else
	{
		if (connectError()) {
			STDLOG_ERROR << "connect to " << serverAddr_.toIpPort() << " error";
		}
		else {
			STDLOG_INFO << name() << " is " << (connected() ? "UP" : "DOWN");
		}
	}
}

void TcpClient::onMessage(const TcpConnectionShptr& conn, Buffer* buffer)
{
	if (messageCallback_) {
		messageCallback_(shared_from_this(), buffer);
	}
	else {
		LOG_WARN << conn->name() << " receved " << buffer->readableBytes() << " byte data";
		buffer->retrieveAll();
	}
}

void TcpClient::onCleanup(const TcpConnectionShptr& conn)
{
	if (cleanupCallback_) {
		cleanupCallback_(shared_from_this());
	}

	loop_->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn_));
}
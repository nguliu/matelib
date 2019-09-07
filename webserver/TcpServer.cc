// Author Fangping Liu
// 2019-09-04

#include "base/Logging.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace lfp::detail
{
	void defaultConnectionCallback(const TcpConnectionShptr& conn)
	{
		printf("On defaultConnectionCallback [%s]-[%s] is %s\n",
				conn->localAddress().toIpPort().c_str(),
				conn->peerAddress().toIpPort().c_str(),
				(conn->isConnected() ? "UP" : "DOWN"));
	}

	void defaultMessageCallback(const TcpConnectionShptr& conn,
								Buffer* buf,
								Timestamp time)
	{
		
		printf("On defaultMessageCallback recved %d bytes data from %s at %s\n",
				static_cast<int>(buf->readableBytes()),
				conn->peerAddress().toIpPort().c_str(),
				time.toFormattedString().c_str());
		buf->retrieveAll();
	}

	void setNONBLOCKandCLOEXEC(int sockfd) {
		//set nonblocking
		int flag = ::fcntl(sockfd, F_GETFL, 0);
		flag |= O_NONBLOCK;
		::fcntl(sockfd, F_SETFL, flag);
		//set close on exec
		flag = ::fcntl(sockfd, F_GETFD, 0);
		flag |= O_CLOEXEC;
		::fcntl(sockfd, F_SETFD, flag);
	}
}  //end of namespace lfp::detail

using namespace lfp;


TcpServer::TcpServer(EventLoop* mainLoop,
					 const InetAddress& listenAddr,
					 int threadNum,
					 const std::string& serverName)
  : started_(false),
  	mainLoop_(mainLoop),
	idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
	listenSock_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)), //2.6.27以上的内核才支持SOCK_NONBLOCK与SOCK_CLOEXEC
	acceptChannel_(mainLoop_, listenSock_),													 //若不支持请使用::socket + setNONBLOCKandCLOEXEC
	hostIpPort_(listenAddr.toIpPort()),
	serverName_(serverName),
	threadPool_(new EventLoopThreadPool(mainLoop_, threadNum)),
	nextConnId_(0),
	threadInitCallback_(),
	connectionCallback_(&detail::defaultConnectionCallback),
	messageCallback_(&detail::defaultMessageCallback)
{
	assert(listenSock_ > 0);

	//设定地址复用，主要作用是防止重启bind时返回Address already in use
	//详见：https://blog.csdn.net/songchuwang1868/article/details/83304578
	int on = 1;
	::setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	//bind
	int ret = ::bind(listenSock_,
					 (const struct sockaddr*)&listenAddr.getSockAddrInet(),
					 static_cast<socklen_t>(sizeof listenAddr));
	assert(ret == 0);

	//设置连接到来时的回调
	acceptChannel_.setReadCallback(std::bind(&TcpServer::handleConnection, this));
}

TcpServer::~TcpServer()
{
	for (auto it = connectionMap_.begin();
			it != connectionMap_.end(); ++it)
	{
		TcpConnectionShptr conn = it->second;
		it->second.reset();  //释放TcpConnection对象，引用计数减一
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
		conn.reset(); //释放conn对象
	}
}

void TcpServer::start()
{
	mainLoop_->assertInLoopThread();

	if (!started_) {
		SYNC_LOG << "TcpServer::start() [" << serverName_ << ":" << hostIpPort_ << "] start looping";

		started_ = true;
		threadPool_->start(threadInitCallback_);
		
		int ret = ::listen(listenSock_, SOMAXCONN);
		assert(ret == 0);

		acceptChannel_.enableReading();
	}
}

void TcpServer::handleConnection()	//用于接受新连接
{
	mainLoop_->assertInLoopThread();

	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	socklen_t addrlen = sizeof addr;
	
	//如果内核版本过低不支持accept4请使用accept + setNONBLOCKandCLOEXEC
	int connfd = ::accept4(listenSock_, (struct sockaddr*)&addr,
							&addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd < 0) {
		if (errno == EMFILE)	//优雅关闭连接
		{
			::close(idleFd_);
			idleFd_ = ::accept(listenSock_, NULL, NULL);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
		else {
			ASYNC_LOG << "TcpServer::handleConnection(): accept4 error";
		}
		return;
	}

	InetAddress peerAddr(addr);
	::getsockname(connfd, (struct sockaddr*)&addr, &addrlen);
	InetAddress localAddr(addr);

	char buf[32];
	snprintf(buf, sizeof buf, " #%d:%s", nextConnId_, peerAddr.toIpPort().c_str());
	++nextConnId_;
	std::string connName = serverName_ + buf;
	ASYNC_LOG << "TcpServer::handleConnection() [" << serverName_ << ":" << hostIpPort_ << "] NewConnection [" << connName << "]";

	EventLoop* ioLoop = threadPool_->getNextLoop();
	TcpConnectionShptr conn(new TcpConnection(this, ioLoop, connfd, connName, localAddr, peerAddr));
	//这里传入this是为了断开连接是TcpConnection向TcpServer中注册回调removeConnection，已删除列表中的对象
	
	connectionMap_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionShptr& conn)
{
	mainLoop_->assertInLoopThread();
	ASYNC_LOG << "TcpServer::removeConnection() [" << conn->name() << "]";

	EventLoop* ioLoop = conn->getLoop();
	
	//将conn从map中删除，conn对象的应用计数减一
	connectionMap_.erase(conn->name());
	//最后将conn绑定到一个对象，延迟conn的销毁时机
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
}
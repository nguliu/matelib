// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

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
		printf("New connection %s is %s\n",
				conn->name().c_str(),
				(conn->connected() ? "UP" : "DOWN"));
	}

	void defaultMessageCallback(const TcpConnectionShptr& conn,
								Buffer* buf)
	{
		
		printf("Recved %d bytes data from %s\n",
				static_cast<int>(buf->readableBytes()),
				conn->name().c_str());
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
}


using namespace lfp;
using namespace std::placeholders;  //std::bind(_1)

TcpServer::TcpServer(EventLoop* baseLoop,
					 const InetAddress& listenAddr,
					 int threadNum)
  : started_(false),
  	baseLoop_(baseLoop),
	idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
	listenSock_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)), //2.6.27以上的内核才支持SOCK_NONBLOCK与SOCK_CLOEXEC
	acceptChannel_(baseLoop_, listenSock_),													 //若不支持请使用::socket + setNONBLOCKandCLOEXEC
	hostIpPort_(listenAddr.toIpPort()),
	ioThreadPool_(new EventLoopThreadPool(baseLoop_, threadNum)),
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

	int ret = ::bind(listenSock_,
					 (const struct sockaddr*)&listenAddr.getSockAddrInet(),
					 static_cast<socklen_t>(sizeof(sockaddr)));
	assert(ret == 0);

	//设置连接到来时的回调
	acceptChannel_.setReadCallback(std::bind(&TcpServer::handleConnection, this));

	SET_ASYNCLOG_BASENAME("TcpServer");
	LOG_INFO << "TcpServer: idlefd = " << idleFd_;
	LOG_INFO << "TcpServer: listenfd = " << listenSock_;
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
	baseLoop_->assertInLoopThread();

	if (!started_) {
		printf("TcpServer start, listen at %s.%d\n", hostIpPort_.c_str(), listenSock_);

		started_ = true;
		ioThreadPool_->start(threadInitCallback_);
		
		int ret = ::listen(listenSock_, SOMAXCONN);
		assert(ret == 0);

		acceptChannel_.enableReading();
	}
}

//acceptChannel可读回调，用于接受新连接
void TcpServer::handleConnection()
{
	baseLoop_->assertInLoopThread();

	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	socklen_t addrlen = sizeof addr;
	
	//如果内核版本过低不支持accept4请使用accept + setNONBLOCKandCLOEXEC
	int connfd = ::accept4(listenSock_, (struct sockaddr*)&addr,
						   &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd < 0)
	{
		if (errno == EMFILE)	//优雅关闭连接
		{
			::close(idleFd_);
			idleFd_ = ::accept(listenSock_, NULL, NULL);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
		else {
			int savedErrno = errno;
			LOG_ERROR << "syscall accept4 error: " << strerror_tl(savedErrno);
		}
		return;
	}

	InetAddress peerAddr(addr);

	::getsockname(connfd, (struct sockaddr*)&addr, &addrlen);
	InetAddress localAddr(addr);

	char buf[32];
	snprintf(buf, sizeof buf, "#%d:%s.%d-%s", nextConnId_++, localAddr.toPort().c_str(), connfd, peerAddr.toIpPort().c_str());
	std::string connName(buf);
	LOG_DEBUG << "TcpServer::handleConnection " << buf;

	EventLoop* ioLoop = ioThreadPool_->getNextLoop();
	TcpConnectionShptr conn(new TcpConnection(ioLoop, connfd, connName, localAddr, peerAddr));
	
	connectionMap_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCleanupCallback(std::bind(&TcpServer::removeConnection, this, _1));
	conn->setNoDelay();

	//要将connectionEstablished放到conn所属的线程中去执行
	ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

//将实际的移除操作放到TcpServer所属的loop中去执行
void TcpServer::removeConnection(const TcpConnectionShptr& conn)
{
	baseLoop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

//在mainLoop_中执行
void TcpServer::removeConnectionInLoop(const TcpConnectionShptr& conn)
{
	baseLoop_->assertInLoopThread();
	LOG_DEBUG << "TcpServer::removeConnection " << conn->name();
	
	//将conn从map中删除，conn对象的应用计数减一
	connectionMap_.erase(conn->name());

	//最后将conn绑定到一个对象，延迟conn的销毁时机
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
}
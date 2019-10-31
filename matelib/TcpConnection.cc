// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include "base/Logging.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include <functional>
#include <netinet/tcp.h> //TCP_NODELAY
#include <sys/socket.h>
#include <unistd.h>

using namespace lfp;

TcpConnection::TcpConnection(EventLoop* loop,
							  int sockfd,
							  const std::string& name,
							  const InetAddress& localAddr,
							  const InetAddress& peerAddr)
  : loop_(loop),
	sockfd_(sockfd),
	state_(kConnecting),
	name_(name),
	channel_(new Channel(loop, sockfd_)),
	localAddr_(localAddr),
	peerAddr_(peerAddr)
{
	//设置channel各事件回调函数
	channel_->setReadCallback(
			std::bind(&TcpConnection::handleRead, this));
	channel_->setWriteCallback(
			std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(
			std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(
			std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
	::close(sockfd_);
}

void TcpConnection::send(const void* data, size_t len)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread()) {
			sendInThread(data, len);
		}
		else {
			//如果是跨线程调用，这里会有内存拷贝对效率有损失
			std::string message(static_cast<const char*>(data), len);
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
										this,
										message));
		}
	}
}

void TcpConnection::send(const StringPiece& message)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread()) {
			sendInThread(message.data(), message.size());
		}
		else {
			//如果是跨线程调用，这里会有内存拷贝对效率有损失
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
										this,
										message.asString()));
		}
	}
}

void TcpConnection::send(Buffer* buffer)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread()) {
			sendInThread(buffer->peek(), buffer->readableBytes());
		}
		else {
			//如果是跨线程调用，这里会有内存拷贝对效率有损失
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
										this,
										buffer->retrieveAllAsString()));
		}
	}
}


void TcpConnection::sendInLoop(const StringPiece& message)
{
	sendInThread(message.data(), message.size());
}

void TcpConnection::sendInThread(const void* data, size_t len)
{
	loop_->assertInLoopThread();

	if (state_ == kDisconnected) {
		LOG_WARN << "disconnected, give up writing";
		return;
	}

	bool error = false;
	int nwrote = 0;
	size_t remaining = len;
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nwrote = ::write(sockfd_, data, len);

		if (nwrote >= 0) {
			remaining = len - nwrote;
		}		
		else {
			nwrote = 0;
			if (errno != EWOULDBLOCK) {
				LOG_ERROR << "TcpConnection::sendInThread";
				if (errno == EPIPE)
					error = true;
			}
		}
	}

	//如果内核发生缓冲区满，将数据追加到应用层发送缓冲区
	if (!error && remaining > 0) {
		LOG_WARN << "kernel send buffer is full";

		outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
		if (!channel_->isWriting()) {
			channel_->enableWriting(); //关注通道可写事件
		}
	}
}


//只需调用一次
void TcpConnection::shutdown()
{
	if (state_ == kConnected) {
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();

	if (!channel_->isWriting()) { //如果发送缓冲区还有数据则不能立即关闭
		::shutdown(sockfd_, SHUT_WR);
	}
}

//在连接建立完成时被调用
void TcpConnection::connectionEstablished()
{
	loop_->assertInLoopThread();

	setState(kConnected);
	channel_->enableReading();
	connectionCallback_(shared_from_this());	//连接建立完成，调用用户连接回调函数
}

//在连接断开时被调用
void TcpConnection::connectionDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected) {
		setState(kDisconnected);
		channel_->disableAll();
		connectionCallback_(shared_from_this());	//连接断开，调用用户连接回调函数
	}
	channel_->remove();
}

//消息到来时被channel调用
void TcpConnection::handleRead()
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(sockfd_, &savedErrno);

	if (n > 0) {
		messageCallback_(shared_from_this(), &inputBuffer_);
	}
	else if(n == 0) {
		handleClose();
	}
	else {
		LOG_ERROR << "TcpConnection::handleRead error";
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	if (channel_->isWriting())
	{
		ssize_t n = ::write(sockfd_,
							outputBuffer_.peek(),
							outputBuffer_.readableBytes());
		if (n > 0)
		{
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0)		//发送缓冲区清空
			{
				LOG_DEBUG << "application layer send buffer is empty";

				channel_->disableWriting();		//缓冲区清空后要取消关注当前通道的可写事件
				if (state_ == kDisconnecting) {	//如果用户想关闭，则在数据发送完成后关闭
					shutdownInLoop();
				}
			}
		}
		else
		{
			int savedErrno = errno;
			LOG_ERROR << "syscall write error: " << strerror_tl(savedErrno);
		}
	}
	else {
		LOG_WARN << "Connection " << name_ << " is down, no more writing";
	}
}

void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();

	assert(state_ == kConnected || state_ == kDisconnecting);

	setState(kDisconnected);
	channel_->disableAll();

	connectionCallback_(shared_from_this());
	//通知上层清理当前连接相关资源
	cleanupCallback_(shared_from_this());	//TcpServer::removeConnection.....
}

void TcpConnection::handleError()
{
	int err;
	socklen_t len = sizeof err;
	::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len);
	LOG_ERROR << "connection " << name_ << ", SO_ERROR=" << err << ": " << strerror_tl(err);
}

void TcpConnection::setKeepalive()
{
	int on = 1;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);
	//TCP keepalive是指定期探测连接是否存在，如果应用层有心跳的话，这个选项不是必需要设置的
}

void TcpConnection::setNoDelay()
{
	int on = 1;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on);
	//禁用Nagle算法，可以避免连续发包出现网络延迟（Nagle算法可以一定程度上避免网络拥塞）
}
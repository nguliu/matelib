// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_TCPCONNECTION_H
#define MATELIB_TCPCONNECTION_H

#include "base/noncopyable.h"
#include "Buffer.h"
#include "callbacks.h"
#include "Channel.h"
#include "InetAddress.h"
#include "StringPiece.h"
#include <memory>
#include <string>

//TcpConnection是一个TCP连接的抽象
namespace lfp
{
	class Channel;
	class EventLoop;

	class TcpConnection : noncopyable,
						  public std::enable_shared_from_this<TcpConnection>
	{
	public:
		TcpConnection(EventLoop* loop,
					  int sockfd,
					  const std::string& name,
					  const InetAddress& localAddr,
					  const InetAddress& peerAddr);
		~TcpConnection();

		EventLoop* getLoop() const { return loop_; }
		std::string& name() { return name_; }
		const std::string& name() const { return name_; }
		InetAddress& localAddress() { return localAddr_; }
		const InetAddress& localAddress() const { return localAddr_; }
		InetAddress& peerAddress() { return peerAddr_; }
		const InetAddress& peerAddress() const { return peerAddr_; }
		bool connected() const { return state_ == kConnected; }

		void send(const void* data, size_t len);
		void send(const StringPiece& message);
		void send(Buffer* buffer);
		void shutdown();
		
		void setConnectionCallback(const ConnectionCallback& cb)
		{ connectionCallback_ = cb; }
		void setMessageCallback(const MessageCallback& cb)
		{ messageCallback_ = cb; }
		void setCleanupCallback(const CleanupCallback& cb)
		{ cleanupCallback_ = cb; }

		Buffer* inputBuffer()
		{ return &inputBuffer_; }

		void connectionEstablished();
		void connectionDestroyed();

		void setKeepalive();
		void setNoDelay();

	private:
		enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };
		//以下是channel回调函数
		void handleRead();
		void handleWrite();
		void handleClose();
		void handleError();

		void sendInLoop(const StringPiece& message);
		void sendInThread(const void* data, size_t len);
		void shutdownInLoop();
		void setState(State s) { state_  = s; }

		EventLoop* loop_;	//当前connetion所属的EventLoop对象
		int sockfd_;
		State state_;
		std::string name_;
		std::unique_ptr<Channel> channel_;
		InetAddress localAddr_;
		InetAddress peerAddr_;
		ConnectionCallback connectionCallback_;  //连接/断开回调函数，主要作用为输出消息提示、通知应用层连接建立/连接断开等
		MessageCallback messageCallback_;		 //消息到来回调函数，主要作用为通知应用层处理消息
		CleanupCallback cleanupCallback_;		 //资源清理回调函数，主要作用为通知应用层删除相应数据结构等

		Buffer inputBuffer_;	//应用层输入缓冲区
		Buffer outputBuffer_;	//应用层输出缓冲区
	};

} // namespace lfp

#endif // !MATELIB_TCPCONNECTION_H
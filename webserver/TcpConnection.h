// Author Fangping Liu
// 2019-09-04

#ifndef WEBSERVER_TCPCONNECTION_H
#define WEBSERVER_TCPCONNECTION_H

#include "base/MutexLock.h"
#include "base/noncopyable.h"
#include "Buffer.h"
#include "callbacks.h"
#include "Channel.h"
#include "InetAddress.h"
#include "StringPiece.h"
#include <memory>
#include <string>

namespace lfp
{
	class Channel;
	class EventLoop;
	class TcpServer;

	class TcpConnection : noncopyable,
						  public std::enable_shared_from_this<TcpConnection>
	{
	public:
		TcpConnection(TcpServer* server,
					  EventLoop* loop,
					  int sockfd,
					  const std::string& name,
					  const InetAddress& localAddr,
					  const InetAddress& peerAddr);
		~TcpConnection();

		EventLoop* getLoop() const { return loop_; }
		bool isConnected() const { return connected_; }
		const std::string name() const { return name_; }
		const InetAddress& localAddress() const { return localAddr_; }
		const InetAddress& peerAddress() const { return peerAddr_; }

		void send(const void* data, size_t len);
		void send(const StringPiece& message);
		void send(Buffer* buffer);
		void shutdown();
		
		void setConnectionCallback(const ConnectionCallback& cb)
		{ connectionCallback_ = cb; }
		void setMessageCallback(const MessageCallback& cb)
		{ messageCallback_ = cb; }

		Buffer* inputBuffer()
		{ return &inputBuffer_; }

		void connectionEstablished();
		void connectionDestroyed();

	private:
		void handleRead();
		void handleWrite();
		void handleClose();
		void handleError();

		void sendInLoop(const StringPiece& message);
		void sendInThread(const void* data, size_t len);
		void shutdownInLoop();

		bool connected_;	//是否已连接
		bool needDisconn_;	//是否需要断开
		TcpServer* server_;	//当前连接所属的TcpServer对象
		EventLoop* loop_;	//当前连接所属的EventLoop对象
		MutexLock mutex_;
		int sockfd_;
		Channel channel_;
		const std::string name_;
		InetAddress localAddr_;
		InetAddress peerAddr_;
		ConnectionCallback connectionCallback_;  //外部用户回调函数
		MessageCallback messageCallback_;		 //外部用户回调函数
		Buffer inputBuffer_;	//应用层输入缓冲区
		Buffer outputBuffer_;	//应用层输出缓冲区
	};

} //end of namespace lfp

#endif //end of WEBSERVER_TCPSERVER_H

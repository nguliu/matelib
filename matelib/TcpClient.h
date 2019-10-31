// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_TCPCLIENT_H
#define MATELIB_TCPCLIENT_H

#include "base/noncopyable.h"
#include "callbacks.h"
#include "InetAddress.h"
#include "StringPiece.h"
#include "TcpConnection.h"
#include <functional>
#include <memory>

//带自动重连功能的TcpClient实现

namespace lfp
{
	class Buffer;
	class EventLoop;
	class TcpClient;
	typedef std::shared_ptr<TcpClient> TcpClientShptr; 

	class TcpClient : noncopyable,
					  public std::enable_shared_from_this<TcpClient>
	{
	public:
		typedef std::function<void (const TcpClientShptr&)> ConnectionCallback;
		typedef std::function<void (const TcpClientShptr&)> CleanupCallback;
		typedef std::function<void (const TcpClientShptr&, Buffer*)> MessageCallback;
	public:
		//默认连接失败的时延为0.5s，断开后不需要重连
		TcpClient(EventLoop* loop, const InetAddress& addr,
				  int retryDelayMs = 500);

		int connect();	//连接服务器，连接成功返回套接字，重连返回0，失败返回-1

		void setConnectionCallback(const TcpClient::ConnectionCallback& cb)
		{ connectionCallback_ = cb; }
		void setMessageCallback(const TcpClient::MessageCallback& cb)
		{ messageCallback_ = cb; }
		void setCleanupCallback(const TcpClient::CleanupCallback& cb)
		{ cleanupCallback_ = cb; }

		void send(const void* data, size_t len) { conn_->send(data, len); }
		void send(const StringPiece& message) { conn_->send(message); }
		void send(Buffer* buffer) { conn_->send(buffer); }
		void shutdown() { setState(kDisconnected);conn_->shutdown(); }

		int sockfd() const { return sockfd_; }
		const std::string& name() const { return conn_->name(); }
		const InetAddress& serverAddress() const { return serverAddr_; }
		bool connected() const { return state_ == kConnected; }
		bool connectError() const { return state_ == kConnectError; }

	private:
		enum State { kDisconnected, kConnected, kConnectError };	//客户端状态
		void setState(State s) { state_ = s; }

		//注册到conn_中的回调
		void onConnection(const TcpConnectionShptr& conn);
		void onMessage(const TcpConnectionShptr& conn, Buffer* buffer);
		void onCleanup(const TcpConnectionShptr& conn);
		//连接建立成功时执行
		int initConnection();

		State state_;			//连接状态
		int connectTimes_;		//尝试重连的次数
		int sockfd_;			//当前连接的socket
		EventLoop* loop_;		//所属的loop对象
		InetAddress serverAddr_;
		int retryDelayMs_;			//连接失败时的重连时延
		TcpConnectionShptr conn_;	//当前的连接对象

		TcpClient::ConnectionCallback connectionCallback_;   //连接/断开回调函数，主要作用为输出消息提示、通知应用层连接建立/连接断开等
		TcpClient::MessageCallback messageCallback_;		 //消息到来回调函数，主要作用为通知应用层处理消息
		TcpClient::CleanupCallback cleanupCallback_;		 //资源清理回调函数，主要作用为通知应用层删除相应数据结构等

		const static int maxRetryDelayMs_ = 30 * 1000;	//最大重连时延为30s
		const static int maxRetryTimes = 10;	//最大重连次数为10次
	};

} // namespace lfp

#endif // !MATELIB_TCPCLIENT_H
// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_TCPSERVER_H
#define MATELIB_TCPSERVER_H


#include "base/noncopyable.h"
#include "callbacks.h"
#include "Channel.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <memory>
#include <map>
#include <string>

namespace lfp
{
	class EventLoop;
	class EventLoopThreadPool;

	class TcpServer : noncopyable
	{
	public:
		TcpServer(EventLoop* baseLoop,
				  const InetAddress& listenAddr,
				  int threadNum);
		~TcpServer();

		void start();

		EventLoop* getMainLoop() const { return baseLoop_; }
		const std::string& hostIpPort() const { return hostIpPort_; }

		void setThreadInitCallback(const ThreadInitCallback& cb)
		{ threadInitCallback_ = cb; }
		void setConnectionCallback(const ConnectionCallback& cb)
		{connectionCallback_ = cb; }
		void setMessageCallback(const MessageCallback& cb)
		{ messageCallback_ = cb; }

	private:
		void handleConnection();	//用于接受新连接
		void removeConnection(const TcpConnectionShptr& conn); //连接断开时，删除列表中的TcpConnection对象
		void removeConnectionInLoop(const TcpConnectionShptr& conn);

		bool started_;
		EventLoop* baseLoop_;	//main loop，用于接受连接请求
		int idleFd_;			//用于优雅关闭连接保留的句柄
		int listenSock_;		//服务端监听的套接字
		Channel acceptChannel_; //监听套接字所属channel
		const std::string hostIpPort_;
		std::unique_ptr<EventLoopThreadPool> ioThreadPool_;
		unsigned int nextConnId_;		//下一个连接ID
		std::map<std::string, TcpConnectionShptr> connectionMap_;

		//外部用户回调函数
		ThreadInitCallback threadInitCallback_;
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
	};

} // namespace lfp

#endif // !MATELIB_TCPSERVER_H
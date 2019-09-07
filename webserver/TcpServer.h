// Author Fangping Liu
// 2019-09-04

#ifndef WEBSERVER_TCPSERVER_H
#define WEBSERVER_TCPSERVER_H


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
		TcpServer(EventLoop* mainLoop,
				  const InetAddress& listenAddr,
				  int threadNum = 0,	//默认只使用一个main IO thread
				  const std::string& serverName = "TcpServer");
		~TcpServer();

		void start();

		EventLoop* getMainLoop() const { return mainLoop_; }
		const std::string& name() const { return serverName_; }
		const std::string hostIpPort() const { return hostIpPort_; }

		void setThreadInitCallback(const ThreadInitCallback& cb)
		{ threadInitCallback_ = cb; }
		void setConnectionCallback(const ConnectionCallback& cb)
		{connectionCallback_ = cb; }
		void setMessageCallback(const MessageCallback& cb)
		{ messageCallback_ = cb; }

		void removeConnection(const TcpConnectionShptr& conn); //连接断开时，删除列表中的TcpConnection对象

	private:
		void handleConnection();	//用于接受新连接

		bool started_;
		EventLoop* mainLoop_;	//main loop，用于接受连接请求
		int idleFd_;			//用于优雅关闭连接保留的句柄
		int listenSock_;		//服务端监听的套接字
		Channel acceptChannel_; //监听套接字所属channel
		const std::string hostIpPort_;
		const std::string serverName_;
		std::unique_ptr<EventLoopThreadPool> threadPool_;
		unsigned int nextConnId_;		//下一个连接ID
		std::map<std::string, TcpConnectionShptr> connectionMap_;

		ThreadInitCallback threadInitCallback_;
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_TCPSERVER_H
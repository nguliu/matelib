// Author Fangping Liu
// 2019-09-07

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H


#include "../base/noncopyable.h"
#include "../TcpServer.h"
#include "HttpContext.h"
#include <functional>
#include <string>

namespace lfp
{
	class HttpRequest;
	class HttpResponse;

	class HttpServer : noncopyable
	{
	public:
		typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

		HttpServer(EventLoop* mainLoop,
					const InetAddress& listenAddr,
					int threadNum = 0,	//默认不适用sub IO thread
					const std::string& serverName = "HttpServer");
		~HttpServer();

		void setHttpCallback(const HttpCallback& cb) {
			httpCallback_ = cb;
		}

		void start();

	private:
		void onConnection(const TcpConnectionShptr& conn);	//TcpServer连接到来回调
		void onMessage(const TcpConnectionShptr& conn,		//TcpServer消息到来回调
						Buffer* buf,
						Timestamp time);

		TcpServer server_;
		HttpContext httpContext_;
		HttpCallback httpCallback_;		//http请求到来用户回调函数
	};

} //end of namespace lfp

#endif //end of WEBSERVER_HTTPSERVER_H
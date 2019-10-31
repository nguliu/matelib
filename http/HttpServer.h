// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_HTTPSERVER_H
#define MATELIB_HTTPSERVER_H

#include <matelib/base/noncopyable.h>
#include <matelib/base/ThreadLocal.h>
#include <matelib/TcpServer.h>
#include "HttpContext.h"
#include <functional>
#include <string>
#include <memory>

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
					int threadNum,
					const std::string& serverName = "HttpServer");
		~HttpServer();

		void setHttpCallback(const HttpCallback& cb) {
			httpCallback_ = cb;
		}

		void start();

	private:
		void onConnection(const TcpConnectionShptr& conn);	//TcpServer连接到来回调
		void onMessage(const TcpConnectionShptr& conn,		//TcpServer消息到来回调
					   Buffer* buf);

		std::unique_ptr<TcpServer> server_;
		ThreadLocal<HttpContext> threadLocalContext_;	//用于解析HTTP请求
		HttpCallback httpCallback_;		//http请求到来用户回调函数
	};

} // namespace lfp

#endif // !MATELIB_HTTPSERVER_H
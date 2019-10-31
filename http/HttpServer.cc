// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include <matelib/base/Logging.h>
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include <matelib/Buffer.h>
#include <stdio.h>

namespace lfp::detail
{
	void defaultHttpCallback(const HttpRequest&, HttpResponse* response)
	{
		response->setStatusCode(HttpResponse::k404NotFound);
		response->setStatusMessage("Not Found");
		response->setCloseConnection(true);
	}

}

using namespace lfp;
using namespace std::placeholders; //std::bind(_1)


HttpServer::HttpServer(EventLoop* mainLoop,
			const InetAddress& listenAddr,
			int threadNum,
			const std::string& serverName)
  : server_(new TcpServer(mainLoop, listenAddr, threadNum)),
	threadLocalContext_(),
  	httpCallback_(detail::defaultHttpCallback)
{
	server_->setConnectionCallback(
		std::bind(&HttpServer::onConnection, this, _1));
	server_->setMessageCallback(
		std::bind(&HttpServer::onMessage, this, _1, _2));

	SET_ASYNCLOG_BASENAME("HttpServer");
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
	//这里设定线程初始化函数，为每个线程创建一个上下文对象
	server_->setThreadInitCallback(
				std::bind(&ThreadLocal<HttpContext>::initValue, &threadLocalContext_));

	server_->start();
}

//TcpServer连接到来回调
void HttpServer::onConnection(const TcpConnectionShptr& conn)
{
//	printf("connection %s is %s\n",
//			conn->name().c_str(),
//			(conn->connected() ? "UP" : "DOWN"));
}

//TcpServer消息到来回调
void HttpServer::onMessage(const TcpConnectionShptr& conn, Buffer* buf)
{
	//获取当前线程私有的上下文对象
	HttpContext& httpContext = threadLocalContext_.value();
//	printf("tid = %d, &httpContext = %p\n", CurrentThread::tid(), &httpContext);

	if (!httpContext.parseRequest(buf)) {
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}
	else
	{
		const HttpRequest& request = httpContext.request();
		const std::string& connection = request.header("Connection");
		bool close = (connection == "close" ||
						request.version() == HttpRequest::kHttp10);

		HttpResponse response(close, static_cast<HttpResponse::Version>(request.version()));
		httpCallback_(request, &response);	//调用外部用户函数处理请求

		Buffer buf;
		response.appendToBuffer(&buf);
		conn->send(&buf);

		if (response.closeConnection()) {
			LOG_DEBUG << "close connection " << conn->name();
			conn->shutdown();
		}
		
		httpContext.reset();
	}
}
// Author Fangping Liu
// 2019-09-07

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "../Buffer.h"
#include <stdio.h>

namespace lfp::detail
{
	void defaultHttpCallback(const HttpRequest&, HttpResponse* response)
	{
		response->setStatusCode(HttpResponse::k404NotFound);
		response->setStatusMessage("Not Found");
		response->setCloseConnection(true);
	}

}	//end of namesapce lfp::detail

using namespace lfp;
using namespace std::placeholders; //std::bind( _1)


HttpServer::HttpServer(EventLoop* mainLoop,
			const InetAddress& listenAddr,
			int threadNum,
			const std::string& serverName)
  : server_(mainLoop, listenAddr, threadNum, serverName),
	httpContext_(),
  	httpCallback_(detail::defaultHttpCallback)
{
	server_.setConnectionCallback(
		std::bind(&HttpServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
	server_.start();
}

//TcpServer连接到来回调
void HttpServer::onConnection(const TcpConnectionShptr& conn)
{
	if (conn->isConnected()) {
		printf("onConnection(): new connection [%s] from %s is UP\n",
				conn->name().c_str(),
				conn->peerAddress().toIpPort().c_str());
	}
	else {
		printf("onConnection(): connection [%s] is DOWN\n",
				conn->name().c_str());
	}
}

//TcpServer消息到来回调
void HttpServer::onMessage(const TcpConnectionShptr& conn,
							Buffer* buf,
							Timestamp time)
{
	if (!httpContext_.parseRequest(buf)) {
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}
	else
	{
		const HttpRequest& request = httpContext_.request();
		const std::string& connection = request.header("Connection");
		bool close = (connection == "close" ||
						request.version() == HttpRequest::kHttp10);

		HttpResponse response(close, static_cast<HttpResponse::Version>(request.version()));
		httpCallback_(request, &response);	//调用外部用户函数处理请求

		Buffer buf;
		response.appendToBuffer(&buf);
		conn->send(&buf);

		if (response.closeConnection()) {
			conn->shutdown();
		}
		
		httpContext_.reset();
	}
}
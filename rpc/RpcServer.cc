// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include <matelib/base/Logging.h>
#include <matelib/Buffer.h>
#include "RpcServer.h"


using namespace lfp;
using namespace lfp::detail;
using namespace std::placeholders; //std::bind(_1)

RpcServer::RpcServer(EventLoop* baseLoop,
					const InetAddress& listenAddr,
					int threadNum,
					const std::string& serverName)
  : serverName_(serverName),
  	server_(new TcpServer(baseLoop, listenAddr, threadNum)),
	threadLocalSerializer_(),
	functions_()
{
	server_->setMessageCallback(
		std::bind(&RpcServer::onMessage, this, _1, _2));
	
	SET_ASYNCLOG_BASENAME(serverName_);
}

// 启动服务器
void RpcServer::start()
{
	//这里设定线程初始化函数，为每个线程创建一个上下文对象
	server_->setThreadInitCallback(
				std::bind(&ThreadLocal<Serializer>::initValue, &threadLocalSerializer_));

	server_->start();
}


void RpcServer::onMessage(const TcpConnectionShptr& conn, Buffer* buf)
{
//	STDLOG_INFO << "Recved " << buf->readableBytes() << " bytes data from " << conn->name();

	detail::Serializer& ds = threadLocalSerializer_.value();	//获取当前线程所绑定的Serializer对象

	ds.streamBuffer().swap(*buf);	//使用swap将buf交换到Serializer中，避免一次内存拷贝

	std::string funcname;
	ds >> funcname;

	if (functions_.find(funcname) == functions_.end())
	{
		ds.clear();
		ds << RpcStateCode(RPC_FUNCTION_NOTBIND);
		ds << std::string("function not bind: " + funcname);
	}
	else {
		auto fun = functions_[funcname];
		fun(ds);
	}

	conn->send(&ds.streamBuffer());
	ds.clear();		//数据发送完后也一定要清空序列化器，因为这是一个上下文对象而不是局部对象
}
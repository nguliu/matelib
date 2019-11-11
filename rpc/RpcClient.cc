// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#include <matelib/base/Logging.h>
#include <matelib/InetAddress.h>
#include "RpcClient.h"
#include <sys/socket.h>

using namespace lfp;

RpcClient::RpcClient()
  : stateCode_(RPC_SUCCESS),
	sockFd_(-1),
	serializer_()
{
	sockFd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockFd_ < 0) {
		int savedErrno = errno;
		STDLOG_ERROR << "syscall socket error: " << strerror_tl(savedErrno);
		abort();
	}

	SET_ASYNCLOG_BASENAME("rpc_client");
}


void RpcClient::connect(std::string ip, int port) {
	InetAddress addr(ip.c_str(), port);

	int i = 1;
	while (i < 11)
	{
		if (::connect(sockFd_, (const sockaddr*)&addr.getSockAddrInet(), sizeof(struct sockaddr)) == 0)
		{
			STDLOG_INFO << "connect to " << addr.toIpPort() << " succeed";
			return;
		}

		int savedErrno = errno;
		STDLOG_ERROR << "connect to " << addr.toIpPort() << " faild: " << strerror_tl(savedErrno);

		printf("The no.%d reconnection will be in %d seconds\n", i, i);
		sleep(i++);
	}
	exit(0);
}
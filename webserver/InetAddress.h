// Author Fangping Liu
// 2019-09-04

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include "base/copyable.h"
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

namespace lfp
{

	class InetAddress : copyable
	{
	public:
		explicit InetAddress(uint16_t port) {
			bzero(&addr_, sizeof addr_);
			addr_.sin_family = AF_INET;
			addr_.sin_addr.s_addr = htons(INADDR_ANY);
			addr_.sin_port = htons(port);
		}
		InetAddress(const std::string& ip, uint16_t port) {
			bzero(&addr_, sizeof addr_);
			addr_.sin_family = AF_INET;
			::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
			addr_.sin_port = ::htons(port);
		}
		InetAddress(const struct sockaddr_in& addr)
		  : addr_(addr)
		{ }

		const std::string toIpPort() const
		{
			char host[32];
			::inet_ntop(AF_INET, &addr_.sin_addr, host, static_cast<socklen_t>(sizeof host));
			uint16_t port = ::ntohs(addr_.sin_port);

			char buf[48];
			snprintf(buf, sizeof buf, "%s:%u", host, port);
			return buf;
		}

		const std::string toIp() const
		{
			char host[32];
			::inet_ntop(AF_INET, &addr_.sin_addr, host, static_cast<socklen_t>(sizeof host));

			return host;
		}

		const struct sockaddr_in& getSockAddrInet() const { return addr_; }
	private:
		struct sockaddr_in addr_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_INETADDRESS_H
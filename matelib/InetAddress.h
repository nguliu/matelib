// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_INETADDRESS_H
#define MATELIB_INETADDRESS_H

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
			addr_.sin_addr.s_addr = ::htonl(INADDR_ANY);
			addr_.sin_port = ::htons(port);
		}
		InetAddress(const char* ip, uint16_t port) {
			bzero(&addr_, sizeof addr_);
			addr_.sin_family = AF_INET;
			::inet_pton(AF_INET, ip, &addr_.sin_addr);
			addr_.sin_port = ::htons(port);
		}
		InetAddress(const struct sockaddr_in& addr)
		  : addr_(addr)
		{ }
		
		//operator= assignment返回自身引用是为了实现链式表达
		InetAddress& operator=(const InetAddress& rhs) {
			addr_ = rhs.addr_;
			return *this;
		}

		const std::string toPort() const
		{
			char buf[16];
			uint16_t port = ::ntohs(addr_.sin_port);
			snprintf(buf, sizeof buf, "%u", port);
			return buf;
		}

		const std::string toIp() const
		{
			char host[32];
			::inet_ntop(AF_INET, &addr_.sin_addr, host, static_cast<socklen_t>(sizeof host));
			return host;
		}

		const std::string toIpPort() const
		{
			return toIp() + ":" + toPort();
		}

		

		const struct sockaddr_in& getSockAddrInet() const { return addr_; }
	private:
		struct sockaddr_in addr_;
	};

} // namespace lfp

#endif // !MATELIB_INETADDRESS_H
// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_RPC_RPCCLIENT_H
#define MATELIB_RPC_RPCCLIENT_H

#include <matelib/base/noncopyable.h>
#include "Serializer.h"

#include <functional>
#include <string>
#include <memory>
#include <unistd.h>

namespace lfp
{
	class RpcClient : noncopyable
	{
	public:
		RpcClient();

		void connect(std::string ip, int port);

		template<typename RtTy, typename...Args>
		value_t<RtTy> call(std::string func, Args...args) {
			using ArgsTuple = std::tuple<typename std::decay<Args>::type...>;
			ArgsTuple tuple = std::make_tuple(args...);		//将参数打包为一个tuple

			serializer_ << func;	//将函数名和参数打包到客户端的序列化器
			serializer_.packageArgs(tuple);
			
			return netCall<RtTy>();
		}

		template<typename RtTy>
		value_t<RtTy> call(std::string func) {
			serializer_ << func;
			return netCall<RtTy>();
		}
	private:
		template<typename RtTy>
		value_t<RtTy> netCall();

		RpcStateCode stateCode_;
		int sockFd_;			  //客户端套接字
		detail::Serializer serializer_;	  //客户端序列化器
	};


	// 泛型方法不能在别的文件实现
	template<typename RtTy>
	inline value_t<RtTy> RpcClient::netCall()
	{
		bool error = (stateCode_ != RPC_RECV_TIMEOUT ? false : true);

		if (!error) {
			int n = serializer_.readableBytes();
			int ret = ::write(sockFd_, serializer_.current(), n);
			serializer_.clear();		//将序列化器清空
			if (ret != n) {
				error = true;
			}
		}
		if (!error) {
			int ret = serializer_.streamBuffer().readFd(sockFd_, nullptr);	//直接将数据读取到序列化器里
			if (ret <= 0) {
				error = true;
			}
		}
		if (!error) {
			value_t<RtTy> val;
			serializer_ >> val;
			serializer_.clear();		//将序列化器清空

			if (val.stateCode() == RPC_RECV_TIMEOUT) {
				stateCode_ = RPC_RECV_TIMEOUT;
			}

			return val;
		}
		
		// error
		stateCode_ = RPC_RECV_TIMEOUT;
		value_t<RtTy> val;
		val.setStateCode(RPC_RECV_TIMEOUT);
		val.setMessage("recv timeout");
		return val;
	}

} // namespace lfp

#endif // !MATELIB_RPC_RPCCLIENT_H
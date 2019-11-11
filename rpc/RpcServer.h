// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_RPC_RPCSERVER_H
#define MATELIB_RPC_RPCSERVER_H

#include <matelib/base/noncopyable.h>
#include <matelib/base/ThreadLocal.h>
#include <matelib/TcpServer.h>
#include "Serializer.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>

namespace lfp
{
namespace detail
{
	// 用tuple做参数调用函数
	template<typename Func, typename Tuple, std::size_t...I>
	decltype(auto) invoke_impl(Func&& func, Tuple&& t, std::index_sequence<I...>)
	{
		return func(std::get<I>(std::forward<Tuple>(t))...);
	}

	template<typename Func, typename Tuple>
	decltype(auto) invoke(Func&& func, Tuple&& t)	//将Tuple元素的索引展开后再进行调用
	{
		constexpr auto N = std::tuple_size<typename std::decay<Tuple>::type>::value;
		return invoke_impl(std::forward<Func>(func), std::forward<Tuple>(t), std::make_index_sequence<N>{});
	}

	// 调用帮助类，主要用于区别返回类型是否为void的情况
	template<typename RTy, typename Func, typename ArgsTuple>	//如果是void，返回的是int
	typename std::enable_if<std::is_same<RTy, void>::value, typename type_xx<RTy>::type>::type
	call_helper(Func func, ArgsTuple args) {
		invoke(func, args);
		return 0;
	}

	template<typename RTy, typename Func, typename ArgsTuple>	//如果不是void，返回的是结果
	typename std::enable_if<!std::is_same<RTy, void>::value, typename type_xx<RTy>::type >::type
	call_helper(Func func, ArgsTuple args) {
		return invoke(func, args);
	}

} // namespace detail

	class Buffer;
	class EventLoop;

	class RpcServer : noncopyable {
	public:
		RpcServer(EventLoop* baseLoop,
				  const InetAddress& listenAddr,
				  int threadNum,
				  const std::string& serverName = "rpc_server");

		void start();

		//绑定本地方法
		template<typename Func>		//自由函数
		void bind(const std::string& name, Func func)
		{
			functions_[name] = std::bind(&RpcServer::callproxy<Func>, this, func,	//func是真正需要调用的函数对象
											std::placeholders::_1);		//这里需要传入调用func的参数的序列
		}

		template<typename Func, typename Obj>	//成员函数
		void bind(const std::string& name, Func func, Obj* obj)
		{
			functions_[name] = std::bind(&RpcServer::callproxy<Func, Obj>, this, func, obj,	//func是真正需要调用的函数对象，obj是类实例
											std::placeholders::_1);		//这里需要传入调用func的参数的序列
		}

	private:
		//调用代理：用于调用自由函数
		template<typename Func>
		void callproxy(Func func, detail::Serializer& ds) {
			callproxyImpl(func, ds);
		}

		//调用代理：用于调用成员函数（非const版本）
		template<typename Func, typename Obj>
		void callproxy(Func func, Obj* obj, detail::Serializer& ds) {
			callproxyImpl(func, obj, ds);
		}
		//调用代理：用于调用成员函数（const版本）
		template<typename Func, typename Obj>
		void callproxy(Func func, const Obj* obj, detail::Serializer& ds) {
			callproxyImpl(func, obj, ds);
		}

	private:
		void onMessage(const TcpConnectionShptr& conn, Buffer* buf);	//TcpServer消息到来回调
		
		//用于调用自由函数指针
		template<typename RTy, typename...Args>
		void callproxyImpl(RTy(*func)(Args...), detail::Serializer& ds) {
			callproxyImpl(std::function<RTy(Args...)>(func), ds);
		}

		//用于调用类成员函数指针（非const版本）
		template<typename RTy, typename Class, typename Obj, typename...Args>
		void callproxyImpl(RTy(Class::*func)(Args...), Obj* obj, detail::Serializer& ds);
		//用于调用类成员函数指针（const版本）
		template<typename RTy, typename Class, typename Obj, typename...Args>
		void callproxyImpl(RTy(Class::*func)(Args...) const, const Obj* obj, detail::Serializer& ds);

		//用于调用function
		template<typename RTy, typename...Args>
		void callproxyImpl(std::function<RTy(Args...)> func, detail::Serializer& ds);

		std::string serverName_;
		std::unique_ptr<TcpServer> server_;
		ThreadLocal<detail::Serializer> threadLocalSerializer_;	//线程局部数据，每个线程一个Serializer对象

		std::map<std::string, std::function<void(detail::Serializer&)>> functions_;
	};


	//用于调用类成员函数指针（非const版本）
	template<typename RTy, typename Class, typename Obj, typename...Args>
	void RpcServer::callproxyImpl(RTy(Class::*func)(Args...), Obj* obj, detail::Serializer& ds)
	{
		using ArgsTuple = std::tuple<typename std::decay<Args>::type...>;
		constexpr auto argc = std::tuple_size<typename std::decay<ArgsTuple>::type>::value;

		ArgsTuple args = ds.getTuple<ArgsTuple>(std::make_index_sequence<argc>{});
		ds.clear();		//取出参数后一定记得清空序列化器

		auto ff = [=](Args...arg)->RTy {
			return (obj->*func)(arg...);
		};
		typename type_xx<RTy>::type ret = detail::call_helper<RTy>(ff, args);

		value_t<RTy> val;
		val.setStateCode(RPC_SUCCESS);
		val.setValue(ret);
		ds << val;
	}

	//用于调用类成员函数指针（非const版本）
	template<typename RTy, typename Class, typename Obj, typename...Args>
	void RpcServer::callproxyImpl(RTy(Class::*func)(Args...) const, const Obj* obj, detail::Serializer& ds)
	{
		using ArgsTuple = std::tuple<typename std::decay<Args>::type...>;
		constexpr auto argc = std::tuple_size<typename std::decay<ArgsTuple>::type>::value;

		ArgsTuple args = ds.getTuple<ArgsTuple>(std::make_index_sequence<argc>{});
		ds.clear();		//取出参数后一定记得清空序列化器

		auto ff = [=](Args...arg)->RTy {
			return (obj->*func)(arg...);
		};
		typename type_xx<RTy>::type ret = detail::call_helper<RTy>(ff, args);

		value_t<RTy> val;
		val.setStateCode(RPC_SUCCESS);
		val.setValue(ret);
		ds << val;
	}

	//用于调用function
	template<typename RTy, typename...Args>
	void RpcServer::callproxyImpl(std::function<RTy(Args...)> func, detail::Serializer& ds)
	{
		using ArgsTuple = std::tuple<typename std::decay<Args>::type...>;
		constexpr auto argc = std::tuple_size<typename std::decay<ArgsTuple>::type>::value;

		ArgsTuple args = ds.getTuple<ArgsTuple>(std::make_index_sequence<argc>{});
		ds.clear();		//取出参数后一定记得清空序列化器

		typename type_xx<RTy>::type ret = detail::call_helper<RTy>(func, args);

		value_t<RTy> val;
		val.setStateCode(RPC_SUCCESS);
		val.setValue(ret);
		ds << val;
	}

} // namespace lfp

#endif // !MATELIB_RPC_RPCSERVER_H
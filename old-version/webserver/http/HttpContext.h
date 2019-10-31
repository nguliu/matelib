// Author Fangping Liu
// 2019-09-07

#ifndef WEBSERVER_HTTPCONTEXT_H
#define WEBSERVER_HTTPCONTEXT_H


#include "../base/copyable.h"
#include "HttpRequest.h"

namespace lfp
{
	//HTTP请求解析类封装
	
	class Buffer;
	class HttpContext : copyable
	{
	public:
		enum HttpRequestParseState { //请求解析状态
			kExpectRequestLine,		 //正在解析请求行
			kExpectHeaders,			 //正在解析头部（请求行解析成功）
			kExpectBody,			 //正在解析实体（请求行和头部解析成功）
			kGotAll					 //全部解析成功
		};
		
		HttpContext()
		  : state_(kExpectRequestLine)
		{ }

		bool expectRequestLine() const {
			return state_ == kExpectRequestLine;
		}
		bool expectHeaders() const {
			return state_ == kExpectHeaders;
		}
		bool expectBody() const {
			return state_ == kExpectBody;
		}
		bool gotAll() const {
			return state_ == kGotAll;
		}

		void receiveRequestLine() {	//请求行解析成功
			state_ = kExpectHeaders;
		}

		void receiveHeaders() {	//头部解析成功
			state_ = kExpectBody;
		}

		void receiveBody() {	//body解析成功
			state_ = kGotAll;
		}

		void reset() {
			state_ = kExpectRequestLine;
			request_.reset();
		}

		HttpRequest& request() {
			return request_;
		}
		const HttpRequest& request() const {
			return request_;
		}

		//利用有限状态机解析HTTP请求
		bool parseRequest(Buffer* buf);

	private:
		//解析[begin, end)是否为正确的请求行
		bool processRequestLine(const char* begin, const char* end);

		HttpRequestParseState state_;
		HttpRequest request_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_HTTPCONTEXT_H
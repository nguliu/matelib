// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_HTTPRESPONSE_H
#define MATELIB_HTTPRESPONSE_H

#include <matelib/base/copyable.h>
#include <matelib/Buffer.h>
#include <map>
#include <string>
#include <stdio.h>


//以下是对HTTP响应消息体封装

namespace lfp
{
	class HttpResponse : copyable
	{
	public:
		enum HttpStatusCode {	//HTTP状态响应码
			kInvalid,
			k200Ok = 200,				// 成功
			k301MovedPermanently = 301,	// 301重定向，请求的页面永久性移至另一个地址
			k400BadRequest = 400,		// 错误的请求，语法格式有错，服务器无法处理此请求
			k404NotFound = 404			// 请求的网页不存在
		};
		enum Version {	//协议版本
			kUnknown = -1,
			kHttp10 = 0,
			kHttp11 = 1
		};
		
		HttpResponse(bool close, Version v)
		  : closeConnection_(close),
			version_(v),
			statusCode_(kInvalid)
		{ }

		void setCloseConnection(bool on) { closeConnection_ = on; }
		bool closeConnection() const  { return closeConnection_; }

		void setVeresion(Version v) { version_ = v; }

		void setStatusCode(HttpStatusCode status) { statusCode_ = status; }

		void setStatusMessage(const std::string& message) {
			statusMessage_ = message;
		}
	
		//设置文档媒体类型
		void setContentType(const std::string& type) {
			addHeader("Content-Type", type);
		}

		void addHeader(const std::string& feild, const std::string& value) {
			headers_[feild] = value;
		}

		void setBody(const std::string& body) {
			body_ = body;
		}
		
		//将当前对象的内容添加到buffer
		void appendToBuffer(Buffer* buffer) const
		{
			char buf[32];
			snprintf(buf, sizeof buf, "HTTP/1.%d %d", version_, statusCode_);
			buffer->append(buf);
			buffer->append(statusMessage_);
			buffer->append("\r\n");

			if (closeConnection_) {
				//如果是需要关闭，不需要告诉浏览器Content-Length它也能正确处理
				//因为已经关闭连接了，不存在粘包问题
				buffer->append("Connection: close\r\n");
			}
			else {
				snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
				buffer->append(buf);
				buffer->append("Connection: Keep-Alive\r\n");
			}

			//header列表
			for (auto it = headers_.begin(); it != headers_.end(); ++it) {
				buffer->append(it->first);
				buffer->append(": ");
				buffer->append(it->second);
				buffer->append("\r\n");
			}

			buffer->append("\r\n");	//header与body之间的空行
			buffer->append(body_);
		}

	private:
		bool closeConnection_;		 //是否短连接
		Version version_;			 //协议版本
		HttpStatusCode statusCode_;	 //状态响应码
		std::string statusMessage_;	 //响应状态码对应的文本信息
		std::map<std::string, std::string> headers_; //header列表
		std::string body_;	//实体
	};


} // namespace lfp

#endif // !MATELIB_HTTPRESPONSE_H
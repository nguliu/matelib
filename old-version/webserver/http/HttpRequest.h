// Author Fangping Liu
// 2019-09-07

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H


#include "../base/copyable.h"
#include <map>
#include <string>

namespace lfp
{
	//HTTP请求消息体封装

	class HttpRequest : copyable
	{
	public:
		enum Method {	//请求方法
			kInvalid, kGet, kPost, kHead, kPut, kDelete
		};
		enum Version {	//协议版本
			kUnknown = -1,
			kHttp10 = 0,
			kHttp11 = 1
		};

		HttpRequest()
		  : method_(kInvalid),
			version_(kUnknown)
		{ }

		void setVersion(Version v) { version_ = v; }
		Version version() const { return version_; }
		std::string versionToString() const {
			std::string v = "UNKNOWN";
			if (version_ == kHttp10)
				v = "HTTP/1.0";
			else if (version_ == kHttp11)
				v = "HTTP/1.1";
			return v;
		}

        bool setMethod(const char* start, const char* end)
        {
            std::string method(start, end);

            if (method == "GET") {
                method_ = kGet;
            }
            else if (method == "POST") {
                method_ = kPost;
            }
            else if (method == "HEAD") {
                method_ = kHead;
            }
            else if (method == "PUT") {
                method_ = kPut;
            }
            else if (method == "DELETE") {
                method_ = kDelete;
            }
            else {
                method_ = kInvalid;
            }

            return method_ != kInvalid;
        }

        Method method() const { return method_; }
        const char* methodToString() const
		{
            const char* method = "UNKNOWN";
            switch(method_)
            {
                case kGet:
                    method = "GET";
                    break;
                case kPost:
                    method = "POST";
                    break;
                case kHead:
                    method = "HEAD";
                    break;
                case kPut:
                    method = "PUT";
                    break;
                case kDelete:
                    method = "DELETE";
                    break;
                default:
                    break;
            }
			return method;
        }

		void setPath(const char* start, const char* end) {
			path_.assign(start, end);
		}
		const std::string& path() const { return path_; }

		void addHeader(const char* start, const char* colon, const char* end)
		{
			std::string field(start, colon);	//header域
			++colon;	//去掉冒号
			//去除左空格
			while (colon < end && isspace(*colon)) {
				++colon;
			}
			//去除右空格
			while (colon < end && isspace(*(end - 1))) {
				--end;
			}

			std::string value(colon, end);
			headers_[field] = value;
		}
		std::string header(const std::string& field) const {
			std::string result;
			auto it = headers_.find(field);
			if (it != headers_.end()) {
				result = it->second;
			}
			return result;
		}
		const std::map<std::string, std::string> headers() const {
			return headers_;
		}

		void reset() {
			method_ = kInvalid;
			path_.clear();
			version_ = kUnknown;
			headers_.clear();
			body_.clear();
		}
	
	private:
		Method method_;		//请求方法
		std::string path_;	//请求路径
		Version version_;	//协议版本
		std::map<std::string, std::string> headers_; //请求头部
		std::string body_;	//请求体
	};

} //end of namespace lfp

#endif //end of WEBSERVER_HTTPREQUEST_H

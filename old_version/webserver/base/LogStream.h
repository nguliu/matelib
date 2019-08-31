// Author Fangping Liu
// 2019-08-29

#ifndef WEBSERVER_BASE_LOGSTREAM_H
#define WEBSERVER_BASE_LOGSTREAM_H

#include "noncopyable.h"
#include <string>
#include <string.h>

namespace lfp
{

namespace detail
{
	const int kSmallBuffer = 4096;			// 4K
	const int kLargeBuffer = 4096 * 1024;	// 4M

	//日志应用层缓冲区。非类型模板参数，指定缓冲区大小
	template<int SIZE>
	class FixedBuffer : noncopyable
	{
	public:
		FixedBuffer()
		  : cur_(data_),
			bufSize_(SIZE)
		{
		}
		~FixedBuffer() { }

		void append(const char* str, size_t len)
		{
			if (avail() > static_cast<int>(len)) {
				memcpy(cur_, str, len);
				cur_ += len;
			}
		}

		const char* data() const { return data_; }
		char* current() { return cur_; }
		int length() const { return static_cast<int>(cur_ - data_); }
		int avail() const { return static_cast<int>(end() - cur_); }
		
		void add(size_t len) { cur_ += len; }
		void reset() { cur_ = data_; }
		void bzero() { ::bzero(data_, bufSize_); }
		const char* debugString() {  //将缓冲区内的数据当做字符串
			*cur_ = '\0';
			return data_;
		}

	private:
		const char* end() const { return data_ + bufSize_; }

		char data_[SIZE];
		char* cur_;
		int bufSize_;
	};

}  //end of namespace detail


	class LogStream : noncopyable
	{
	public:
		typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
		
		//以下返回LogStream&是为了实现链式表达（即连续输出日志）

		LogStream& operator<<(bool v)
		{
			buffer_.append(v ? "true" : "false", v ? 4 : 5);
			return *this;
		}
		LogStream& operator<<(char v)
		{
			buffer_.append(&v, 1);
			return *this;
		}

		LogStream& operator<<(short);
		LogStream& operator<<(unsigned short);
		LogStream& operator<<(int);
		LogStream& operator<<(unsigned int);
		LogStream& operator<<(long);
		LogStream& operator<<(unsigned long);
		LogStream& operator<<(long long);
		LogStream& operator<<(unsigned long long);

		LogStream& operator<<(float v)
		{
			*this << static_cast<double>(v);
			return *this;
		}
		LogStream& operator<<(double);

		//如果是一个指针则将指针所指地址格式化为十六进制格式
		LogStream& operator<<(const void*);

		//以下特化char*为格式化指针所指内容到缓冲区去
		LogStream& operator<<(const char* str)
		{
			if (str) {
				buffer_.append(str, strlen(str));
			}
			else {
				buffer_.append("(null)", 6);
			}
			return *this;
		}
		LogStream& operator<<(const unsigned char* str)
		{
			return operator<<(reinterpret_cast<const char*>(str));
		}

		LogStream& operator<<(const std::string& str)
		{
			buffer_.append(str.c_str(), str.size());
			return *this;
		}

		void append(const char* data, int len) { buffer_.append(data, len); }
		const Buffer& buffer() { return buffer_; }
		void resetBuffer() { buffer_.reset(); }

	private:
		template<typename T> void formatInterger(T); //格式化int型数据到缓冲区
		Buffer buffer_;

		static const int kMaxNumericSize = 32;  //格式化数字的最大字节数
	};

} //end of namespace lfp

#endif //end of WEBSERVER_BASE_LOGSTREAM_H
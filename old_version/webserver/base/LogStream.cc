// Author Fangping Liu
// 2019-08-29

#include "LogStream.h"
#include <algorithm> //std::reverse

namespace lfp
{

namespace detial
{
    const char digits[] = "9876543210123456789";
    const char* zero = digits + 9;
    const char digitsHex[] = "0123456789ABCDEF";


    //将整型的value装换为字符串放在buf中，返回字符串长度
    template<typename T>
	size_t convertInt(char buf[], T value)
	{
		T i = value;
		char* p = buf;

		do {
			int lsd = static_cast<int>(i % 10);
			i /= 10;
			*p++ = zero[lsd];
		} while (i != 0);

		if (value < 0) {
			*p++ = '-';
		}
		*p = '\0';
		std::reverse(buf, p);

		return p - buf;
	}

    //uintptr_t在32位平台下即unsigned int，64位平台下即unsigned long int
    size_t convertHex(char buf[], uintptr_t value)
    {
        uintptr_t i = value;
        char* p = buf;

        do {
            int lsd = i % 16;
            i /= 16;
            *p++ = digitsHex[lsd];
        } while (i != 0);

        *p = '\0';
        std::reverse(buf, p);

        return p - buf;
    }

} //end of namespace detial

} //end of namespace lfp


using namespace lfp;

LogStream& LogStream::operator<<(short v)
{
	*this << static_cast<int>(v);
	return *this;
}
LogStream& LogStream::operator<<(unsigned short v)
{
	*this << static_cast<unsigned int>(v);
	return *this;
}

LogStream& LogStream::operator<<(int v)
{
	formatInterger(v);
	return *this;
}
LogStream& LogStream::operator<<(unsigned int v)
{
	formatInterger(v);
	return *this;
}
LogStream& LogStream::operator<<(long v)
{
	formatInterger(v);
	return *this;
}
LogStream& LogStream::operator<<(unsigned long v)
{
	formatInterger(v);
	return *this;
}
LogStream& LogStream::operator<<(long long v)
{
	formatInterger(v);
	return *this;
}
LogStream& LogStream::operator<<(unsigned long long v)
{
	formatInterger(v);
	return *this;
}

LogStream& LogStream::operator<<(double v)
{
	// buffer容不下kMaxNumericSize个字符的话会被直接丢弃
	if (buffer_.avail() >= kMaxNumericSize) {
		int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
		buffer_.add(len);
	}
	return *this;
}

//如果是一个指针则将指针所指地址格式化为十六进制格式
LogStream& LogStream::operator<<(const void* p)
{
	uintptr_t v = reinterpret_cast<uintptr_t>(p);

	if (buffer_.avail() >= kMaxNumericSize)
	{
		char* buf = buffer_.current();
		buf[0] = '0';
		buf[1] = 'x';
		size_t len = detial::convertHex(buf + 2, v);
		buffer_.add(len + 2);
	}
	return *this;
}

//格式化int型数据到缓冲区
template<typename T>
void LogStream::formatInterger(T v)
{
	// buffer容不下kMaxNumericSize个字符的话会被直接丢弃
	if (buffer_.avail() >= kMaxNumericSize) {
		size_t len = detial::convertInt(buffer_.current(), v);
		buffer_.add(len);
	}
}
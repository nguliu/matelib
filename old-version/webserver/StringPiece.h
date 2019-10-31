// Author Fangping Liu
// 2019-09-05

#ifndef WEBSERVER_STRINGPIECE_H
#define WEBSERVER_STRINGPIECE_H

#include <string>
#include <string.h>

namespace lfp
{
	//StringPiece主要用于高效的字符串传递，减少内存拷贝
	class StringPiece
	{
	public:
		StringPiece()
		  : data_(nullptr), length_(0)
		{ }
		StringPiece(const char* data)
		  : data_(data), length_(static_cast<int>(strlen(data)))
		{ }
		StringPiece(const char* offset, int len)
		  : data_(offset), length_(len)
		{ }
		StringPiece(const std::string& str)
		  : data_(str.data()), length_(static_cast<int>(str.size()))
		{ }

		const char* data() const { return data_; }
		int size() const { return length_; }
		void clear() { data_ = nullptr; length_ = 0; }

		std::string asString() const {
			return std::string(data_, length_);
		}

	private:
		const char* data_;
		int length_;
	};

} //end of namespace lfp

#endif //end of WEBSERVER_TCPSERVER_H
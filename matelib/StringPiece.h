// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_STRINGPIECE_H
#define MATELIB_STRINGPIECE_H

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
		  : data_(data), length_(strlen(data))
		{ }
		StringPiece(const char* offset, size_t len)
		  : data_(offset), length_(len)
		{ }
		StringPiece(const std::string& str)
		  : data_(str.data()), length_(str.length())
		{ }

		const char* data() const { return data_; }
		size_t size() const { return length_; }
		void clear() { data_ = nullptr; length_ = 0; }

		std::string asString() const {
			return std::string(data_, length_);
		}

	private:
		const char* data_;
		size_t length_;
	};

} // namespace lfp

#endif // !MATELIB_STRINGPIECE_H
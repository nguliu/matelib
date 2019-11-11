// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BUFFER_H
#define MATELIB_BUFFER_H

#include "base/copyable.h"
#include "StringPiece.h"
#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>

namespace lfp
{
	//注意使用Buffer时的陷阱：
	//	  由于Buffer采用了惰性删除，所以使用peek()打印数据是不正确的，正确的做法是
	//	  使用 string str(buf.peek(), buf.readableBytes()) 将其构成字符串再打印

	class Buffer : copyable
	{
	public:
		static const size_t kCheapPrepend = 8;
		static const size_t kInitialSize = 1024;

		Buffer()	//默认构造函数
		  : buffer_(kCheapPrepend + kInitialSize),
			readIndex_(kCheapPrepend),
			writeIndex_(kCheapPrepend)
		{
		}
		Buffer(const Buffer& rhs)	//拷贝构造函数
		  : buffer_(kCheapPrepend + kInitialSize + rhs.readableBytes()),
			readIndex_(kCheapPrepend),
			writeIndex_(kCheapPrepend)
		{
			append(rhs.peek(), rhs.readableBytes());
			//将rhs的数据追加到当前对象，会有内存拷贝
		}
		Buffer(Buffer&& rhs)	//移动拷贝构造函数
		  : buffer_(kCheapPrepend + kInitialSize),
			readIndex_(kCheapPrepend),
			writeIndex_(kCheapPrepend)
		{
			this->swap(rhs);
			//将这里使用swap进行移动拷贝，不会有内存拷贝
		}
		Buffer(const char* data, size_t len)
		  : buffer_(kCheapPrepend + kInitialSize),
			readIndex_(kCheapPrepend),
			writeIndex_(kCheapPrepend)
		{
			append(data, len);
		}

		void swap(Buffer& rhs) {
			buffer_.swap(rhs.buffer_);
			std::swap(readIndex_, rhs.readIndex_);
			std::swap(writeIndex_, rhs.writeIndex_);
		}

		size_t readableBytes() const {
			return writeIndex_ - readIndex_;
		}
		size_t writeableBytes() const {
			return buffer_.size() - writeIndex_;
		}
		size_t prependableBytes() const {
			return readIndex_;
		}

		char* data() {
			return &*(buffer_.begin() + kCheapPrepend);
		}
		const char* data() const {
			return &*(buffer_.begin() + kCheapPrepend);
		}
		const char* peek() const {
			return &*(buffer_.begin() + readIndex_);
		}

		char* beginWrite() {
			return &*(buffer_.begin() + writeIndex_);
		}

		const char* beginWrite() const {
			return &*(buffer_.begin() + writeIndex_);
		}

		const char* findc(char c) const {
			return findStr(&c, 1);
		}
		const char* findCRLF() const {
			const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
			return crlf == beginWrite() ? nullptr : crlf;
		}
		const char* findCRLF(const char* start) const {
			assert(peek() <= start);
			assert(start <= beginWrite());

			const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
			return crlf == beginWrite() ? nullptr : crlf;
		}
		const char* findStr(const char* first, size_t len) const
		{
			assert(len <= readableBytes());

			const char* res = std::search(peek(), beginWrite(), first, first + len);
			return res == beginWrite() ? nullptr : res;
		}


		void retrieve(size_t n) {
			assert(n <= readableBytes());

			if (n < readableBytes()) {
				readIndex_ += n;
			}
			else {
				retrieveAll();
			}
		}
		void retrieveUntil(const char* end)
		{
			assert(peek() <= end);
			assert(end <= beginWrite());

			retrieve(end - peek());
		}
		void retrieveAll()
		{
			readIndex_ = kCheapPrepend;
			writeIndex_ = kCheapPrepend;
		}
		std::string retrieveAsString(size_t n) {
			assert(n <= readableBytes());

			std::string result(peek(), n);
			retrieve(n);
			return result;
		}
		std::string retrieveAllAsString() {
			return retrieveAsString(readableBytes());
		}

		void append(const StringPiece& str) {
			append(str.data(), str.size());
		}
		void append(const char* data, size_t len) {
			ensureWritableBytes(len);
			std::copy(data, data + len, beginWrite());
			writeIndex_ += len;
		}
		void append(const void* data, size_t len)
		{
			append(static_cast<const char*>(data), len);
		}

		void erase(const char* start, size_t len)
		{
			assert(peek() <= start);
			assert(start + len <= beginWrite());

			if (start == peek()) {
				retrieve(len);
			}
			else {
				std::vector<char>::iterator begin = buffer_.begin() + (start - &*buffer_.begin());
				std::vector<char>::iterator end = begin + len;
				buffer_.erase(begin, end);
				writeIndex_ -= len;
			}
		}
		void erase(const char* start, const char* end) {
			erase(start, end - start);
		}

		void clear() {
			::memset(data(), '\0', writeIndex_ - kCheapPrepend);	//清空[kCheapPrepend, writeIndex_)内的数据
			retrieveAll();
		}

		ssize_t readFd(int fd, int* savedErrno);

	private:
		void ensureWritableBytes(size_t len);

		std::vector<char> buffer_;		//以vector作为底层数据容器
		size_t readIndex_;
		size_t writeIndex_;
		
		static const char kCRLF[];
	};

} // namespace lfp

#endif // !MATELIB_BUFFER_H
// Author Fangping Liu
// 2019-09-03

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include "base/copyable.h"
#include "StringPiece.h"
#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>

namespace lfp
{
	//Buffer主要用于异步IO的应用层缓冲区
	class Buffer : copyable
	{
	public:
		static const size_t kCheapPrepend = 8;
		static const size_t kInitialSize = 1024;

		Buffer()
		  : buffer_(kCheapPrepend + kInitialSize),
			readIndex_(kCheapPrepend),
			writeIndex_(kCheapPrepend)
		{
		}

		void swap(Buffer& rhs) {
			buffer_.swap(rhs.buffer_);
			std::swap(readIndex_, rhs.readIndex_);
			std::swap(writeIndex_, rhs.writeIndex_);
		}

		size_t readableBytes() const {
			return writeIndex_ - readIndex_;
		}
		size_t writableBytes() const {
			return buffer_.size() - writeIndex_;
		}
		size_t prependableBytes() const {
			return readIndex_;
		}

		const char* peek() const {
			return &*buffer_.begin() + readIndex_;
		}

		char* beginWrite() {
			return &*buffer_.begin() + writeIndex_;
		}

		const char* beginWrite() const {
			return &*buffer_.begin() + writeIndex_;
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

		ssize_t readFd(int fd, int* savedErrno);

	private:
		void ensureWritableBytes(size_t len);

		std::vector<char> buffer_;
		size_t readIndex_;
		size_t writeIndex_;
		
		static const char kCRLF[];
	};

} //end of namespace lfp

#endif //end of WEBSERVER_BUFFER_H
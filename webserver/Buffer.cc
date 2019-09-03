// Author Fangping Liu
// 2019-09-03

#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

using namespace lfp;

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;
const char Buffer::kCRLF[] = "\r\n";


//结合栈上的空间，避免内存使用过大，提高内存使用率
//如果有5K个连接，每个连接就分配64K+64K的缓冲区的话，将占用640M内存，
//而大多数时候，这些缓冲区的使用率很低
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
	const size_t writable = writableBytes();

    // 节省一次ioctl系统调用（获取有多少可读数据）
    char extrabuf[65536];
    struct iovec vec[2];
    // 第一块缓冲区
    vec[0].iov_base = &*buffer_.begin() + writeIndex_;
    vec[0].iov_len = writable;
    // 第二块缓冲区
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const ssize_t n = ::readv(fd, vec, 2);

    if (n < 0) {
        *savedErrno = errno;
    }
    else if (n <= static_cast<ssize_t>(writable))	//第一块缓冲区足够容纳
    {
        writeIndex_ += n;
    }
    else  // 当前缓冲区，不够容纳，因而数据被接收到了第二块缓冲区extrabuf，将其append至buffer
    {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

void Buffer::ensureWritableBytes(size_t len)
{
	if (writableBytes() < len)
	{
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
			//如果所有空闲区域都不够则直接扩大
			buffer_.resize(writeIndex_+len);
		}
		else {
			//否则将数据向前移动
			assert(kCheapPrepend < readIndex_);

			size_t readable = readableBytes();
			std::copy(&*buffer_.begin() + readIndex_,
					  &*buffer_.begin() + writeIndex_,
					  &*buffer_.begin() + kCheapPrepend);

			readIndex_ = kCheapPrepend;
			writeIndex_ = readIndex_ + readable;
			assert(readable == readableBytes());
		}
	}
}
// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_COUNTDOWNLATCH_H
#define MATELIB_BASE_COUNTDOWNLATCH_H

#include "Condition.h"
#include "MutexLock.h"

namespace lfp
{
	class CountDownLatch : noncopyable
	{
	public:
		explicit CountDownLatch(int count)
		  : mutex_(),
			condition_(mutex_),
			count_(count)
		{ }

		void wait() {
			MutexLockGuard lock(mutex_);
			while (count_ > 0)
			{
				condition_.wait();
			}
		}

		void countDown() {
			MutexLockGuard lock(mutex_);
			--count_;
			if (count_ == 0)
			{
				condition_.notifyAll();
			}
		}

		int getCount() const {
			MutexLockGuard lock(mutex_);
			return count_;
		}

	private:
		mutable MutexLock mutex_;	//matable表示在const成员函数中也可以改变他的状态
		Condition condition_;
		int count_;
	};

} // namespace lfp

#endif // !MATELIB_BASE_COUNTDOWNLATCH_H
// Author Fangping Liu
// 2019-08-28

#ifndef WEBSERVER_BASE_COUNTDOWNLATCH_H
#define WEBSERVER_BASE_COUNTDOWNLATCH_H

#include "Condition.h"
#include "MutexLock.h"

namespace lfp
{

	class CountDownLatch : noncopyable
	{
	public:
		explicit CountDownLatch(int count);

		void wait();

		void countDown();

		int getCount() const;
	private:
		mutable MutexLock mutex_;
		Condition condition_;
		int count_;
	};

} //namespace lfp

#endif //WEBSERVER_BASE_COUNTDOWNLATCH_H
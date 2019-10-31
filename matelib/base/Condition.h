// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_CONDITION_H
#define MATELIB_BASE_CONDITION_H

#include "MutexLock.h"
#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

namespace lfp
{

	class Condition : noncopyable
	{
	public:
		Condition(MutexLock& mutex)
		  : mutex_(mutex)
		{
			int ret = pthread_cond_init(&cond_, nullptr);
			assert(ret == 0); (void)ret;
		}

		~Condition()
		{
			int ret = pthread_cond_destroy(&cond_);
			assert(ret == 0); (void)ret;
		}

		void wait()
		{
			pthread_cond_wait(&cond_, mutex_.getMutex());
		}

		// rturns true if time out, false otherwise
		bool waitForSeconds(int seconds)
		{
			struct timespec abstime;
			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec += static_cast<time_t>(seconds);
			return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getMutex(), &abstime);
		}

		void notify()
		{
			pthread_cond_signal(&cond_);
		}

		void notifyAll()
		{
			pthread_cond_broadcast(&cond_);
		}

	private:
		MutexLock& mutex_;
		pthread_cond_t cond_;
	};

} //namespace lfp

#endif // !MATELIB_BASE_CONDITION_H
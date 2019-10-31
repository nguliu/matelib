// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_MUTEXLOCK_H
#define MATELIB_BASE_MUTEXLOCK_H

#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>

namespace lfp
{
	class MutexLock : noncopyable
	{
	public:
		MutexLock()
		{
			int ret = pthread_mutex_init(&mutex_, nullptr);
			assert(ret == 0); (void)ret;
		}

		~MutexLock()
		{
			int ret = pthread_mutex_destroy(&mutex_);
			assert(ret == 0); (void)ret;
		}

		void lock()
		{
			pthread_mutex_lock(&mutex_);
		}

		void unlock()
		{
			pthread_mutex_unlock(&mutex_);
		}

		pthread_mutex_t *getMutex()
		{
			return &mutex_;
		}

	private:
		pthread_mutex_t mutex_;
	};


	class MutexLockGuard : noncopyable
	{
	public:
		explicit MutexLockGuard(MutexLock& mutex)
		  : mutex_(mutex)
		{
			mutex_.lock();
		}

		~MutexLockGuard()
		{
			mutex_.unlock();
		}

	private:
		MutexLock& mutex_;
	};

} //namespace lfp

#endif // !MATELIB_BASE_MUTEXLOCK_H
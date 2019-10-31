// Author Fangping Liu
// 2019-08-28

#ifndef WEBSERVER_BASE_CONDITION_H
#define WEBSERVER_BASE_CONDITION_H

#include "MutexLock.h"
#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>

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
		bool waitForSeconds(int seconds);

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

#endif //WEBSERVER_BASE_CONDITION_H
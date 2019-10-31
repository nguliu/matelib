// Author Fangping Liu
// 2019-08-28

#include "Condition.h"
#include <errno.h>
#include <time.h>

// rturns true if time out, false otherwise
bool lfp::Condition::waitForSeconds(int seconds)
{
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME, &abstime);
	abstime.tv_sec += static_cast<time_t>(seconds);
	return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getMutex(), &abstime);
}
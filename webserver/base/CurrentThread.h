// Author Fangping Liu
// 2019-08-28

#ifndef WEBSERVER_BASE_CURRENTTHREAD_H
#define WEBSERVER_BASE_CURRENTTHREAD_H

namespace lfp
{

namespace CurrentThread
{
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread const char* t_threadName;

	void cachedId();

	inline int tid()
	{
		if (t_cachedTid == 0) {
			cachedId();
		}
		return t_cachedTid;
	}

	inline const char* tidString()
	{
		return t_tidString;
	}

	inline const char* name()
	{
		return t_threadName;
	}

} //namespace CurrentThread

} //namespace lfp

#endif //WEBSERVER_BASE_CURRENTTHREAD_H
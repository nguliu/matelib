// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_CURRENTTHREAD_H
#define MATELIB_BASE_CURRENTTHREAD_H

namespace lfp
{
namespace CurrentThread
{
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread const char* t_threadName;

	//缓存当前线程的tid，在Thread.cc实现
	extern void cachedId();

	inline int tid()
	{
		if (t_cachedTid == 0) {
			cachedId();
		}
		return t_cachedTid;
	}

	inline const char* tidString()
	{
		if (t_cachedTid == 0) {
			cachedId();
		}
		return t_tidString;
	}

	inline const char* name()
	{
		return t_threadName;
	}

} //namespace CurrentThread
} //namespace lfp

#endif // !MATELIB_BASE_CURRENTTHREAD_H
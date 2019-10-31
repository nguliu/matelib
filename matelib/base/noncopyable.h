// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_NONCOPYABLE_H
#define MATELIB_BASE_NONCOPYABLE_H

namespace lfp
{
	// noncopyable是一个空基类，作为标识类，
	// 凡继承自noncopyable的类都是不可以拷贝的对象语意类型。
	class noncopyable
	{
	protected:
		noncopyable() {}
		~noncopyable() {}
	public:
		noncopyable(const noncopyable&) = delete;
		noncopyable& operator=(const noncopyable&) = delete;
	};

}

#endif // !MATELIB_BASE_NONCOPYABLE_H
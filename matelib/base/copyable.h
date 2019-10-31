// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_COPYABLE_H
#define MATELIB_BASE_COPYABLE_H

namespace lfp
{
	// copyable是一个空基类，作为标识类，
	// 凡继承自copyable的类都是可以拷贝的值语意类型。
	class copyable
	{
	protected:
		copyable() {}
		~copyable() {}
	};

}

#endif // !MATELIB_BASE_COPYABLE_H
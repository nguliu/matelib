// Author Fangping Liu
// 2019-08-28

#ifndef WEBSERVER_BASE_COPYABLE_H
#define WEBSERVER_BASE_COPYABLE_H

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

} //end of namespace lfp

#endif //end of WEBSERVER_BASE_COPYABLE_H
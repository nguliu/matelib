#ifndef WEBSERVER_CALLBACKS_H
#define WEBSERVER_CALLBACKS_H

#include <functional>

namespace lfp
{
	typedef std::function<void ()> EventCallback;
	
	typedef std::function<void ()> TimerCallback;

} //end of namespace lfp

#endif //end of WEBSERVER_CALLBACKS_H
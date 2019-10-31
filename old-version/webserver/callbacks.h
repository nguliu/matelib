#ifndef WEBSERVER_CALLBACKS_H
#define WEBSERVER_CALLBACKS_H

#include "base/Timestamp.h"
#include <functional>
#include <memory>

namespace lfp
{
	class Buffer;
	class EventLoop;
	class TcpConnection;

	typedef std::function<void ()> EventCallback;
	
	typedef std::function<void ()> TimerCallback;

	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	typedef std::shared_ptr<TcpConnection> TcpConnectionShptr;
	typedef std::function<void (const TcpConnectionShptr&)> ConnectionCallback;
	typedef std::function<void (const TcpConnectionShptr&)> CloseCallback;
	typedef std::function<void (const TcpConnectionShptr&, Buffer*, Timestamp)> MessageCallback;

} //end of namespace lfp

#endif //end of WEBSERVER_CALLBACKS_H
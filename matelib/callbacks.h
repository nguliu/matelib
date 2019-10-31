#ifndef MATELIB_CALLBACKS_H
#define MATELIB_CALLBACKS_H

#include <functional>
#include <memory>

namespace lfp
{
	class Buffer;
	class EventLoop;
	class TcpConnection;

	typedef std::shared_ptr<TcpConnection> TcpConnectionShptr;

	typedef std::function<void ()>	EventCallback;
	typedef std::function<void ()>	TimerCallback;
	typedef std::function<void(EventLoop*)>	ThreadInitCallback;

	typedef std::function<void (const TcpConnectionShptr&)> ConnectionCallback;
	typedef std::function<void (const TcpConnectionShptr&)> CleanupCallback;
	typedef std::function<void (const TcpConnectionShptr&, Buffer*)> MessageCallback;

} // namespace lfp

#endif // !MATELIB_CALLBACKS_H
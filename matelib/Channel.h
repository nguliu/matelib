// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_CHANNEL_H
#define MATELIB_CHANNEL_H

#include "callbacks.h"
#include "base/noncopyable.h"
#include <string>

namespace lfp
{
	class EventLoop;	//前向声明

	class Channel : noncopyable
	{
	public:
		Channel(EventLoop* loop, int fd);
		~Channel();

		void handleEvent();

		void setReadCallback(const EventCallback& cb)
		{ readCallback_ = cb; }
		void setWriteCallback(const EventCallback& cb)
		{ writeCallback_ = cb; }
		void setCloseCallback(const EventCallback& cb)
		{ closeCallback_ = cb; }
		void setErrorCallback(const EventCallback& cb)
		{ errorCallback_ = cb; }

		void enableReading() { events_ |= kReadEvent; update(); }
		void disableReading() { events_ &= ~kReadEvent; update(); }
		void enableWriting() { events_ |= kWriteEvent; update(); }
		void disableWriting() { events_ &= ~kWriteEvent; update(); }
		void disableAll() { events_ = kNoneEvent; update(); }
		bool isWriting() const { return events_ & kWriteEvent; }

		int fd() const { return fd_; }
		int events() const { return events_; }
		int revents() const { return revents_; }
		void setRevents(int revt) { revents_ = revt; }
		bool isNoneEvent() const { return events_ == kNoneEvent; }

		int state() const { return state_; }
		void setState(int stat) { state_ = stat; }

		EventLoop* ownerLoop() { return loop_; }
		void remove();	//移除当前通道

		// for debug
		std::string eventsToString() const;
		std::string reventsToString() const;
	
	private:
		void update();		//更新poller对当前Channel关注的事件

		static const int kNoneEvent;
		static const int kReadEvent;
		static const int kWriteEvent;

		EventLoop*	loop_;	//当前Channel所属的EventLoop
		const int	fd_;
		int			events_;
		int			revents_;
		int			state_;	//当前Channel的状态（空白的/关注的/保留的）
		bool		eventHandling_; //是否处于事件处理状态

		EventCallback readCallback_;
		EventCallback writeCallback_;
		EventCallback closeCallback_;
		EventCallback errorCallback_;
	};

} // namespace lfp

#endif // !MATELIB_CHANNEL_H
// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_EPOLLPOLLER_H
#define MATELIB_EPOLLPOLLER_H

#include "base/noncopyable.h"
#include "EventLoop.h"
#include <map>
#include <vector>

struct epoll_event;

namespace lfp
{
	class Channel;

	class EpollPoller : noncopyable
	{
	public:
		typedef std::vector<Channel*> ChannelList;

		EpollPoller(EventLoop* loop);
		~EpollPoller();

		int poll(int timeoutInMS, ChannelList* activeChannels);
		
		void updateChannel(Channel* channel);
		
		void removeChannel(Channel* channel);

		void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

	private:
		typedef std::vector<struct epoll_event> EventList;
		typedef std::map<int, Channel*> ChannelMap;

		void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

		void update(int operation, Channel* channel);

		EventLoop*	ownerLoop_;		//Poller所属EventLoop
		int			epollfd_;		//epoll进行查询的句柄
		EventList	events_;		//保存epoll返回的事件列表
		ChannelMap	channels_;		//保存每个fd到对应的channel

		static const int kInitEventListSize = 16;	//epoll返回的事件列表的初始大小
	};

} // namespace lfp

#endif // !MATELIB_EPOLLPOLLER_H
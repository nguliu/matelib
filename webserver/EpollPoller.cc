// Author Fangping Liu
// 2019-09-01

#include "base/Logging.h"
#include "Channel.h"
#include "EpollPoller.h"
#include <sys/epoll.h>
#include <unistd.h>

using namespace lfp;

//以下表示fd在epoll中的状态
namespace {
	const int kNew = -1;	//空白状态：epoll未关注过该fd，channels_中也没有该fd到对应channel的存储
	const int kAdded = 1;   //关注状态：epoll正在关注该fd，channels_中有该fd到对应channel的存储
	const int kDeleted = 2; //保留状态：epoll停止了对该fd的关注，channels_中保留该fd到对应channel的存储
}

EpollPoller::EpollPoller(EventLoop* loop)
  : ownerLoop_(loop),
	epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	events_(kInitEventListSize)
{
	if (epollfd_ < 0) {
		ASYNC_LOG << "EPollPoller::EPollPoller:epoll_create1 error";
		abort();
	}
}

EpollPoller::~EpollPoller()
{
	::close(epollfd_);
}

int EpollPoller::poll(int timeoutInMS, ChannelList* activeChannels)
{
	int numEvents = ::epoll_wait(epollfd_,
								 &*events_.begin(),
								 events_.size(),
								 timeoutInMS);
	if (numEvents > 0) {
		ASYNC_LOG << numEvents << " events happended in EPollPoller::poll()";

		fillActiveChannels(numEvents, activeChannels);
		
		//当用于保存epoll返回的事件列表被填满时需要将列表扩大。因为到这里之后events_内的
		//数据已无用处，所以这里采用交换进行扩大而不是直接扩大，避免了内存拷贝。
		if (static_cast<int>(events_.size()) == numEvents) {
			EventList tmpList(events_.size() * 2);
			events_.swap(tmpList);
			assert(events_.size() == tmpList.size() * 2);
		}
		//离开作用域后events_原来的内存(被tmpList接管)自动释放
	}
	else if(numEvents == 0) {
		SYNC_LOG << "Nothing happended in EPollPoller::poll()";
	}
	else {
		ASYNC_LOG << "EPollPoller::poll() error";
	}

	return numEvents;
}

void EpollPoller::updateChannel(Channel* channel)
{
	const int state = channel->state();

	ASYNC_LOG << "EPollPoller::updateChannel: fd=" << channel->fd()
			  << ",events=" << channel->eventsToString()
			  << ",state=" << (state < 0 ? "New" : (state == 1 ? "Concerned" : "Retained"));
    assertInLoopThread();
 
    if (state == kNew || state == kDeleted)
    {
        int fd = channel->fd();

        if (state == kNew) {  //新的channel
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;  //将新channel添加到channel列表
        }
        else //保留的channel
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->setState(kAdded);
        update(EPOLL_CTL_ADD, channel);	//添加该channel的事件关注
    }
    else
    {
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(state == kAdded);

        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel); //删除该channel的事件关注
            channel->setState(kDeleted);	//如果不关注当前channel的任何事件了，将channel设为保留状态
        }
        else	//否则只是修改epoll的关注事件，channel仍为关注状态
        {
            update(EPOLL_CTL_MOD, channel); //修改该channel的事件关注
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
	int fd = channel->fd();
	int state = channel->state();

	ASYNC_LOG << "EPollPoller::removeChannel: fd=" << channel->fd()
			  << ",events=" << channel->eventsToString()
			  << ",state=" << (state < 0 ? "New" : (state == 1 ? "Concerned" : "Retained"));
	assertInLoopThread();

	assert(channel->isNoneEvent());	//一定是没有关注任何事件的channel才能移除
	assert(channels_[fd] == channel);
	assert(state == kAdded || state == kDeleted);

	channels_.erase(fd);
	if (state == kAdded) {
		update(EPOLL_CTL_DEL, channel);
	}
	channel->setState(kNew);
}

/*epoll_event结构如下：
  struct epoll_event {
	  uint32_t	 events;	// Epoll return events
	  epoll_data_t data;	// User data variable
  };
  typedef union epoll_data {
     void    *ptr;
     int      fd;
     uint32_t u32;
     uint64_t u64;
 } epoll_data_t;
*/

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
	for (int i = 0; i < numEvents; ++i) {
		//这里 #1 处在epoll_event中保留了channel的指针
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

		channel->setRevents(events_[i].events);	  //将socket的事件设置到对应的channel
		activeChannels->push_back(channel);		  //将对应的channel加入活跃channel列表
	}
}

void EpollPoller::update(int operation, Channel* channel)
{
	struct epoll_event event;
	::bzero(&event, sizeof event);

	event.events = channel->events();
	event.data.ptr = channel;	//#1 在event中保存channel
	int fd = channel->fd();

	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
	{
		ASYNC_LOG << "epoll_ctl error,operation=" << operation << ",fd=" << fd;
		if (operation != EPOLL_CTL_DEL)
			abort();
	}
}
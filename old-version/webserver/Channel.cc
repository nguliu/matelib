// Author Fangping Liu
// 2019-09-01

#include "base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include <sstream>
#include <assert.h>
#include <sys/epoll.h>

using namespace lfp;


/*(e)poll事件:

	POLLIN			有数据可读。
	POLLPRI			有紧迫数据可读。
	POLLOUT			写数据不会导致阻塞。
	POLLHUP　　 	指定的文件描述符挂起事件。
	POLLRDHUP		对等方关闭
	POLLRDNORM		有普通数据可读。
	POLLRDBAND		有优先数据可读。
	POLLWRNORM		写普通数据不会导致阻塞。
	POLLWRBAND		写优先数据不会导致阻塞。
	POLLMSGSIGPOLL	消息可用。
	POLLERR			指定的文件描述符发生错误。
	POLLNVAL　　	指定的文件描述符非法。
*/

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
  	fd_(fd),
	events_(0),
	revents_(0),
	state_(-1),		//空白状态
	eventHandling_(false)
{
}

Channel::~Channel()
{
	assert(!eventHandling_);
}

void Channel::handleEvent()
{
	eventHandling_ = true;

	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))	//对等方挂起(HUP)事件
	{
		if (closeCallback_)
			closeCallback_();
	}
	if (revents_ & EPOLLERR) {	//套接字错误
		if (fd_ > 0) {			//如果大于0表示合法套接字发生错误，否则表示套接字非法
			ASYNC_LOG << "Channel::handleEvent() EPOLLERR: socket error";
			if (errorCallback_)
				errorCallback_();
		}
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		if (readCallback_)
			readCallback_();
	}
	if (revents_ & EPOLLOUT) {
		if (writeCallback_)
			writeCallback_();
	}

	eventHandling_ = false;
}


//移除当前通道
void Channel::remove()
{
	assert(isNoneEvent());
	loop_->removeChannel(this);
}

//更新poller对当前Channel关注的事件
void Channel::update()
{
	loop_->updateChannel(this);
}


// for debug
std::string Channel::eventsToString() const
{
    std::ostringstream oss;

    if (events_ == 0) {
        oss << "null ";
    }
	else {
		if (events_ & EPOLLIN)
			oss << "IN ";
		if (events_ & EPOLLPRI)
			oss << "PRI ";
		if (events_ & EPOLLOUT)
			oss << "OUT ";
		if (events_ & EPOLLHUP)
			oss << "HUP ";
		if (events_ & EPOLLRDHUP)
			oss << "RDHUP ";
		if (events_ & EPOLLERR)
			oss << "ERR ";
	}

    return oss.str().c_str();
}
// for debug
std::string Channel::reventsToString() const
{
    std::ostringstream oss;

    oss << "fd=" << fd_ << ",revents: ";
    if (revents_ & EPOLLIN)
        oss << "IN ";
    if (revents_ & EPOLLPRI)
        oss << "PRI ";
    if (revents_ & EPOLLOUT)
        oss << "OUT ";
    if (revents_ & EPOLLHUP)
        oss << "HUP ";
    if (revents_ & EPOLLRDHUP)
        oss << "RDHUP ";
    if (revents_ & EPOLLERR)
        oss << "ERR ";

    return oss.str().c_str();
}
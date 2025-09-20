#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

// channel未添加到Poller中
const int kNew = -1;    // Channel::index = -1
// channel已经添加到Poller中
const int kAdded = 1;
// channel已经删除过
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop* loop) 
    : Poller(loop) 
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) {
    if(epollfd_ < 0) {
        LOG_FATAL("epoll_create1 error : %d \n", errno);
    }
}

EPollPoller::~EPollPoller()  {
    ::close(epollfd_);
}

void EPollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    LOG_INFO("EPollPoller::updateChannel fd = %d events = %d index = %d", 
            channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted) {
        if(index == kNew) {
            // 这个channel之前没被添加过
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded); // 现在这个channel已经添加到channels中了
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // 这个channel之前被添加过
        int fd = channel->fd();
        if(channel->isNoneEvent()) {    // 这个channel对任何事件都无感
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    if(index == kAdded) {   // 表示epoll中还有这个fd
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    LOG_INFO("EPollPoller::poll fd total count:%lu", channels_.size());

    int numEvents = ::epoll_wait(epollfd_, (epoll_event*)(&*events_.begin()), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;  // 先保存下来
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        LOG_INFO("EPollPoller::poll %d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size()) {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents == 0) {
        LOG_INFO("EPollPoller::poll happened\n");
    }
    else {
        if(saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll error and ret < 0\n");
        }
    }
    return now;
}

// 将epoll_wait返回的发生事件fd的channel放入activeChannels中，交给eventLoop
void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

// 更新channel通道，内部调用epoll_ctl
void EPollPoller::update(int operation, Channel* channel) {
    int fd = channel->fd();
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();   // fd感兴趣的事件
    event.data.ptr = channel;
    // event.data.fd = fd;                 // 这个其实不用

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        // 出现异常
        if(operation == EPOLL_CTL_DEL)
            LOG_ERROR("delete failed\n");
        else
            LOG_ERROR("epoll modify or delete failed\n");
    }
}

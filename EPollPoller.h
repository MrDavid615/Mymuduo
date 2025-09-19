#ifndef __EPOLLPOLLER_H__
#define __EPOLLPOLLER_H__

#include "Poller.h"
#include "Channel.h"
#include <vector>
#include <sys/epoll.h>

/*
 epoll的使用
 epoll_creat(); 构造函数中调用
 epoll_ctl();   updateChannel中调用
 epoll_wait();  poll中调用
*/
class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    virtual void updateChannel(Channel* channel) override;
    virtual void removeChannel(Channel* channel) override;

private:
    const static int kInitEventListSize = 16;   // EventList的默认长度
    // 将epoll_wait返回的发生事件fd的channel放入activeChannels中，交给eventLoop
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    // 更新channel通道，内部调用epoll_ctl
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};

#endif

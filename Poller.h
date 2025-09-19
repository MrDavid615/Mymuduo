#ifndef __POLLER_H
#define __POLLER_H

#include "noncopyable.h"
#include "Timestamp.h"
#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

// 多路事件分发器的核心，IO复用
class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;  // 用于存放epoll_wait的输出

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 通过epoll_wait将发生事件的fd放入activeChannels容器中
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    // 更新通道上感兴趣的事件
    virtual void updateChannel(Channel* channel) = 0;
    // 将一个通道变为kNew同时在channels_中删除
    virtual void removeChannel(Channel* channel) = 0;

    // 判断channel是否在Poller中
    bool hasChannel(Channel* channel) const;
    // 获取事件循环的默认Poller
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    // socketfd : channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;
};

#endif

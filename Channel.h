#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "noncopyable.h"
#include "Timestamp.h"  // Channel::handleEvent(Timestamp) 直接传值，需要确定大小，必须包含Timestamp头文件
#include <functional>
#include <memory>

class EventLoop;

/* 
Channel 理解为通道，内部封装了sockfd和其感兴趣的evevt例如
EPOLLIN EPOLLOUT事件
还绑定了poller返回具体的事件
one loop per thread
eventLoop包含了channel list 与 一个 Poller 
channel内部包含event与fd
*/
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    // loop: 指定该channel属于哪个loop  fd: 监视的socket
    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到poller通知后处理事件
    void handleEvent(Timestamp receiveTime);    // 这里直接传值，需要确定大小，必须包含Timestamp头文件

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止channel被手动remove后channel还在执行回调
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    // epoll检测到事件后设置revents_ 调用该接口，由channel负责执行
    void set_revents(int revt) { revents_ = revt; }

    // 设置epoll中fd关注的事件类型 update()调用epoll_ctl将事件添加
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    // 检查当前fd事件状态，返回事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReading() const { return kReadEvent & events_;}
    bool isWriting() const { return kWriteEvent & events_; }

    int index() const { return index_; }
    void set_index(int idx) { index_ = idx; }

    // 获取当前channel所属的loop
    EventLoop* onwerLoop() { return loop_; }
    // 在channel所属的eventloop中删除这个channel
    void remove();

private:
    // 调用epoll_ctl将事件添加
    void update();
    // 根据poller返回的event类型执行对应的回调
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;   // 事件循环
    const int fd_;      // poller监听的对象
    int events_;        // 注册fd感兴趣的事件
    int revents_;       // poller返回的具体发生的事件
    int index_;         // 

    std::weak_ptr<void> tie_;
    bool tied_;

    // channel通道内可以获取fd发生的事件revent，所以它负责调用具体的事件回调操作
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
    ReadEventCallback readCallback_;
};

#endif
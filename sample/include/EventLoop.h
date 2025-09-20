#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;

// 时间循环类，包含两个大模块 Channel 与 Poller(epoll的抽象)
class EventLoop : noncopyable {
public:
    using Functor = std::function<void(void)>;

    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前Loop中执行
    void runInLoop(Functor cb);
    // 把cb放入队列中，唤醒loop所在的线程执行cb
    void queueInLoop(Functor cb);

    // MainLoop接受到新连接后唤醒SubLoop所在的线程，通过向wakeupFd_发送消息实现
    void wakeup();

    // 内部封装了Poller的update，用于更新Channel关心的事件
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    // wakeupFd_的回调
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  // 原子操作，底层通过CAS实现，循环是否实现
    std::atomic_bool quit_;      // 表示退出loop循环
    const pid_t threadId_;      // 记录当前loop所在线程的ID
    Timestamp pollReturnTime_;  // 记录发生事件的Channel的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;      // 当MainLoop获取一个新用户Channel，
                        // 通过轮询选择subLoop，subLoop可能是在挂起状态，通过该成员唤醒subLoop
    std::unique_ptr<Channel> wakeupChannel_;    // 绑定了wakeupFd_，用于唤醒subLoop

    ChannelList activeChannel_;     // Eventloop管理的所有channel

    std::atomic_bool callingPendingFunctors_;   // 标识当前loop是否有需要执行的回调
    std::vector<Functor> pendingFunctors_;  // 存储Loop所有需要执行的回调
    std::mutex mutex_;                      //  保护pendinfFunctors_容器的线程安全
};

#endif
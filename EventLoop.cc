#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

// 防止一个线程创建多个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;
// 定义默认超时事件
const int kPollTimeMs = 10000;


// 创建wakeupfd用于notify唤醒subLoop处理新来的Channel
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0) {
        LOG_FATAL("eventfd error: %d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop() 
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , callingPendingFunctors_(false)
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_)) {
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread) {
        // 这个线程已经创建一个EventLoop了
        LOG_FATAL("Another EventLoop %p is exit in this thread %d\n", t_loopInThisThread, threadId_);
    }
    else {
        t_loopInThisThread = this;
    }

    // 设置回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每个loop都监听了wakeupFd_的读事件
    wakeupChannel_->enableReading();
}


EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// SubLoop监听wakeupFd_，MainLoop可以向wakeupFd_发送消息来唤醒SubLoop
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof one) {
        LOG_ERROR("handleRead read %ld instead of 8", n);
    }

}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_INFO("eventLoop %p start looping", this);

    while(!quit_) {
        activeChannel_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannel_);
        for(Channel* channel : activeChannel_) {
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop需要执行的回调操作
        /**
         * IO线程就是MainLoop，运行accept获取发生连接的fd并使用channel打包fd
         * 并向wakeupFd_写数据，唤醒subloop
         * MainLoop事先注册一个回调，需要subloop执行
         * 回调放在doPendingFunctors中
        */
        doPendingFunctors();
    }

    LOG_INFO("eventLoop %p stop looping", this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if(!isInLoopThread()) {
        // 这里很关键，线程A调用了线程B的loop的quit
        // 防止线程B在阻塞状态，调用wakeup唤醒线程B的loop
        // loop执行while循环后退出
        wakeup();
    }
}

// 在当前Loop中执行
void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

// 把cb放入队列中，唤醒loop所在的线程执行cb
void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 唤醒相应需要执行上面回调操作的线程
    // || callingPendingFunctors_用来处理以下情况
    // 当前线程正在处理回调，这个时候又来了新的回调，必须向Poller中写入一个事件
    // 不然当前线程执行完又会阻塞在poll处
    if(!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }

}

// MainLoop接受到新连接后唤醒SubLoop所在的线程，通过向wakeupFd_发送消息实现
void EventLoop::wakeup() {
    uint64_t i = 1;
    ssize_t n = ::write(wakeupFd_, &i, sizeof(i));
    if(n != sizeof(i)) {
        LOG_ERROR("wakeup() write error\n");
    }
}

// 内部封装了Poller的update，用于更新Channel关心的事件，由Channel的update调用
// Channel与Poller无法直接沟通，因此需要使用EventLoop作为中继
void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functor;    // 用来装Functor;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functor.swap(pendingFunctors_);
    }

    // 这个时候其他的线程又可以向pendingFunctors_中装入回调了
    for(auto f : functor) {
        f();
    }

    callingPendingFunctors_ = false;
}


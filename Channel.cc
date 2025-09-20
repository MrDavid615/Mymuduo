#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"
#include <sys/epoll.h>

// 阉割，不包含poll仅包含epoll
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;     // EPOLLPRI紧急读事件
const int Channel::kWriteEvent = EPOLLOUT;

// 每个channel属于一个loop
Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0) 
    , index_(-1) 
    , tied_(false) {}

Channel::~Channel() {}

// TcpConnection新连接创建的时候调用该函数，之后调用回调时通过tie_检查引用计数
// 引用计数为0时表示TcpConnection已经不存在
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

// 当改变fd的事件后需要update负责在poller中更改，epoll调用epoll_ctl
void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if(tied_) { // 如果tied_被弱智能指针锁定，将其提升后再运行，防止意外析构
        guard = tie_.lock();
        if(guard) { // 提升成功，证明TcpConnection仍然存在
            handleEventWithGuard(receiveTime);
        }
        // else {} // 提升失败，证明TcpConnection已经析构，不执行回调
    }
    else {      // 对于MainLoop中的Acceptor，不需要TcpConnection，tie_为空
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp reveiveTime) {
    LOG_INFO("channel handleEvent revents: %d", revents_);

    if((revents_ & EPOLLHUP) && !(EPOLLIN & revents_) ) {
        if(closeCallback_) closeCallback_();
    }

    if((revents_ & EPOLLERR)) {
        if(errorCallback_) errorCallback_();
    }

    if((EPOLLIN | EPOLLPRI) & revents_) {
        if(readCallback_) readCallback_(reveiveTime);
    }

    if( revents_ & EPOLLOUT ) {
        if(writeCallback_) writeCallback_();
    }
}

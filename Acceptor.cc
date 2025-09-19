#include "Acceptor.h"
#include "Logger.h"
#include "EventLoop.h"
#include "Channel.h"
#include "InetAddress.h"
#include <sys/socket.h>


static int createNonblocking() {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(fd < 0) {
        LOG_FATAL("listen fd create fail\n");
    }
    return fd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) 
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false) { 
    acceptSocket_.setReusePort(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::headleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(acceptSocket_.fd());
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::headleRead() {
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0) {
        if(newConnectionCallback_) {
            // 轮询找到subLoop并分发fd
            newConnectionCallback_(connfd, peerAddr);
        }
        else {
            ::close(connfd);
        }
    }
    else {
        LOG_ERROR("accept error\n");
    }
}

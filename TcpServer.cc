#include "TcpServer.h"
#include "Logger.h"

#include <functional>

EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if(loop) {
        return loop;
    }
    else {
        LOG_FATAL("Main Loop ptr is NULL\n");
    }
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                     std::string nameArg, Option option)
    : loop_(loop)
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop_, listenAddr, option == kReusePort))
    , threadPool_(new EventLoopThreadPool(loop_, name_))
    , connectionCallback_()
    , messageCallback_()
    , nextConnId_()
{ 
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                        std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    auto IOloop = threadPool_->getNextLoop();
    IOloop->wakeup();
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

// 开启服务器监听，相当于开启MainLoop的Acceptor的listen
void TcpServer::start() {
    if(started_ ++ == 0) {   // 防止多次启动
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}


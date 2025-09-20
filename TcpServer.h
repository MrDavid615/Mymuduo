#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

/**
 * 用户使用muduo编写服务器程序
*/

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <atomic>
#include <memory>
#include <unordered_map>


class TcpServer : noncopyable {
public:
    // 启用一个subLoop时调用
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option {
        kNoReusePort, 
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                std::string nameArg, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);

    // 开启服务器监听，相当于开启MainLoop的Acceptor的listen
    void start();
private:
    // 向subLoop中分发channel就是调用这个函数，acceptor中跑的就是这个
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;   // 用户自己定义的Loop，就是MainLoop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;   // one loop per thread

    ConnectionCallback connectionCallback_;             // 有新连接时的回调
    MessageCallback messageCallback_;                   // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;       // 消息发送完成后的回调

    ThreadInitCallback threadInitCallback_;             // Loop线程初始化回调

    std::atomic<int> started_;

    int nextConnId_;
    ConnectionMap connections_;                         // 保存所有的连接
};

#endif
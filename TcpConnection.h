#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer通过Acceptor接口新用户连接，通过accept拿到connfd
 * 将connfd打包为TcpConnection，设置相应的回调，并放在Channel上
 * Poller监听到事件后调用Chanenl内的handleEvent执行回调
*/

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, 
                  const std::string &nameArg, 
                  int sockfd, 
                  const InetAddress& localAddr, 
                  const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void setConnectionCallback(ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    void setCloseCallback(const CloseCallback& cb){
        closeCallback_ = cb;
    }

    void setHighWaterMarkCallback(HighWaterMarkCallback& cb) {
        highWaterMarkCallback_ = cb;
    }

    // 发送数据
    void send(const std::string& buf);
    // 关闭当前连接
    void shutdown();

    // 建立连接
    void connectEstablished();
    // 销毁连接
    void connectDestoryed();

private:
    enum StateE {
        kDisconnected,  // 已经断开连接
        kConnecting,    // 正在连接
        kConnected,     // 已经连接
        kDisconnecting, // 正在断开连接
    };

    void setState(StateE state) {
        state_ = state;
    }

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    EventLoop* loop_;   // 多线程模式下，这个loop一定是subLoop
    const std::string name_;
    std::atomic_int state_; // 对应上面的枚举 StateE
    bool reading_;

    // 这里和Accept类似，Accept是在mainLoop里的，TcpConnection是在SubLoop里的
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;   // 主机的IP port
    const InetAddress peerAddr_;    // 客户端IP port

    ConnectionCallback connectionCallback_;         // 有新连接时的回调，与pollor无关
    MessageCallback messageCallback_;               // 有读写消息时的回调，与pollor无关
    WriteCompleteCallback writeCompleteCallback_;   // 消息发送完成后的回调，与pollor无关
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

#endif
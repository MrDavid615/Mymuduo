#include "TcpServer.h"
#include "Logger.h"

#include <string>
#include <functional>
#include <iostream>

/**
 * mymuduo1.0 示例demo 回声服务器
 * 
 * 使用方法：sample可以独立运行
 * 将/sample/build目录清空，并重新执行cmake.. 与 make命令即可完成编译
 * 生成的可执行文件在build目录中，名为EchoServer
 * */

 
class EchoServer {
public:
    // 使用面向对象的方式编程，面向过程也可以
    // TcpServer类没有默认构造因此EchoServer的构造函数参数列表主要是为了TcpServer的初始化
    EchoServer(EventLoop* loop, 
               const InetAddress& addr, 
               const std::string& name) 
        : loop_(loop)
        , server_(loop, addr, name)
    {
        // 注册回调函数，回调函数由个人实现，主要与业务有关，这里的回调将交给SubLoop中的Channel
        // 注册连接回调
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        // 注册消息回调，当有U读事件时生效
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        // 写事件完成回调
        server_.setWriteCompleteCallback(std::bind(&EchoServer::writeOver, this, std::placeholders::_1));

        // 设置合适的线程数量，不调用时默认仅有一个MainLoop，同时负责监听新连接与已连接事件的读写
        // 假设参数为n，相当于开启n个SubLoop，同时还有一个MainLoop，相当于n+1个线程，也就用n+1个Loop
        server_.setThreadNum(1);
    }

    void start() {
        server_.start();
    }
private:
    // 连接回调
    void onConnection(const TcpConnectionPtr& conn) {
        if(conn->connected()) {
            // 处理连接建立时的事件
            LOG_INFO("conn UP : %s\n", conn->peerAddress().toIpPort().c_str());
        }
        else {
            // 处理连接断开时的事件
            LOG_INFO("conn OVER : %s\n", conn->peerAddress().toIpPort().c_str());
        }
    }

    // 读回调，这个回调的参数包含一个缓冲区，还有一个时间戳
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        std::string msg = buf->retrieveAllAsString();
        
        LOG_INFO("EchoServer: get data from: %s and data is: %s\n", 
            conn->peerAddress().toIpPort().c_str(), msg.c_str());
        conn->send(msg);
        conn->shutdown();
    }

    // 写结束回调
    void writeOver(const TcpConnectionPtr& conn) {
        LOG_INFO("EchoServer: send data over\n");
    }

    EventLoop *loop_;       // 必须有一个loop，这个是MainLoop暨Reactor模型中的MainReactor
    TcpServer server_;      // TcpServer必须包含，这个是服务器的核心
};

int main() {
    EventLoop mainLoop;
    InetAddress addr(2000);     // 端口2000，IP默认为127.0.0.1
    EchoServer server(&mainLoop, addr, "EchoServer01");
    server.start();     // 开启服务器
    mainLoop.loop();    // 开启事件循环
    return 0;
}


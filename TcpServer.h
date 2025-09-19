#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

/**
 * 用户使用muduo编写服务器程序
*/

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"

#include <functional>
#include <string>

class TcpServer : noncopyable {
public:
    // 启用一个subLoop时调用
    using ThreadInitCallback = std::function<void(EventLoop*)>;
private:
    EventLoop* loop_;   // 用户自己定义的Loop
    const std::string ipPort_;
    const std::string name_;
};

#endif
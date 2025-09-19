#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <memory>
#include <functional>
#include "Timestamp.h"

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

#endif
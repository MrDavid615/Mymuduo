#ifndef __INET_ADDRESS_H__
#define __INET_ADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// 封装socket地址类型
class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr)
        : addr_(addr) {}

    // 获取IP地址的字符串
    std::string toIp() const;
    // 获取IP:port以字符串输出
    std::string toIpPort() const;
    // 获取port以无符号16位输出
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const {return &addr_;}
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
private:
    sockaddr_in addr_;
};

#endif

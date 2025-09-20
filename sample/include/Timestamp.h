#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <iostream>
#include <string>

class Timestamp {
public:
    //
    Timestamp();
    //
    explicit Timestamp(int64_t );   // 禁止隐式转换
    // 获取当前时间戳，配合toString使用
    static Timestamp now();
    // 将时间戳转化为字符串
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};

#endif


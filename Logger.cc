#include "Logger.h"
#include "Timestamp.h"
#include <iostream>

// 获取实例对象 单例模式
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

// 设置日志级别
void Logger::setLogLevel(int level) {
    logLevel_ = level;
}

// 写日志
void Logger::log(std::string msg) {
    switch(logLevel_) {
        case INFO:
            std::cout << "[INF0]";
            break;
        case ERROR:
            std::cout << "[ERROR]";
            break;
        case FATAL:
            std::cout << "[FATAL]";
            break;
        case DEBUG:
            std::cout << "[DEBUG]";
            break;
        default:
            break;
    }

    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}
    

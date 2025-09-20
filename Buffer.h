#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>
#include <string>
#include <algorithm>

// 网络库底层缓冲区定义
class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize) 
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}
    
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const {
        return readerIndex_;
    }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const {
        return begin() + readerIndex_;
    }

    // onMessage
    void retrieve(size_t len) {
        if(len < readableBytes()) {
            readerIndex_ += len;    // 只读取了可读缓冲区数据的一部分
        }
        else {
            retrieveAll();
        }
    }

    //  所有数据都已经读完，将可读区域覆盖
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的Buffer数据转换为string
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());   // 可读取数据的长度
    }   

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);  // 上面一句已经读取了缓冲区的可读数据，现在要将缓冲区复位
        return result;
    }

    // 确保写缓冲区空间大于len
    void ensureriteableBytes(size_t len) {
        if(writableBytes() < len) {
            makeSpace(len); // 扩容函数
        }
    }

    // 把data到data + len中的数据添加到writeable缓冲区中
    void append(const char* data, size_t len) {
        ensureriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    // 返回可写区域
    char* beginWrite() {
        return begin() + writerIndex_;
    }

    // 使用两个const才能声明常方法
    const char* beginWrite() const {
        return begin() + writerIndex_;
    }

    // 从fd里读数据
    ssize_t readFd(int fd, int* saveErrno);

    // 向fd写数据
    ssize_t writeFd(int fd, int* saveErrno);

private:
    char* begin() {
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len) {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
            // 已经读取完的缓冲区加目前可以写入的缓冲区不满足写入要求，buffer_需要扩容
            buffer_.resize(writerIndex_ + len);
        }
        else {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

#endif

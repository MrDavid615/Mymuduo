#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

/**
 * 从fd里读数据 Poller工作于水平触发LT模式，数据不读完一直通知
 * Buffer缓冲区有大小，从fd上读数据不知道TCP最终大小
 */
ssize_t Buffer::readFd(int fd, int* saveErrno) {
    char extrabuf[65536] = {0}; // 64k

    iovec vec[2];

    const size_t writable = writableBytes();    // 剩余空间不一定够
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);  // sizeof在编译器就告诉大小

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;   // 不够64k，选择2
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0) {
        *saveErrno = errno;
    }
    else if(n <= writable) {
        writerIndex_ += n;
    }
    else {  // extrabuf中也写入了数据
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

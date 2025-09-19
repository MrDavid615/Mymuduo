#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic<int> Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name) 
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name) {
    setDefaultName();
}

Thread::~Thread() {
    if(started_ && !joined_) {
        thread_->detach();  // 分离线程
    }
}

void Thread::start() {
    started_ = true;

    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() -> void {
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    } ));

    sem_wait(&sem); // 保证下面可以获取到tid
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName() {
    int num = numCreated_.fetch_add(1);
    if(name_.empty()) {
        name_ = std::to_string(num);
    }
}

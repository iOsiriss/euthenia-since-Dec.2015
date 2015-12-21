#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Session.h"
class OFPBuffer
{
public:
    OFPBuffer():is_empty_(true) {}
    void pushBack(Message message)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            is_empty_ = false;
            buffer_.push(message);
        }
        empty_.notify_one();
    }

    Message getFront()
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        while(is_empty_)
        {
            empty_.wait(lock);
        }
        Message rs =  buffer_.front();
        buffer_.pop();
        return rs;
    }

    size_t getSzie()
    {
        return buffer_.size();
    }
private:
    std::mutex queue_mutex_;
    std::condition_variable empty_;
    bool is_empty_;
    std::queue<Message> buffer_;
};

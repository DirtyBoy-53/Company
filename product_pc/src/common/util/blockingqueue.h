#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <comdll.h>

template <typename T> class  BlockingQueue
{
public:
        BlockingQueue() : mutex_(), cond_(), queue_() {
        }

        BlockingQueue(const BlockingQueue &) = delete;
        BlockingQueue &operator=(const BlockingQueue &) = delete;

        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            while(queue_.size() > 0) {
                queue_.pop_front();
            }
        }

        void put(const T &x) {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_back(x);
                cond_.notify_one();
        }

        void put(T &&x) {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_back(x);
                cond_.notify_one();
        }

        T take() {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_.wait(lock, [this] { return !queue_.empty(); });
                T front(std::move(queue_.front()));
                queue_.pop_front();
                return front;
        }

        size_t size() const {
                std::lock_guard<std::mutex> lock(mutex_);
                return queue_.size();
        }

private:
        mutable std::mutex mutex_;
        std::condition_variable cond_;
        std::deque<T> queue_;
};

typedef std::tuple<const char*, int> DataQueue;

#endif // BLOCKINGQUEUE_H

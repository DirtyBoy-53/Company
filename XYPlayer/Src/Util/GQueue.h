#ifndef GQUEUE_H
#define GQUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <QDebug>
template<typename T>
class GQueue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    GQueue() {}
    GQueue(GQueue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(const T& new_value)//
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void push(T&& new_value)//
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)//
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });

        value = data_queue.front();
        data_queue.pop();
    }
    bool wait_and_pop_fortime(T& value, int waitforms=100)
    {
        std::unique_lock<std::mutex> lk(mut);
        if (false == data_cond.wait_for(lk, std::chrono::milliseconds(waitforms), [this] {return !data_queue.empty(); }))
        {
            //timeout
            return false;
        }
        else
        {
            value = data_queue.front();
            data_queue.pop();
        }
        return true;
    }
    bool wait_and_fetch_fortime(T& value, int waitforms)
    {
        std::unique_lock<std::mutex> lk(mut);
        if (false == data_cond.wait_for(lk, std::chrono::milliseconds(waitforms), [this] {return !data_queue.empty(); }))
        {
            //timeout
            return false;
        }
        else
        {
            value = data_queue.front();
        }
        return true;
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
    int size()
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};

#endif // GQUEUE_H

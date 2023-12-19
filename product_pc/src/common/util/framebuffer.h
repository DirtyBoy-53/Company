#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <queue>
#include <mutex>
#include <iostream>
#include <QQueue>
#include <QJsonObject>
#include <comdll.h>

template <typename T> class Q_API_FUNCTION CFrameBuffer
{
public:
    CFrameBuffer() {}
    CFrameBuffer(const CFrameBuffer &) = delete;
    CFrameBuffer &operator=(const CFrameBuffer &) = delete;
    CFrameBuffer::~CFrameBuffer()
    {
        clear();
    }

    void push(T result)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_queue.size() >= m_maxSize) {
            m_queue.pop_front();
        }

        m_queue.push_back(result);
    }

    bool get(qint64 timeT, int limit, T& result)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        int value = -1;
        int max = limit;


        for (int i = 0; i<m_queue.size(); ++i) {
            qint64 ret = abs(std::get<0>(m_queue[i]) - timeT);
            if (ret <= max) {
                max = ret;
                value = i;
            }
        }

        if(value != -1) {
            result = m_queue[value];
        }

        return !(value == -1);
    }

    int size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while(m_queue.size() > 0) {
            m_queue.pop_front();
        }
    }

private:
    QQueue<T> m_queue;
    std::mutex m_mutex;
    int m_maxSize = 20;
};

#endif // FRAMEBUFFER_H

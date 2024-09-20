#ifndef YTHREAD_H
#define YTHREAD_H

#include <thread>
#include <atomic>
#include <chrono>

class YThread
{
public:
    enum Status {
        STOP,
        RUNNING,
        PAUSE,
    };

    enum SleepPolicy {
        YIELD,
        SLEEP_FOR,
        SLEEP_UNTIL,
        NO_SLEEP,
    };

    YThread() {
        m_status = STOP;
        m_status_changed = false;
        m_dotask_cnt = 0;
        m_sleep_policy = YIELD;
        m_sleep_ms = 0;
    }
    virtual ~YThread(){}

    void setStatus(Status stat) {
        m_status_changed = true;
        m_status = stat;
    }
    void setSleepPolicy(SleepPolicy policy, uint32_t ms = 0) {
        m_sleep_policy = policy;
        m_sleep_ms = ms;
        setStatus(m_status);
    }

    virtual int start() {
        if (m_status == STOP) {
            m_thread = std::thread([this] {
                if (!doPrepare()) return;
                setStatus(RUNNING);
                run();
                setStatus(STOP);
                if (!doFinish()) return;
            });
        }
        return 0;
    }

    virtual int stop() {
        if (m_status != STOP) {
            setStatus(STOP);
        }
        if (m_thread.joinable()) {
            m_thread.join();  // wait thread exit
        }
        return 0;
    }

    virtual int pause() {
        if (m_status == RUNNING) {
            setStatus(PAUSE);
        }
        return 0;
    }

    virtual int resume() {
        if (m_status == PAUSE) {
            setStatus(RUNNING);
        }
        return 0;
    }

    virtual void run() {
        while (m_status != STOP) {
            while (m_status == PAUSE) {
                std::this_thread::yield();
            }

            doTask();
            ++m_dotask_cnt;

            YThread::sleep();
        }
    }

    virtual bool doPrepare() {return true;}
    virtual void doTask() {}
    virtual bool doFinish() {return true;}

    std::thread m_thread;
    std::atomic<Status> m_status;
    uint32_t m_dotask_cnt;

protected:
    void sleep() {
        switch (m_sleep_policy) {
        case YIELD:
            std::this_thread::yield();
            break;
        case SLEEP_FOR:
            std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_ms));
            break;
        case SLEEP_UNTIL: {
            if (m_status_changed) {
                m_status_changed = false;
                m_base_tp = std::chrono::system_clock::now();
            }
            m_base_tp += std::chrono::milliseconds(m_sleep_ms);
            std::this_thread::sleep_until(m_base_tp);
        }
            break;
        default:    // donothing, go all out.
            break;
        }
    }

    SleepPolicy m_sleep_policy;
    uint32_t    m_sleep_ms;
    // for SLEEP_UNTIL
    std::atomic<bool> m_status_changed;
    std::chrono::system_clock::time_point m_base_tp;
};

#endif // YTHREAD_H

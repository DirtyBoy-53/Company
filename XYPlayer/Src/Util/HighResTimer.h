#ifndef HIGHRES_TIMER_H
#define HIGHRES_TIMER_H

#include <time.h>
#include <QTimer>
#include <QElapsedTimer>
#include <signal.h>
#include <unistd.h>


class HighResTimer {
public:
    HighResTimer(int interval_ms, std::function<void()> callback) : callback(callback) {
        struct sigevent sev;
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_value.sival_ptr = &timerid;
        sev.sigev_notify_function = &HighResTimer::timerHandler;
        sev.sigev_notify_attributes = nullptr;
        timer_create(CLOCK_MONOTONIC, &sev, &timerid);

        struct itimerspec its;
        its.it_value.tv_sec = interval_ms / 1000;
        its.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
        timer_settime(timerid, 0, &its, nullptr);
    }

    ~HighResTimer() {
        timer_delete(timerid);
    }

private:
    static void timerHandler(union sigval sv) {
        auto* instance = static_cast<HighResTimer*>(sv.sival_ptr);
        instance->callback();
    }

    timer_t timerid;
    std::function<void()> callback;
};

#endif

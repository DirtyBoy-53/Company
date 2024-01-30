#ifndef RECORDER_H
#define RECORDER_H
#include "sharedmembuffer.h"
#include "common.h"
#include <QByteArray>
#include <QQueue>
#include <QMutex>
#include <atomic>
#include <thread>

class Recorder
{
public:
    Recorder();
    void start(int channel);
    void stop();
private:
    void process();

    std::atomic_int m_iStart ;
    std::atomic_int m_channel ;
    QQueue<QByteArray> m_queue;
    std::thread * m_thread;
};

#endif // RECORDER_H

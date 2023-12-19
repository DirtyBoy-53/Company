#ifndef CLOG_H
#define CLOG_H

#include <QObject>
#include <thread>
#include "BlockingQueue.h"

class CLog
{
public:
    CLog();
    static CLog* getInstance();
    void init(QString pluginName);
    void deInit();
    QString getName();
    void addLog(QString log);

private:
    void writeThread();
    bool m_isRunning = false;
    std::thread m_writeThread;

    QString m_pluginName;
    BlockingQueue<QString> m_logList;

};

#endif // CLOG_H

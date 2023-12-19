#include "clog.h"
#include <QTextCodec>
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "util.h"

using namespace Util;

void outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString msgType;
    switch(type)
    {
        case QtDebugMsg:
            msgType = QString("Debug");
        break;
        case QtWarningMsg:
            msgType = QString("Warning");
        break;
        case QtInfoMsg:
            msgType = QString("Info");
        break;
        case QtCriticalMsg:
            msgType = QString("Error");
        break;
    }

    QString fileName = context.file;
    int pos = fileName.lastIndexOf('/');
    if(pos > 0) {
        fileName = fileName.right(fileName.length() - pos -1);
    }
//    QString strMessage = QString("<%1 %2> [%3:%4] : ").arg(time).arg(msgType).arg(fileName).arg(context.line);
    QString strMessage = QString("<%1 %2> : ").arg(time).arg(msgType);
    strMessage += msg;
    CLog::getInstance()->addLog(strMessage);
}

CLog::CLog()
{
//    qInstallMessageHandler(outputLog);
}

CLog* CLog::getInstance()
{
    static CLog instance;
    return &instance;
}

void CLog::init(QString pluginName)
{
    m_pluginName = pluginName;
    m_isRunning = true;
    m_writeThread = std::thread(&CLog::writeThread, this);
}

void CLog::deInit()
{
    m_isRunning = false;
    if(m_writeThread.joinable()) {
        m_writeThread.join();
    }
}

QString CLog::getName()
{
    return m_pluginName;
}

void CLog::addLog(QString log)
{
    m_logList.put(log);
}

void CLog::writeThread()
{
    while(m_isRunning) {
        if(m_logList.size() <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        QString logPath = QCoreApplication::applicationDirPath() + "/log/" + m_pluginName;
        Util::createFileDir(logPath);
        QString path = logPath + "/" + QDateTime::currentDateTime().toString("yyyy_MM_dd") + ".log";

        static QMutex mutex;
        mutex.lock();
        QFile file(path);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text(&file);
        QString msg = m_logList.take();
        text << msg << "\r\n";
        file.flush();
        file.close();
        printf("%s \n", msg.toStdString().c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mutex.unlock();
    }
}

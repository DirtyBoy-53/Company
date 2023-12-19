#include "csvlogger.h"
#include <QFile>
#include <QDebug>

QMutex CsvLogger::mMutex;
CsvLogger::CsvLogger()
{

}

CsvLogger::~CsvLogger()
{
}

void CsvLogger::setCsvLogPath(const QString &filePath)
{
    qDebug() << "change cvs log path " << filePath;
    m_logPath = filePath;
}

void CsvLogger::addCsvLog(const QStringList &data)
{
    QMutexLocker locker(&mMutex);
    QStringList last;
    foreach(QString a, data) {
        a = a.replace(',', ' ');
        last << a;
    }

    QFile file(m_logPath);
    if(file.open(QIODevice::Append)) {
        qDebug() << "addCsvLog " << last;
        QString logData;
        logData.append(last.join(','));
        file.write(logData.toLocal8Bit());
        file.write("\r\n");
        file.close();
    }
}

void CsvLogger::addCsvTitle(const QStringList &data)
{
    QMutexLocker locker(&mMutex);
    QFile file(m_logPath);
    if(file.exists()) {
        return;
    }

    if(file.open(QIODevice::Append)) {
        qDebug() << "add title " << data;
        QString logData;
        logData.append(data.join(','));
        file.write(logData.toLocal8Bit());
        file.write("\r");
        file.close();
    }
}

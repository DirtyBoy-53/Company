#ifndef CSVLOGGER_H
#define CSVLOGGER_H

#include <QObject>
#include <comdll.h>
#include <QMutex>
#include <QMutexLocker>

class Q_API_FUNCTION CsvLogger
{
public:
    CsvLogger();
    ~CsvLogger();

    void setCsvLogPath(const QString& filePath);
    void addCsvLog(const QStringList& data);
    void addCsvTitle(const QStringList &data);

private:
    QString m_logPath;
    bool m_isExist = false;
public:
    static QMutex mMutex;
};

#endif // CSVLOGGER_H

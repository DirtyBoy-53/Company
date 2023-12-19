#ifndef ASICSERIALMANAGER_H
#define ASICSERIALMANAGER_H

#include <QObject>
#include "cserialport.h"

class Q_API_FUNCTION ASicSerialManager : public QObject
{
    Q_OBJECT
public:
    explicit ASicSerialManager(QObject *parent = nullptr);

    static ASicSerialManager* getInstance();

    QString errString();

    bool openSerial(const QString& serialName);
    void closeSerial();
    bool waitForStartup(int timeout);
    bool writeSN(QString sn);
    bool readSN(QString& sn, int len = 17);
    bool readVersion(QString &ver);
    bool readHwVersion(QString &ver);
    bool getCrc(QString &sCrc);

    int getProductMode(int timeout);

    bool enterDebug(int timeout);
    bool burnFile(QString filePath, long addr);
    bool burnFileEx(long addr);
    bool enterMbMode(int timeout);
    bool enterPtzMode(int timeout);

private:
    int xOr(int iv, QString data);
    int c2n(char c);
private:

    CSerialPort* m_serial;

    QString mErrString;
signals:
    void sigLogOut(QString log, int i = 0);

};

#endif // ASICSERIALMANAGER_H

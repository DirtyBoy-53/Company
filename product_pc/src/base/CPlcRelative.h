#ifndef CPLCRELATIVE_H
#define CPLCRELATIVE_H

#include <QObject>
#include <QLibrary>
#include <QMutex>
#include <QMutexLocker>
#include "common_header.h"
#include "IDataLinkTransport.h"
#include "plccommandformat.h"


#define RETCMDLEN 19
#define POSLEN 16

class CPlcRelative : public QObject
{
    Q_OBJECT
public:
    CPlcRelative();

    static QMutex mPlcMutex;

    static CPlcRelative* instance();

    // sysInit
    bool getSerialResource();
    void freeSerialResource();
    QString& errorString();

    // testInit
    bool initResource(QString writeName, int writeBandrate, QString readName, int readBandrate, bool hex = false);
    void deInitResource();

    bool plcReset();
    bool plcTurnOver();// 翻转
    bool plcTurnBack();// 回转
    bool plcFixtureUp();
    bool plcFixtureDown();
    bool plcLightSetWork(bool bLight = true);
    bool plcLockStatus(bool& status);
    bool plcMoveTo20();
    bool plcMoveTo20T();
    bool plcMoveTo60();
    bool plcMoveTo60T();
    bool plcMoveToOrign();
    // 内参标定
    bool plcMoveUp();
    bool plcMoveDown();
    bool plcMoveOrign();
    bool plcMoveToAxis(unsigned short pos);
    bool boxPowerup();
    bool boxPoweroff();

private:
    // testExecute

    bool sendAndRead(QByteArray send, QByteArray& read, int timeout = 5000);
    bool sendAndFind(QByteArray send, QByteArray find, int timeout = 5000);
    bool slotMoveToPosition(PositionInfor pos);
private:
    QLibrary mLibSerial;
    IDataLinkTransport* mSerialRead;
    IDataLinkTransport* mSerialWrite;

    QString mDllPath;
    QString mErrString;

    PositionInfor mCurrentPosition;

    QMutex mPlcResetMutex;
    bool mPlcReseted;


    bool mPlcInitializeOk;
signals:
    void sigCurrentPosition(int pos);

public slots:
    void slotPlcMoveTo(int pos);
};


#endif // CPLCRELATIVE_H

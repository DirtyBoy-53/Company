#ifndef CTECRELATIVE_H
#define CTECRELATIVE_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QLibrary>
#include <QThread>

#include "IDataLinkTransport.h"

class CTecRelative : public QObject
{
    Q_OBJECT
public:
    CTecRelative();
    // sysInit
    bool getTecResource();
    void freeTecResource();
    QString errorString();

    bool initResource(QString name, int bandrate = 57600);
    void deInitResource();

    bool tecStartHeating(double maxTemp);
    bool tecStopHeating();
    double tecGetCurrentTempture();


private:
    IDataLinkTransport* tecSerialPtr;

    static QMutex mTecMutex;

    QLibrary mLibSerial;

    bool mBoolTecHasOpened;

    QString mDllPath;

    QString mErrString;

    bool mBoolIsHeating;

//signals:
//    void sigOpenOk();
//    void sigHeatingOk();
//    void sigUnHeatingOk();
//    void sigCrtTemp(double temp);

//public slots:
//    void slotHeating(double target);
//    void slotUnHeating();
//    void slotGetTemp(QString name);

};

#endif // CTECRELATIVE_H

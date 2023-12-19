#ifndef BURNINGMCUCB1000_H
#define BURNINGMCUCB1000_H

#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include "mcuserialmanager.h"
#include "ASicSerialManager.h"
#include <csvlogger.h>

class BurningMcuCB1000: public BaseProduce
{
    Q_OBJECT
public:
    BurningMcuCB1000();
    void step0();
    void getWorkInfo();
    void getSnPCBMcu();
    void comMcuVersion();
    void fileCheck();
    void saveResult();
    void burning();
    void comVol();
    void stepEnd();
    void waitver();
    void restart();
    void start();
    void checkJiaju();
    void writeSn();
    void checkCan();
    int checkAutority();

public slots:
    void slotsStoped();
private:
    void showItem(MesCheckItem item);
    MesCheckItem  *findItem(QString sItem);

    QString mSnPCBPower;
    QString m_versionPath;

    // 主机外设接口，俩串口
    McuSerialManager* mFixtureMcu = nullptr;

    McuSerialManager* mFixtureMcuCheck = nullptr;

    QList<QPair<QString, double>> mVoltageList;

    uint64_t m_workingTm;
    QString mTestStartTimeStr;
    QString mTestEndTimeStr;

    QString mSoftVersion;
    QString mCrtVersion;

    CsvLogger m_allCvsLog;
    bool m_workStatus = false;
    McuWorkInfo m_mcuInfo;
    ArmWorkInfo m_armInfo;
    QString m_readSn;
public slots:
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
    void slotConnectBtnClicked();
public:
    void getSn();
    void outMes();
    void enterMes();
};

#endif // BURNINGMCUCB1000_H

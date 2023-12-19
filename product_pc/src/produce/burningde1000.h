#ifndef BURNINGDE1000_H
#define BURNINGDE1000_H

#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include "mcuserialmanager.h"
#include "ASicSerialManager.h"
#include <csvlogger.h>

typedef struct {
    bool check;
    QString nameIO;
    QString nameItem;
    double valueMax;
    double valueMin;
} ParamVotageTest;

class BurningDe1000: public BaseProduce
{
    Q_OBJECT
public:
    BurningDe1000();
    void step0();
    void printSN();
    void checkSN();
    void getWorkInfo();
    void getSnPCBPower();
    void getSnPCBAsic();
    void getSnPCBSignal();
    void checkJiaju();
    void burning();
    void comAsicVersion();
    void fileCheck();
    void asicFileCheck();

    void saveResult();
    void comVol();
    void stepEnd();

    void start();
public slots:
    void slotsStoped();
private:
    void showItem(MesCheckItem item);
    MesCheckItem  *findItem(QString sItem);
    bool burnWithPython();
    bool burnWithLocal();

private:

    QString mSnPCBPower;
    QString mSnPCBAsic;
    QString mSnPCBSignal;

    // 主机外设接口，俩串口
    McuSerialManager* mFixtureMcu = nullptr;
    ASicSerialManager* mAsicDbgSerial = nullptr;

    McuSerialManager* mFixtureMcuCheck = nullptr;
    ASicSerialManager* mAsicDbgSerialCheck = nullptr;

    QVector<ParamVotageTest> mVoltageItemSet;
    QList<QPair<QString, double>> mVoltageList;

    uint64_t m_workingTm;
    QString mTestStartTimeStr;
    QString mTestEndTimeStr;

    QString mSoftVersion;
    QString mCrtVersion;
    QString m_readHwVersion;

    CsvLogger m_allCvsLog;
    bool m_workStatus = false;
    MesCheckItem * m_pItem;
    QString m_versionPath;
    quint32 m_crcApp;
    quint32 m_crcBoot;
    quint32 m_crcWc;
    quint32 m_crcSting;
    quint32 m_crcConfig;
    QString m_crcAll;
    McuWorkInfo m_mcuInfo;


    // 注意，要随着新的MCU软件下发更改，新的MCU支持阶梯式上电
    bool m_bOldMcu = true;
    // BaseProduce interface
public slots:
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
    void slotConnectBtnClicked();
    // BaseProduce interface
public:
    void getSn();
    void outMes();
    void enterMes();
};

#endif // BURNINGDE1000_H

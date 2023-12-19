#ifndef BURNINGARMCE1001_H
#define BURNINGARMCE1001_H
#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include "mcuserialmanager.h"
#include "ASicSerialManager.h"
#include <csvlogger.h>
#include <QLibrary>
#include "IDataLinkTransport.h"


class BurningArmCE1001: public BaseProduce
{
    Q_OBJECT
public:
    BurningArmCE1001();
    void step0();
    void getWorkInfo();
    void getSnPCBMcu();
    void comArmVersion();
    void fileCheck();
    void saveResult();
    void burning();
    void checkSum();
    void restart();
    void comVol();
    void stepEnd();
    void waitver();
    void start();
    void checkJiaju();
public slots:
    void slotsStoped();
private:
    void showItem(MesCheckItem item);
    MesCheckItem  *findItem(QString sItem);

    bool getArmSerialHdlc();
    bool openArmSerial();
    bool armCheckSum();
    void closeArmSerial();


    QString mSnPCBPower;
    QString m_versionPath;

    // 主机外设接口，俩串口
    McuSerialManager* mFixtureMcu = nullptr;

    McuSerialManager* mFixtureMcuCheck = nullptr;

    IDataLinkTransport* m_armDebugSerial = nullptr;

    QList<QPair<QString, double>> mVoltageList;

    uint64_t m_workingTm;
    QString mTestStartTimeStr;
    QString mTestEndTimeStr;

    QString mSoftVersion;
    QString mImgPath;
    QString mWorkPath;
    QString mCrtVersion;

    CsvLogger m_allCvsLog;
    bool m_workStatus = false;
    McuWorkInfo m_mcuInfo;
public slots:
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
    void slotConnectBtnClicked();
public:
    void getSn();
    void outMes();
    void enterMes();
};

#endif // BURNINGARMCE1001_H

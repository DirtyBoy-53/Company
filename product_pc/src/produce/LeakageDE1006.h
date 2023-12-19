#ifndef LEAKAGEDE1006_H
#define LEAKAGEDE1006_H

#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include <CLeakageRelative.h>
#include <QElapsedTimer>
#include <QDateTime>
#include <XyScanDialog.h>
#include <mescom.h>

#include "LeakagePLC.h"
#include "LeakageM5001A.h"
#include <QDebug>

#define FALSE_RETURN(x) { \
 if (!x) { \
    qDebug() << #x << "is error"; \
    return false; \
 } \
}

class LeakageDE1006: public BaseProduce
{
    Q_OBJECT
public:
    LeakageDE1006();
    ~LeakageDE1006();
    // 1. 连接串口
    // 2. 复位
    // 3. 发送开始测试
    // 4. 循环监测
    // - 4 充气
    // - 5 保压，结果监测，泄露大于30，报错
    // - 6 测试，结果监测，泄露大于50，报错
    // - 7 泄气
    // 5. 测试完成
    // -1. 断开串口
    void step0();
    void step1();
    void step2();
    void step3();
    void step4();
    void step5();

    void pumpStep();
    void mesInforClear();
    void mesAddInfor(int seq, QString value, QString rst = "NG", QString err="");
    void mesUpload();

    void start();

    void init();

    // 新的设备
    // 1. 开启串口资源
    // 2. 提示扫码,放产品
    // 3. PLC设备解锁，侦测是否解锁
    // 4. PLC不在位检测，并上锁
    // 5. PLC到位检测，PLC原定检测
    bool CycleInit();

    bool getSnAndExchangeDUT();

    bool readyToTest();

    bool leakageTest();

    void CycleExit();



private:
    LeakageM5001A m_leakageM5001A;
    LeakagePLC m_leakagePlc;
    bool m_isNewDevice;
    bool m_lastTestResult = true;

    CSerialDirect *m_handleScanner{nullptr}; // 用时开
    QString m_portScanner = "";

    void addLog(QString log, bool rst = true)
    {
        if (rst) {
            logAddNormalLog(log);
            showProcess(log);
        } else {
            logAddFailLog(log);
            showFail(log);
        }
    }


private:
    QString getAlarmString(int alarmCode);

public slots:
    void slotsStoped();
private:
    QString mSnPCBPower;
    QString mSnPCBAsic;
    QString mSnPCBSignal;


    CLeakageRelative mLeakageBase;
    bool mBoolMesIgnore;

    QString m_plcSerialName;
    QString mSerialName;
    int mFailLimitPressure;
    int mFailLimitLeakage;

    int mErrorCode;
    QString mErrString;

    QString mLogPath;
    QString mCurrentSN;

    QDateTime mStartTime;
    float mPressureValue;
    float mLeakageValue;
    QElapsedTimer mTimer;

    XyScanDialog* mScanDlg;
    MesCom mMesObj;
private:

    // true
    bool testCompleteJugde(unsigned char step, unsigned char status);
    // ComState interface
public:
//    void getSn() override;

    // BaseProduce interface
    bool getAutoBarcode(QString &sn);
public slots:
    void slotConnectBtnClicked();
    void slotStartWorkBtnClicked();
    void slotDisConnectBtnClicked();
    void slotStopWorkBtnClicked();
};
#endif // LEAKAGEDE1006_H

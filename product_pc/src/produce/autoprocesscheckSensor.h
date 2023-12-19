#ifndef AUTOPROCESSCHECKSENSOR_H
#define AUTOPROCESSCHECKSENSOR_H

#include <QObject>
#include <thread>
#include <QStateMachine>
#include <QTimer>
#include "WebSocketClientManager.h"
#include "baseproduce.h"
#include "mcuserialmanager.h"
#include "asiccontrol.h"
#include "csvlogger.h"
#include <blockingqueue.h>

typedef enum {
    PowerSupply_None = -1,
    PowerSupply_Arm = 0,
    PowerSupply_Sensor = 1
}PowerSupplyWay;

class AutoProcessCheckSensor : public BaseProduce
{
    Q_OBJECT
public:
    AutoProcessCheckSensor();
    bool init();

    void stopWork();

public slots:
    void slotRecvVideoFrameWsIvi(QByteArray frameData, FrameInfo_S info);
    void slotsStoped();
    void slotConnectBtnClicked();

private slots:
    void checkIoState();
    void workStepPowerOn();
    void getSn();
    void workStepCompareVersion();
    void workCheckAgeFlag();
    void workStepCheckPower();
    void workStepVoltageWide();
    void workStepWindowHot();
    void workStepCheckShutter();
    void workCheckVideoAuto();
    void workCheckVideoManual();
    void workPrinter();
    void workCompareSn();
    void workCheckCalibIn();
    void checkNext();
    void workReset();
    void mesUpload();
    void reportResult();
    void saveCsvLog();
    //横纹
    void EvaluateHorStripe();
    // 快门检测
    bool checkShutter();

    void checkStepCheckVideo(bool result);
private:
    void initStateMachine();
    void updateTableResult(QStringList list);
    void addLog(QString log, const int result = 0);
    bool testPing(QString ip);
    bool dutSaveImage(QString name, QByteArray data);
    bool widePowerTest(float targetVol, float &power);
    void getBit8FromY16(QByteArray src, unsigned char *&dest);

    WebSocketClientManager* m_webSocketIvi = nullptr;
    McuSerialManager* m_mcuSerial = nullptr;
    AsicControl* m_asic = nullptr;
//    int m_errorCode = 0;
    qint64 m_startTime = 0;
    qint64 m_endTime = 0;

    float m_dataVoltage = .0f;

    bool m_isWaiting = false;
    int m_waitResult = 0;

    QString m_snPrint;

    bool m_ioState = false;
    int m_iviFrameCount = 0;
    QList<MesCheckItem> m_mesPackage;

    int m_vol;

    QStringList m_reportList;
    CsvLogger m_csvLogger;
    unsigned char* m_frameData = nullptr;
    unsigned char* m_frameY = nullptr;
    BlockingQueue<QByteArray> m_videoQueue;
    std::atomic_int m_start;

    QString m_logPath;
    int m_seqImage;

    int m_timesCheckHot = 5;
    PowerSupplyWay m_powerSupply = PowerSupply_Arm; // 0 - ARM, 1 - Sensor
};


#endif // AUTOPROCESSCHECKSENSOR_H

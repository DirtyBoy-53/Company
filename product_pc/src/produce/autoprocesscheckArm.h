#ifndef AUTOPROCESSCHECKARM_H
#define AUTOPROCESSCHECKARM_H
#include <QObject>
#include <thread>
#include <QStateMachine>
#include <QTimer>
#include "WebSocketClientManager.h"
#include "baseproduce.h"
#include "mcuserialmanager.h"
#include "csvlogger.h"
#include "CSerialDirect.h"

#define LENGTH_8 8
#define LENGTH_6 6
class AutoProcessCheckArm : public BaseProduce
{
    Q_OBJECT
public:
    AutoProcessCheckArm();
    ~AutoProcessCheckArm();
    void init();

public slots:
    void stopWork();
    void slotsStoped();
    void slotRecvVideoFrameWsIvi(QByteArray frameData, FrameInfo_S info);
    void slotRecvVideoFrameWsLvds(QByteArray frameData, FrameInfo_S info);
    void checkIoState();
    void slotConnectBtnClicked();

private slots:
    void checkNext();
    void workStepPowerOn();
    void workStepCompareVersion();
    void workStepOemVersion();
    void workStepCheckPower();
    void workCheckLvds();
    void workCheckCan();
    void workCheckRs232();
    void getSn();
    void workPrinter();
    void workCompareSn();
    void workStepVoltageWide();
    void workStepCheckUsb();
    void workStepCheckAlgo();
    void mesUpload();
    void reportResult();

private:
    bool checkAlgoBox();
    bool widePowerTest(float targetVol, float &power);
    void initStateMachine();
    void updateTableResult(QStringList list);
    void addLog(QString log, const int result = 0);
    bool checkComport(CSerialDirect* handle, QString port);
    bool checkMcuPort(QString port);

    bool writePlc(unsigned char cmd[], int len);

    bool openFixture(int timeout = 3600000);

    bool waitForOpenFixture(int timeout = 3600000);

    bool waitForCloseFixture(int timeout = 3600000);
    bool watiForButtonPress(int timeout = 3600000);
    QString waitForOkNgSignal(int timeout = 3600000);

    bool checkProductRs232();

    bool getAutoBarcode(QString& sn);

    QString scanAutoBarcode();


private:

    WebSocketClientManager* m_webSocketIvi = nullptr;
    WebSocketClientManager* m_webSocketLvds = nullptr;

    qint64 m_startTime = 0;
    qint64 m_endTime = 0;
    QList<MesCheckItem> m_mesPackage;

    int m_lvdsFrameCount = 0;
    int m_iviFrameCount = 0;

    bool m_ioState = false;

    QStringList m_reportList;
    CsvLogger m_csvLogger;

    float m_lastWideVoltage = .0f;
    float m_lastWidePower = .0f;

    QString m_oemHwVersion;
    QString m_oemSwVersion;

    McuSerialManager* m_mcuSerial = nullptr; // 一直开
    CSerialDirect *m_handlePlc = nullptr; // 一直开
    CSerialDirect *m_handleScanner = nullptr; // 用时开
    CSerialDirect *m_handleProduct = nullptr; // 用时开

    QString m_portMcu = "";
    QString m_portPlc = "";
    QString m_portScanner = "";
    QString m_portProduct = "";

    QByteArray m_imgBuffer;
    uchar *m_dataImg;
};

#endif // AUTOPROCESSCHECKARM_H

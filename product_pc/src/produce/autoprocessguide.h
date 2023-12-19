#ifndef AUTOPROCESSGUIDE_H
#define AUTOPROCESSGUIDE_H

#include <CPingTest.h>
#include "baseproduce.h"
#include "comguidecfg.h"
#include "WebSocketClientManager.h"
#include <QQueue>
#include <HttpClient.h>

class AutoProcessComGuide : public BaseProduce
{
    Q_OBJECT
public:
    AutoProcessComGuide(const int type = 0);

    void getSn();
    void getLenSn();
    void getInnerSn();
public slots:
    void slotsStoped();
    void mesUpload();
    void slotConnectBtnClicked();
    void slotRecvVideoFrameWs(QByteArray frameData, FrameInfo_S frameInfo);

private:
    void initTipsInfo();
    void initStateMachine();
    void tipsWork();
    void addLog(QString log, const int  result = 0);
    void updateTable(QString name, qint64 workTime, bool result);
    void stopWork();
    void checkNext();
    void checkVideo();

    void getSensorSn();
    void getRegister();
    void checkVideoAuto();
    void checkShutterAuto();
    void checkVideoManual();
    void printerWork();
    void workCompareSn();
    void checkRegister();


    bool dutSaveImage(QString name, QByteArray data);
    void getBit8FromY16(QByteArray src, unsigned char* & dest);
    bool dutGetRegister_i2c(int page, int addr, int &data);
    bool dutGetRegister_com(int addr, int &data);
    bool getRegisterDataI2c();
    bool getRegisterDataCom();
    void saveCsvReport();
    bool checkShutter();


    bool dutPing(QString ip);

private:
    int m_dataHssd = 0;
    int m_dataRaselLow = 0;
    int m_dataRaselHigh = 0;
    int m_dataRasel = 0;

    WebSocketClientManager* m_webSocket = nullptr;
    QList<GuideCfgInfo> m_tipsList;
    QString m_lenSn;
    int m_type;
    int m_frameCount = 0;
    QString m_imgPath;
    QString m_completeSn;

    QQueue<QByteArray> m_videoQueue;

    QString m_ipAddress;

    int m_seqImage = 0;

    QString m_logPath;

    QString m_sensorCode;

};

#endif // AUTOPROCESSGUIDE_H

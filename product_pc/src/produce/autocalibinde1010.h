#ifndef AUTOCALIBINDE1010_H
#define AUTOCALIBINDE1010_H

#include "baseproduce.h"
#include <WebSocketClientManager.h>
#include <atomic>
#include <QMutex>
#include <IRCalibIntrinsic.h>
// #include "plcautocalibin.h"
#include <CPlcRelative.h>
#include <mescom.h>

struct AutoCfgInfo {
    QString dataPath = "E:/data";
    int offsetX = 0;
    int offsetY = 19;
};

struct WorkPosition {
    int distance;
    int direction;
};

class AutoCalibInDe1010 : public BaseProduce
{
    Q_OBJECT
public:
    AutoCalibInDe1010();

private:
    void workInit();
    void workMoveRaise();
    void workCheckVideo();
    void workMoveCheckAxis();
    void workCheckAxisCalculate();
    bool workCapture(int index);
    void workCalculateParam();
    void workHandleDown();
    void workMoveOrigin();
    void reportResult();
    void workMoveCapture();

    void recvFrame(char *pData, int iLen);

    void getParameters();
    bool getCurrentSN();
    bool dutPing();
    bool dutSwitchPattle(int mode = 0); // 0白热
    void writeLog(QString log, int status = 0); // 0 normal, 1 pass, 2 fail

    void mesClear();
    void mesAddData(int item, QString value, QString rst, QString err = "");
    void mesUpload();

    bool checkLimit();
    unsigned short getPos(int distance, int direction, QString &posDesp);

    void crossAxisInit(int diameter = 60);
    void crossAxisDiffInit(int diameter = 60);

public:
    void getSn();
public slots:
    void slotsStoped();
    void slotConnectBtnClicked();
    void slotStopWorkBtnClicked();
    void slotVideoSlot(QByteArray frameData, FrameInfo_S frameInfo);

private:
    unsigned char* m_frameData = nullptr;
    unsigned char* m_frameY = nullptr;
    unsigned char* m_rgbData = nullptr;

    float m_offsetAxisMaximum = 26.48f;
    float m_rmsMaximum = 0.72f;

    QPoint mOffsetAxis;
    float m_offsetValue = 0;
    WebSocketClientManager * m_videoClient = nullptr;
    std::atomic_int m_frameCount;
    QMutex m_mutex;
    std::vector<std::string> m_imagePathList;
    QList<WorkPosition> m_workList;
    //PlcAutoCalibin* m_plc = nullptr;
    IntrinsicParam m_calibParam;
    CPlcRelative* plcRelative = nullptr;
    int m_errorCode = 0;
    int m_ageErrorCode = 0;

    QByteArray mByteData;

    QPoint mCenterPoint;

    MesCom mMesObj;
    QString mLogPath;
    QString mMainLogPath;
    QString mImageSavePath;
    QString mBoxIP;
    QString mPlcReadSerial;
    QString mPlcWriteSerial;

    QString m_productName;

    bool mPlcSupported = true;

    QString mErrString = "";
    std::chrono::system_clock::time_point mStartPoint;
    bool mBoolIgnoreUpload = false;
    bool mBoolFixtureHasRaised = false;

    QPoint m_fixEnvOffset;

private:
    bool plcInit();
};

#endif // AUTOCALIBINDE1010_H

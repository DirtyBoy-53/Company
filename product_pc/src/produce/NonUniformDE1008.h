#ifndef NonUniform_H
#define NonUniform_H

#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include <QElapsedTimer>
#include <QDateTime>
#include <CPlcRelative.h>
#include <CTecRelative.h>
#include <WebSocketClientManager.h>
#include <CPingTest.h>
#include <CDutRelative.h>
#include <XyScanDialog.h>
#include <QMessageBox>
#include <QElapsedTimer>

extern int gErrorCode[MAXDUT];
extern bool gTestStatus[MAXDUT];
extern QString gErrorString[MAXDUT];
extern int gTestSteps;
class NonUniform: public BaseProduce
{
    Q_OBJECT
public:
    NonUniform();
    void connect();

private:
    // Sys initiliaze...
    void getParameters();
    bool dutPing(QString local, QString ip);
    void initSlotUI(int slot = 0);

    void addHvProcess();
    void addKProcess();
    void addBpProcess();
    void addSffcProcess();
    void addPitsProcess();

    int mStateSequence = 1;
    int mConnStatus = -999;
    int mDutStepStatus[MAXDUT] = {-999};

    std::chrono::system_clock::time_point mStartTimePoint;
private:
    // 测试同步，只有四个都OK才能结束状态机
    void clearTestFlag();
    void waitTestOver();
    void plcRstToUi(QString log, int status = 0);
    void setAllErrString(int errCode, QString errInfo);

signals:
    void sigTestStart(QStringList sn);
    void sigTestOver();

    // hv横竖纹测试
    void sigHvTest(int step);
    void sigStdTest(int step);
    void sigTimeStdTest(int step);
    void sigTimeStdRaiseTemp();
    void sigRestart();

    // 校K
    void sigKPreCal();
    void sigKCal20();
    void sigKCal60();
    void sigKCheck20();
    void sigKCheck60();

    // 校坏点
    void sigBPPreCal();
    void sigBpCal20();
    void sigBpCal60();
    void sigBpCheck20();
    void sigBpCheck60();

    // SFFC
    void sigSffcPreCal();
    void sigSffcRaiseTemp();
    void sigSffcCal();
    void sigSffcCheck();

    // Pits
    void sigPitsPreCal();
    void sigPitsCal(int step);
    void sigPitsAfterCal();
    void sigPitsCheck();

private:
    void start();
    void stop();

    // k
    void testKPreCal();
    void testBlTest();
    void testBhTest();
    void testBlCheck();
    void testBhCheck();

    // bp
    void testBpPreCal();
    void testBpCal20();
    void testBpCal60();
    void testBpCheck20();
    void testBpCheck60();

    // sffc
    void testSffcPreCal();
    void testSffcRaiseTemp();
    void testSffcCal();
    void testSffcCheck();

    // pits
    void testPitsPreCal();
    void testPitsCal(int step);
    void testPitsAfterCal();
    void testPitsCheck();

    bool plcInit();
    bool plcMoveReset();
    bool plcMoveStop();// not support now
    bool plcMoveToZero();
    bool plcMoveTo20();
    bool plcMoveTo60();
    bool PlcMovePlain();
    bool PlcMoveRaise();

    bool plcMoveTo20T();
    bool plcMoveTo60T();
    bool PlcFixtureUp();
    bool PlcFixtureDown();
    bool plcSetWorking(bool status);
    bool plcCheckCoverStatus();

    bool tecInit();
    bool tecStart();
    bool tecStop();
    bool tecQuery40();

    void testTempratureOk();
    void testRestart();
    void testHvStrips();
    void testStd(int step);
    void testTimeStd(int step);
    void testTimeStdRaiseTemp();
    //


public slots:
    void slotsStoped();
    void slotPitsFrame(int frame_cnt, int slotid);
    void slotPitsCount(int pits_cnt, QString list, int slotid);
    void slotSffcFocusTemp(float temp, int slotid);

private:
    int mIntFps;//帧率
    int mPitsMaxValue = 80;
    int mFocusTempLimit = 30;
    BpFileGetMethod mIntBpFileWay;

    QString mLogPath;
    QString mBPFilePath;
    QString mCurrentSN;

    QDateTime mStartTime;
    float mPressureValue;
    float mLeakageValue;
    QElapsedTimer mTimer;

    int mIntErrCode;
    bool mBoolLastResult = false;
    QString mErrString = "";
    bool mBoolTecSupported = false;
    bool mBoolPlcSupported = false;
    bool mBoolSffcSupport = false;
    bool mBoolPitsSupport = false;
    bool mBoolHvSupport = false;
    bool mBoolKSupport = false;
    bool mBoolBpSupport = false;
    bool mBoolPitsSaveAllImage = false;
    QString mTecSerial = "";
    QString mPlcReadSerial = "";
    QString mPlcWriteSerial = "";
    int mTecTargetTemp = 50;

    QStringList mAddrList;

    CTecRelative* mTecDevice;
    CPlcRelative* mPlcDevice;
    CDutRelative* mDutPtrArray[MAXDUT] = {nullptr};
    QThread* m_thread[MAXDUT];
    XyScanDialog* mScanDlg;
    WebSocketClientManager* mVideoSocket[MAXDUT] = {nullptr};

    QStringList mSnFromManual;

    bool mBoolRecordPicData = false;
    int mIntPicCount[MAXDUT];
    int mFTRecordTO = 30; // 分钟

    bool m_pclRaised = true;
    bool m_fixtureDown = false;
private:

public:
public slots:
    void slotConnectBtnClicked();
    void slotStartWorkBtnClicked();
    void slotDisConnectBtnClicked();
    void slotStopWorkBtnClicked();
    void slotVideoSlot1(QByteArray frameData, FrameInfo_S frameInfo);
    void slotVideoSlot2(QByteArray frameData, FrameInfo_S frameInfo);
    void slotVideoSlot3(QByteArray frameData, FrameInfo_S frameInfo);
    void slotVideoSlot4(QByteArray frameData, FrameInfo_S frameInfo);
    void videoDistribute(int id, QByteArray frameData, FrameInfo_S frameInfo);
    void slotPlayVideo(int slot);
    void slotStopVideo(int slot);

    void slotRecordFTData(int timeoutS);
    void slotStopRecordFTData();
    void recordDataTimeout(int slot, int timeoutS);
};
#endif // NonUniform_H

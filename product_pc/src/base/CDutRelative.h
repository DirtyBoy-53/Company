#ifndef CDUTRELATIVE_H
#define CDUTRELATIVE_H

#include <QObject>
#include <QFile>
#include <QElapsedTimer>
#include <asiccontrol.h>
#include <WebSocketClientManager.h>
#include <CPingTest.h>
#include "findbpy8.h"
#include "findBPY16.h"
#include <baseproduce.h>
#include <mescom.h>
#include <QQueue>
#include <imagecheck.h>

#define MAXDUT 4

#define LENGTH_50 50
#define LENGTH_100 100
#define LENGTH_128 128
#define LENGTH_1500 1500

#define REGISTER_PAGE_3 3
#define REGISTER_ADDR_9 9
#define REGISTER_ADDR_32 32
#define REGISTER_ADDR_33 33
#define REGISTER_WRITE_6F0 0x6F0
#define REGISTER_WRITE_6F1 0x6F1
#define REGISTER_READ_6F2 0x6F2
typedef enum {
    BpFileGet_Mes = 0,
    BpFileGet_Local,
    BpFileGet_None
} BpFileGetMethod;

typedef enum {
    CheckProc_PitsCal = 1,
    CheckProc_PitsCheck = 2,
    CheckProc_HvStripeCheck = 3,
    CheckProc_StdCheck = 4,
    CheckProc_TimeStdCheck = 5
} CheckProcProcess;

class CDutRelative : public QObject
{
    Q_OBJECT
public:
    explicit CDutRelative(QObject *parent = nullptr);
    ~CDutRelative();
    void setIpAddr(int slotId, QString ip);
    void setLogPath(QString path);
    void setTransmitFps(int fps);
    void setBPFilePath(QString path);
    void setBpGetWay(BpFileGetMethod way = BpFileGet_Local);
    void setProduce(BaseProduce* duce);
    void setPitsTest(bool pitsupprted = false, int frameCount = 1500, int maxFrame = 30, int maxValue = 80);
    void setSnRules(SnGetMethod way, int len, QString prefix = "");
    void setMesEnable(bool mesEnable = false);
    void setFocusTempLimit(int focus = 30);
    void setPitsSaveAllImage(bool saveImage = false);
    void setProjectName(QString proj_name);

 signals:
    void videoUpdate(const int handle, QByteArray frameData, qint64 timeT);

private:
    QString mArtOsynIp = "192.168.1.10";
    int mSlotId = 1; // 1, 2, 3, 4
    int mTempRaiseTOS = 420 * 5;
    int mTempRaiseDiff = 3;
    int mKCalBPMaxLimit = 350;
    int mY16CalBPMaxLimit = 30;
    bool mVideoConneted = false;

    QString m_projName = "";

    bool mBoolIsTest = false;
    bool mBoolTestResult = false;

    QString mErrString;
    int mIntPitsCalStep = 0; // 21,22,23,61,62,63
    int mIntPitsProc = 0; // 0, nothing 1/2,cal 3,check
    int mIntFrameCount = 0;
    //
    int mPitsCalImageCount = 1500;
    int mPitsMaxFrame = 30;
    int mPitsMaxValue = 80;

    int m_hvCheckImgCount = LENGTH_50;
    int mCheckProcImageCount = LENGTH_1500;
    int m_timeStdCheckImgCount = LENGTH_50;
    int m_spaceStdCheckImgCount = LENGTH_50;

    // std::mutex mMutexFrameData;
    QMutex mMutexFrameData;
    QByteArray mCrtFrameData;
    BaseProduce* mProduce = nullptr;

    unsigned char m_szY[WIDTH * HEIGHT] = {0};
    unsigned short m_szY16[WIDTH * HEIGHT] = {0};
    unsigned short m_szK[WIDTH * HEIGHT] = {0};

    QString mLogPath;
    QString mTmpLogPath;
    QString mBPFilePath;

    QString mTestStartTime;
    QString mTestEndTime;
    float mTestCostS = .0f;

    QString mAsicSN;
    QString mAsicDetectorSN;
    QString mAsicVersion;
    QString mArtOsVersion;

    QString mBdList;

    float mFocusTempFirst = .0f;
    float mKCheckMaxValue20 = .0f;
    float mKCheckMinValue20 = .0f;
    float mKCheckMaxValue60 = .0f;
    float mKCheckMinValue60 = .0f;
    int mK60Ad1 = -1;
    int mK20Ad2 = -1;
    std::pair<float, float> m_tempFocus_20_60;
    float mKRespRate = .0f;
    float mFocusTempAdGet = .0f;
    QString mBpAllList = "";
    QString mBpCheckList20 = "";
    QString mBpCheckList60 = "";
    float mSffcFocusTempLast = .0f;
    int mSffcDiff = -1;
    float mSffcNoise = .0f;
    QString mPitsBplist20_1 = "";
    QString mPitsBplist20_2 = "";
    QString mPitsBplist20_3 = "";
    QString mPitsBplist60_1 = "";
    QString mPitsBplist60_2 = "";
    QString mPitsBplist60_3 = "";
    // 横竖纹检测
    float m_hvValueStatics  = .0f;
    float m_hvValueExpectMax = .0f;
    float m_hvValueExpectMin = .0f;
    float m_hvStripsValue = .0f;
    // STD
    std::tuple<float, float> m_stdSpaceStep1;
    std::tuple<float, float> m_stdSpaceStep2;
    float m_stdSpace = .0f;
    float m_stdSpaceHigh = .0f;
    int m_spaceStdStep = 0;
    // Time STD
    float m_timeStdValue = .0f;
    float m_timeStdValue2 = .0f;
    int m_timeStdStep = 0;
    unsigned short *m_timeStdImageBuffer = nullptr;

    //
    int m_dataRaselHigh = 0;
    int m_dataRaselLow = 0;
    int m_dataRasel = 0;
    int m_dataHssd = 0;

    double m_stdGausKernel[GAUSSIAN_SIZE * GAUSSIAN_SIZE];

    int mTransmitFps = 30;
    int mImageSequence = 0;
    int mFocusTempLimit = 30;
    BpFileGetMethod mBpFileGetMethod;

    std::chrono::system_clock::time_point mStartPoint;

    MesCom mMesObj;
    bool mBoolPitsSupported = false;
    bool mIsOfflineMode = false;
    bool mIsOnlineMode = false;
    bool mBoolIngoreMes = false;
    bool mPitsSaveAllImage = false;

    QString mTestMode = "";
    SnGetMethod mSnGetMethod;
    int mSnLength;
    QString mSnPrefix;
    int mImageCount = 0;
    QQueue<QByteArray> mImageQue;

    bool mBoolCalKOk = true;
    bool mBoolCalSffcOk = true;

    QString m_deviceName = "";

    std::thread *mImgJudgeProc;
    MesCheckItem m_mesCheckPackage[MESMAXCHECKNUM];
    MesCheckItem m_mesCheckPackage2[MESMAXCHECKNUM];

private:
    void clearErrString();
    void setErrString(int errCode, QString log);
    bool hasError();
    bool execProc(QString item, QStringList list, std::function<bool(void)> func, int delay = 0);
    void writeLog(QString log, int status = 0);
    void writeTable(QStringList list);
    void updateTable(QString data, int col);
    void changeTableStatus(int status);
    void showLabel(QString rst, int status = 0);

    bool fileExists(QString path);
    void mySleep(int ms);
    void mySleepSelf(int ms);

    void mesInforClear();
    void mesAddInfor(int seq, QString value, QString rst = "NG", QString err="");
    void mesUpload();
    void saveCsvReport();
    QString getFormatDateTime();

    void imgJudgeProc();
    bool videoJudgeNormally();

signals:
    void sigDutConnectStatus(int status);// 999 connect ok, -999 connect fail, -1 default
    void sigFrameNumber(int, int slot = 0);
    void sigPitsNumber(int, QString, int slot = 0);
    void sigSffcFocusTemp(float temp, int slotid = 0);
    void sigPlayVideo(int slot);
    void sigStopVideo(int slot);
public slots:
    void slotConnected();
    void slotRecvVideoFrame(const int handle, QByteArray frameData, qint64 timeT);

    void slotStartTest(QStringList sn);
    void slotOverTest();

    void slotKPreCal();
        bool dutPing();
        bool dutReadyForTest(QString &err); // 对出图条件进行设置
            bool dutGetAsicSn();
            bool dutGetDeviceName();
            bool dutGetArtOsVersion();
            bool dutGetAsicVersion();
            bool dutOpenShutter();
            bool dutSetFps();
            // 1. UYUV... 3. Y16
            bool dutSetVideoFormat(int fmt);
            // 0. 白热
            bool dutSetPallte(int mode);//白热
            bool dutCloseShutter();
        bool dutPlayVideo();

    void slotKCal20();
        bool dutCloseAdaptive();
        bool dutSceneNUC();
        bool dutBLCompensation();
        bool dutGetAdValue(int &ad);
    void slotKCal60();
        bool dutBHCompensation();
        bool dutKCoffCalculate();
        bool dutKCoffSave();
        bool dutSendGetKCmds(int addr);
        bool dutSaveKFile(int addr, QString fileName);
        bool dutGetK(QByteArray k1, int page);
        bool dutOpenAdaptive();
    void slotKCheck20();
        bool dutKCheck(int kStep = 20);
    void slotKCheck60();

    void slotBpPreCal();
        bool dutClearBPList();
        bool dutGetBPListFromFile(QString sPath, QString detectorSN, QString & list);
        bool dutSetBPList(const int mode, QString list);
        bool dutBPKCal();
    void slotBpCal20();
        bool dutShutterComp();
        bool dutBPY16Cal();
    void slotBpCal60();
        // bool dutBPY16Cal();
        bool dutBPSave();
        bool dutGetBpCount(int &count);
        bool dutGetBPListFromDUT(QString &list);
    void slotBpCheck20();
    bool dutBpCheck(int kStep = 20);
    void slotBpCheck60();

    bool getImageData(QByteArray& imgData);

    void slotSffcPreCal();
        // bool dutCloseAdaptive();
        // bool dutSceneNUC();
        bool dutSceneCompensation();
    void slotSffcRaiseTemp();
        bool dutSffcEnable(float diff);
            bool dutGetFocusTemp(float &temp);
    void slotSffcCal();
        bool dutSffcCollect();
        bool dutSffcSave();
    void slotSffcCheck();
        bool dutSffcCheck();
    void slotPitsPreCal();
        bool dutMRTDAlgorithm(int id, int value);
    void slotPitsCal(int step);
        // 仅实用UYUV图像格式，暂不支持Y16数据传输。
        void getValueY(QByteArray data, unsigned char * &pretY);
        void getValueY_short(QByteArray data, unsigned short * &pretY);
        void dutPitsGetBp(QByteArray data);
        int dutPitsBpStatics(QString &list);
    void slotPitsAfterCal();
    void slotPitsCheck();
        void dutPitsCheckBp(QByteArray data);

    bool dutSaveImage(QString name, QByteArray data = QByteArray());
    bool dutSaveTimeStdBuffer();

    void slotHvStripsCheck(int step);
        bool dutCloseAllAlg(bool bClose);
        void dutHvStripesGetOnce(QByteArray data);
        double dutHvStripsGetResult();
    void slotStd(int step);
        bool dutStdGetOnce(QByteArray data);
        void dutStdGetResult();
    void slotTimeStd(int step);
    void slotTimeStdRaiseTemp();
    void slotRestart();
    bool dutSendCustom(int addr, int value);
    bool dutReadCustom(int addr, int & value);
    bool dutGetRegister_i2c(int page, int addr, int &data);
    bool dutGetRegister_com(QString &ret);
    bool recordRegisterData_i2c();

    void slotSaveTemplate();
};

#endif // CDUTRELATIVE_H

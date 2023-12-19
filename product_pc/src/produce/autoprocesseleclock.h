#ifndef AUTOPROCESSCHECK1_H
#define AUTOPROCESSCHECK1_H

#include <QObject>
#include "eleclockserialmanager.h"
#include "ioserialmanager.h"
#include "baseproduce.h"
#include "configinfoelec.h"
#include "selectlockserial.h"
#include "screwselectserial.h"
#include "civ3socket.h"

struct AutoWorkInfo {
    int workIndex = 0;
    int workState = 0;
    int result;
    int errorCode;
    float circle;
    float maxMnm;
    int workTime;
    int reworkTimes = 1;
    int direction;
};

struct AutoPosState {
    int x;
    int y;
    bool isCrc;
    bool enable = false;
    int offset;
    QString imgPath;
    QString name;
};

struct AutoWorkResult {
    QString name;
    int index;
    QString workPos;
    float workMnm;
    float workCircle;
    int workTime;
    int reworkTimes;
    int errorCode;
    int result;
};

class AutoProcessElecLock : public BaseProduce
{
    Q_OBJECT
public:
    AutoProcessElecLock();
    bool initConnect();
    void initScrewCfg(const int index);
    void getSn();

    void checkNext();
    void checkWorkMode();
    static void realDataCb(ElecLockWorkInfo info, void* param);
    static void realPosInfoCb(ElecLockPosInfo info, void* param);
    static void ScrewSelectCb(ScrewSelectInfo info, void* param);
    void autoWork();

    void waitFixtureStatus(int status);
    void pullProduct();
    void pullCoverages();
    void moveToOriginal();
    void moveToCCD();
    void iv3Test();

public slots:
    void slotConnectBtnClicked();
    void slotStopWorkBtnClicked();
    void slotStartWorkBtnClicked();

    void slotCheckIoState(const int state);
    void slotsStoped();
    void slotScrewSeletState(const int state, bool flag);
    void initScrewSelect();

signals:
    void signalInitConnect();
    void signalInitCfg(const int index);
    void signalScrewPosClamped(const int state, bool flag);
    void signalOperate(int code, bool status);

private:
    void setScrewDeviceEnable(const bool enable);
    void checkWorkResult();
    void praseWorkInfo(ElecLockWorkInfo info);
    void prasePosInfo(ElecLockPosInfo info);
    void addLog(QString log, const int result = 0);
    void mesUpload();
    void overTest();
    void getSensorSn();
    void getBackSN();
    void getShutterCode();
    void initStateMachine();
    void updateMesPackage(AutoWorkResult info, bool result);

    int m_productIndex = 0;

	QString checkErrorCode(AutoWorkResult workInfo);
    void updateTable(AutoWorkResult info, bool result);

    ElecLockSerialManager* m_serial;
    IoSerialManager* m_ioSerial;
    ScrewSelectSerial* m_screwSelectSerial;
    QList<MesCheckItem> m_mesPackage;

    ElecLockWorkInfo m_autoWorkInfo;
    AutoPosState m_autoPosInfo;
    AutoWorkResult m_result;

    int m_index = 0;   // 当前第几颗螺丝
    int m_reWorkTimes = 0;  // 返工次数
    bool m_isRunning = false;
    IoSerialState m_switchStatus = IO_SERIAL_STATE_INVALID;
    bool m_produceStart = false;   // 产品是否在位，已经正常开始工作标识
    bool m_isAutoWorking = false;
    bool m_isScrewWorking = false;

    SystemInfo m_sysInfo;
    QList<LockPosInfo> m_currenWorkList;
    QList<AutoWorkIndex> m_workList;
    int m_screwCfgIndex = 0;
    int m_screwIndex = 0; // 螺丝配置编号
    int m_mnmMax;
    int m_mnmMin;

    ScrewSelectInfo m_screwSelectInfo;

    bool m_iv3Support = false;
    QString m_iv3ProjectCode = "";
    QString m_iv3Address = "192.168.10.16";
};


#endif // AUTOPROCESSCHECK1_H

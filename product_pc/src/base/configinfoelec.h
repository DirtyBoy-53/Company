#ifndef CONFIGINFOELEC_H
#define CONFIGINFOELEC_H


#include <QMutex>
#include <QObject>
#include <QRect>

struct SystemInfo {
    QString readCom = "COM10";
    QString writeCom = "COM12";
    QString ioCom = "COM5";
    QString selectCom = "COM9";
    bool ioSwitch;
    bool selectSwitch;
};

struct LockPosInfo {
    QString name;
    QString cfgIndex;
    QString x;
    QString y;
    QString limit;
    QString imgPath;
    QString screwModel;
};

struct LockCfgInfo {
    QString mnm;//N.m
    QString mnmLimit;//N.m
    QString step1Circle;
    QString step2Circle;
    QString step3Circle;
    QString step1Speed;
    QString step2Speed;
    QString step3Speed;
    QString circleFugao;
    QString circleHuaya;
};

struct CustomWorkList {
    QString index;
    QString txt;
    QString path;
    QString tips;
};

struct AutoWorkIndex {
    int type;  // 0 螺丝， 1 提示
    int index; // 螺丝、提示的序号
};

class ConfigInfoElecLock
{
public:
    explicit ConfigInfoElecLock();
    ~ConfigInfoElecLock();
    static ConfigInfoElecLock* getInstance();

    void changeProduce(const QString& produce);

    void SetImagePath(const QString path);
    void GetImagePath(QString& path);

    void SetSysInfo(const SystemInfo& systemInfo);
    void GetSysInfo(SystemInfo& systemInfo);

    void SetLockInfo(const QList<LockPosInfo> list);
    void GetLockInfo(QList<LockPosInfo>& list);

    void SetLockCfgInfo(const int index, const LockCfgInfo list);
    void GetLockCfgInfo(const int index, LockCfgInfo& list);

    void SetTipsList(const QList<CustomWorkList> list);
    void GetTipsList(QList<CustomWorkList>& list);

    void SetWorkList(const QList<AutoWorkIndex> list);
    void GetWorkList(QList<AutoWorkIndex>& list);

private:
    void ReadAllConfig();
    QMutex m_mutex;
    QString m_configPath;
    QString m_imgPath;
    SystemInfo m_sysInfo;
    QList<LockPosInfo> m_posList;
    QList<CustomWorkList> m_tipsList;
    QString m_produceName;
    QList<LockCfgInfo> m_lockCfgList;
    QList<AutoWorkIndex> m_worklist;
};

#endif // CONFIGINFOELEC_H

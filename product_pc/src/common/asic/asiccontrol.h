#ifndef ASICCONTROL_H
#define ASICCONTROL_H

#include <QObject>
#include "socketmanager.h"
#include <comdll.h>
#include <tcpclient.h>

struct UpdateInfo {
    QString softwareName;
    QString version;
    QString type;
    qint64 md5;
};

struct AsicAlgorithmInfo {
    int castUp = 0;
    int castDown = 0;
    int noiseGrade0 = 0;
    int noiseGrade1 = 0;
    int ideDetails = 0;
};

struct AsicSensorInfo {
    int bigRingTempMax = 0;
    int nucTempMax = 0;
    int localTemp = 0;
    int autoCompTime = 0;
    int rasel = 0;
    int hssd = 0;
    int nucTime = 0;
};

struct NvsInfo {
    float  personMin;
    float  personMax;
    float  vehicleMin;
    float  vehicleMax;
};

class Q_API_FUNCTION AsicControl : public QObject
{
    Q_OBJECT
public:
    AsicControl(SocketManager* socket);
    AsicControl(QString sIp);
    ~AsicControl();

public slots:
    void Reset();
    void Save();
    void StartWindowHot(const int mode = 0, const int max = 7);
    bool GetSffcValue();
    bool SaveSffcValue();
    bool ChangeWorkMode(const bool isSleep);

    /// 算法id分类：
    /// \param id
    /// 0 - 去坏点     1 - 边缘增强        2 - 对比度增强
    /// 3 - 去锅盖     4 - 空域去噪        5 - 去竖纹
    /// 6 - 去横纹     7 - 3D去噪(Y16)     8 - 3D去噪(Y8)
    /// \param value
    /// 0 - 关闭算法    1 - 开启算法
    /// \return
    bool ChangeMrtdSwitch(const int id, const int value);
    void GetWindowTemp();
    bool BhComp();
    bool BlComp();
    bool CaluteK();
    bool DeleteBadPoint();
    bool LoadDefaultK();
    bool LoadK();
    bool SaveBadPoint();
    bool SaveK();
    bool SceneComp();
    bool ShutterComp();
    bool GetAd(int &ad);
    void GetCurrentAd(const int x, const int y);
    void AddPoint(int x, int y);
    void AddBadPoint(int x, int y);

    bool GetArtOsVersion(QString &version);
    bool GetAsicVersion(QString &version);
    bool GetDeviceName(QString &deviceName);
    bool SetKBadPoint(int max);
    bool SetY16BadPoint(int max);
    void ManulNuc();
    void SffcEnable(const int enable);
    bool SffcLoad();
    void AutoBadPoint();
    bool SetPattle(const int pattle);
    bool AutoAdapteComp(const int mode);
    bool GetFocusTemp(float &ftemp);
    void SetAlgorithm(AsicAlgorithmInfo asicInfo);
    bool SceneNuc();
    bool setTransmitFps(int fps);
    void SetEnhanceGrade(const int value);
    bool SetSn(QString sn);
    QString GetSn();
    void DeleteCurrentBadPoint();
    bool changeSensorFormat(const int mode);// 1. UYUV 3. Y16
    void ChangeSensorCfg(const int type);

    void SetBigRingTemp(const float value);
    void SetNucTempMax(const float value);
    void SetNucValue(const int value);
    void SetBottomTempMax(const float value);
    void SetRasel(const int value);
    void SetHssd(const int value);
    void SetAutoCompTime(const int value);
    void SetDebugMode(const int mode);
    bool SetBadPointList(const int mode, QString list);
    bool GetBadPointsCount(int &count);
    bool GetBadPointsList(QString &list);
    void SetIspAlgValue(const int id, QString data);
    void GetIspAlgValue(const int id, const int len);

    void ChangeNvsInfo(NvsInfo info);

    QString GetParamIn();
    bool SetParamIn(float fx, float fy, float cx, float cy, float rms, float coff[]);

    // 0x04 K, 0x05 Sffc, 0x06 b, 0x02/0x03 高低温本地
    bool SendCustom(int addr, int value);
    bool ReadCustom(int addr, int & value);
    bool SendCustomUart(QString value);
    bool ClearAgeErrorCode();
    bool GetAgeErrorCode(int& errorCode);
    bool ClearSensorErrorCode();
    bool GetSensorErrorCode(int& errorCode);

private:
    bool resultConfirm(int msgId, QString sOut);
private:
    AsicAlgorithmInfo m_algorithmPage;
    AsicSensorInfo m_sensorPage;
    SocketManager* m_socket;
    TcpClient *m_tcpClient = nullptr;
    QString m_sOut;
};

#endif // ASICCONTROL_H

#ifndef CONFIGINFO_H
#define CONFIGINFO_H


#include <QMutex>
#include <QObject>
#include <QRect>
#include "inisettings.h"
#include "common.h"

struct CSenceInfo{//场景标签信息
    QString _Time; // 时间戳
    float _Temp;  // 温度
    float _Wet;   // 湿度
    QString _TimeQuantum;  //时间段
    QString _Weather; // 天气
    QString _Scene;   // 场景
    QString _Project[MAX_PANLE_NUM];   //项目
};

struct CPreSenceInfo{
    QStringList _TimeQuantum;//时间段
    QStringList _Weather;//天气
    QStringList _Scene;//场景
    QStringList _Project;//项目
};

struct StructOtherInfo
{
    QString m_lidarIP{"192.168.2.201"};
    QString m_CameraIP{"192.168.1.10"};
};

class ConfigInfo
{
public:
    explicit ConfigInfo();
    ~ConfigInfo();
    static ConfigInfo* getInstance();

    void SetSenceInfo(const CSenceInfo& info);
    void GetSenceInfo(CSenceInfo& info);
    void GetSavePath(QString& path);
    void SetSavePath(QString& path);

    void Init(QString &&filename="preConfig.ini");

    void ReadAllConfig();

    QStringList m_listWeather;
    QStringList m_listTimeQuantum;
    QStringList m_listProject;
    QStringList m_listSence;
    QString m_saveDir;

    QStringList m_listIIC_JsonFile;
    QStringList m_listIIC_JsonFile_light;
    QStringList m_listVideoType;
    QStringList m_listColorType;
    QStringList m_listItemType;
    QStringList m_listDisType;

    StructOtherInfo m_otherInfo;

private:
    QMutex m_mutex;

    CSenceInfo m_senceInfo;

    QString m_configPath;
    QString m_preInfo;//前置信息

    IniSettings m_iniSettings_pre;//前置信息
    // IniSettings m_iniSettings_Info;//标签信息

    
};

#endif // CONFIGINFO_H

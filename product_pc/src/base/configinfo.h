#ifndef CONFIGINFO_H
#define CONFIGINFO_H

#include <QMutex>
#include <QObject>
#include <QRect>
#include <CPropertyXml.h>

#define MaxConfigNum 500

enum LoginAccount {
    LOGIN_NORMAL = 0, //一般用户
    LOGIN_ADMIN, //组长
    LOGIN_ROOT, //管理员
    LOGIN_SUPER,//程序员
    LOGIN_OWN,//保留
};

struct XmlConfigInfo {
    QString group;
    QString name;
    QString value;
    QString type;
    int pro; //0 操作员 1 组长 2 管理员 3 all
    QString enumNames;
};

struct SysInfo {
    QString ip = "192.168.1.10";
    QString lvdsIp = "192.168.1.11";
    QString readCom = "COM10";
    QString writeCom = "COM12";
    QString mcuCom = "COM2";
    QString printerModelPath;
    QString hotElecLimit;
    QString logPath;
    QString curProduce;
    QString mesIp;
    QString mesPort;
    int mode; //0 生产模式 1 点检模式 2 调试模式 3 离线模式
    int iFirst;//是否第一次
    QString picDataPath;
    QString sOffsetX;
    QString sOffsetY;
    QString ioCom = "COM3";
};

struct CacheInfo {
    QString orderId;//工单号
    QString sCodeId;//工序编码
    QString sSn;
    int unfinishedQuantity;
    int finishedQuantity;
    int Permissionmode;  //获得管理员权限
    QString basProductCode;
};

class ConfigInfo
{
public:
    explicit ConfigInfo();
    ~ConfigInfo();
    static ConfigInfo* getInstance();

    CacheInfo cacheInfo();
    void setCacheInfo(CacheInfo & info);
    SysInfo sysInfo();
    void setSysInfo(SysInfo & info);

    QString configPath() const;
    void ReadAllConfig(int iInit = 0);

    bool getValueString(QString group, QString key, QString &value);
    bool getValueBoolean(QString group, QString key, bool &value);
    bool getValueInt32(QString group, QString key, int &value);
    QString getValueString(QString key);

    //添加配置
    void addConfig(XmlConfigInfo cInfo, bool bSave = true);
    bool isOperator()
    {
        return (LOGIN_NORMAL == m_cacheInfo.Permissionmode);
    }
    bool isEngineer()
    {
        return (LOGIN_ADMIN == m_cacheInfo.Permissionmode);
    }
private:

    QMutex m_mutex;
    QString m_configPath;
    SysInfo m_sysInfo;
    CacheInfo m_cacheInfo;

    CPropertyXml m_xml;
};

#endif // CONFIGINFO_H

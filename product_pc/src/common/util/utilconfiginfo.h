#ifndef UTILCONFIGINFO_H
#define UTILCONFIGINFO_H

#include <QMutex>
#include <QObject>
#include <comdll.h>

struct WorkInfo {
    bool isFirst = true;
    int pluginIndex = 0;
};

struct LoginInfo {
    QString workerPasswd = "123";
    QString adminPasswd = "666666";
};

struct CacheInfo {
    QString orderId;//工单号
    QString sCodeId;//工序编码
    QString sSn;
    int unfinishedQuantity;
    int finishedQuantity;
};

class Q_API_FUNCTION UtilConfigInfo
{
public:
    explicit UtilConfigInfo();
    ~UtilConfigInfo();
    static UtilConfigInfo* getInstance();

    void SetWorkInfo(const WorkInfo& workInfo);
    void GetWorkInfo(WorkInfo& workInfo);

    void SetLoginInfo(const LoginInfo& info);
    void GetLoginInfo(LoginInfo& info);

    CacheInfo cacheInfo() ;
    void setCacheInfo(CacheInfo &newCacheInfo);

    QString getKeyValue(QString name);

    QStringList getAllChildKes(QString name);

private:
    void ReadAllConfig();
    QMutex m_mutex;
    QString m_configPath;
    LoginInfo m_loginInfo;
    WorkInfo m_workInfo;
    CacheInfo m_cacheInfo;
};

#endif // UTILCONFIGINFO_H

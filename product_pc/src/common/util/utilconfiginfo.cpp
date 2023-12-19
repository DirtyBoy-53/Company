#include "utilconfiginfo.h"
#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>

UtilConfigInfo::UtilConfigInfo()
{
    m_configPath = QCoreApplication::applicationDirPath() + "/config/sys.ini";
    ReadAllConfig();
}

UtilConfigInfo::~UtilConfigInfo()
{

}

UtilConfigInfo* UtilConfigInfo::getInstance()
{
    static UtilConfigInfo instance;
    return &instance;
}

void UtilConfigInfo::ReadAllConfig()
{
    QMutexLocker lock(&m_mutex);

    QSettings reader(m_configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));

    reader.beginGroup("WrokInfo");
    m_workInfo.isFirst = reader.value("isFirst", QVariant(true)).toBool();
    m_workInfo.pluginIndex = reader.value("pluginIndex", QVariant(0)).toInt();
    reader.endGroup();

    reader.beginGroup("LoginInfo");
    m_loginInfo.adminPasswd = reader.value("adminPasswd", QVariant("123")).toString();
    m_loginInfo.workerPasswd = reader.value("workerPasswd", QVariant("666666")).toString();
    reader.endGroup();
}

CacheInfo UtilConfigInfo::cacheInfo()
{
    CacheInfo info;
    {
        QMutexLocker lock(&m_mutex);
        info = m_cacheInfo;
    }
    return info;
}

void UtilConfigInfo::setCacheInfo(CacheInfo &newCacheInfo)
{
    QMutexLocker lock(&m_mutex);
    m_cacheInfo = newCacheInfo;
}

QString UtilConfigInfo::getKeyValue(QString name)
{
    QString sValue;
    QMutexLocker lock(&m_mutex);

    QSettings reader(m_configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));

    reader.beginGroup(m_cacheInfo.sCodeId);
    sValue = reader.value(name).toString();
    reader.endGroup();
    return sValue;
}

QStringList UtilConfigInfo::getAllChildKes(QString name)
{
    QStringList sValue;
    QMutexLocker lock(&m_mutex);

    QSettings reader(m_configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));

    reader.beginGroup(m_cacheInfo.sCodeId);
    sValue = reader.childKeys();
    reader.endGroup();
    return sValue;
}

void UtilConfigInfo::SetWorkInfo(const WorkInfo& info)
{
    QMutexLocker lock(&m_mutex);
    m_workInfo = info;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));

    writer.beginGroup("WrokInfo");
    writer.setValue("isFirst", m_workInfo.isFirst);
    writer.setValue("pluginIndex", m_workInfo.pluginIndex);
    writer.endGroup();
}

void UtilConfigInfo::GetWorkInfo(WorkInfo& info)
{
    QMutexLocker lock(&m_mutex);
    info = m_workInfo;
}

void UtilConfigInfo::SetLoginInfo(const LoginInfo& info)
{
    QMutexLocker lock(&m_mutex);
    m_loginInfo = info;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));

    writer.beginGroup("LoginInfo");
    writer.setValue("adminPasswd", m_loginInfo.adminPasswd);
    writer.setValue("workerPasswd", m_loginInfo.workerPasswd);
    writer.endGroup();
}

void UtilConfigInfo::GetLoginInfo(LoginInfo& info)
{
    QMutexLocker lock(&m_mutex);
    info = m_loginInfo;
}

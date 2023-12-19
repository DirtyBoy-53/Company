#include "configinfoelec.h"
#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>
#include <QDebug>
#include "configinfo.h"

ConfigInfoElecLock::ConfigInfoElecLock()
{
    m_produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    m_configPath = QCoreApplication::applicationDirPath() + QString("/config/pluginlock_%1.ini").arg(m_produceName);
    ReadAllConfig();
}

ConfigInfoElecLock::~ConfigInfoElecLock()
{

}

ConfigInfoElecLock* ConfigInfoElecLock::getInstance()
{
    static ConfigInfoElecLock instance;
    return &instance;
}

void ConfigInfoElecLock::changeProduce(const QString &produce)
{
    m_produceName = produce;
    m_configPath = QCoreApplication::applicationDirPath() + QString("/config/pluginlock_%1.ini").arg(produce);
    ReadAllConfig();
}

void ConfigInfoElecLock::ReadAllConfig()
{
    m_lockCfgList.clear();
    m_posList.clear();
    m_tipsList.clear();
    m_worklist.clear();

    QMutexLocker lock(&m_mutex);

    QSettings reader(m_configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));

    reader.beginGroup("SystemInfo");
    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();
    reader.endGroup();

    reader.beginGroup("SystemInfo");
    m_sysInfo.readCom = reader.value("readCom", QVariant("COM3")).toString();
    m_sysInfo.writeCom = reader.value("writeCom", QVariant("COM1")).toString();
    m_sysInfo.ioCom = reader.value("ioCom", QVariant("COM4")).toString();
    m_sysInfo.selectCom = reader.value("selectCom", QVariant("COM2")).toString();
    m_sysInfo.ioSwitch = reader.value("ioSwitch", QVariant(false)).toBool();
    m_sysInfo.selectSwitch = reader.value("selectSwitch", QVariant(false)).toBool();
    reader.endGroup();

    for(int i=0; i<10; i++) {
        reader.beginGroup(QString("LockCfgInfo_%1").arg(i));
        LockCfgInfo lockCfg;
        lockCfg.mnm = reader.value("mnm", QVariant("0")).toString();//N.m
        lockCfg.mnmLimit = reader.value("mnmLimit", QVariant("0")).toString();
        lockCfg.step1Circle = reader.value("step1Circle", QVariant("0.5")).toString();
        lockCfg.step1Speed = reader.value("step1Speed", QVariant("100")).toString();
        lockCfg.step2Circle = reader.value("step2Circle", QVariant("0")).toString();
        lockCfg.step2Speed = reader.value("step2Speed", QVariant("0")).toString();
        lockCfg.step3Circle = reader.value("step3Circle", QVariant("0")).toString();
        lockCfg.step3Speed = reader.value("step3Speed", QVariant("150")).toString();
        lockCfg.circleFugao = reader.value("circleFugao", QVariant("1.5")).toString();
        lockCfg.circleHuaya = reader.value("circleHuaya", QVariant("3")).toString();
        m_lockCfgList.append(lockCfg);
        reader.endGroup();
    }

    reader.beginGroup("LockPosInfo");
    QStringList keyList = reader.allKeys();
    for(int i=0; i<keyList.size(); i++) {
        QStringList list = reader.value(keyList[i], QVariant("0")).toString().split("~");

        if(list.size() < 6) break;
        LockPosInfo info;
        info.name = list[0];
        info.cfgIndex = list[1];
        info.x = list[2];
        info.y = list[3];
        info.limit = list[4];
        info.imgPath = list[5];
        if(list.size() == 7) {
            info.screwModel = list[6];
        } else {
            info.screwModel = "1";
        }
        m_posList << info;
    }
    reader.endGroup();

    reader.beginGroup("TipsWorkList");
    QStringList tipsList = reader.allKeys();
    for(int i=0; i<tipsList.size(); i++) {
        QStringList list = reader.value(tipsList[i], QVariant("")).toString().split("~");
        if(list.size() != 4) break;

        CustomWorkList info;
        info.index = list[0];
        info.txt = list[1];
        info.tips = list[2];
        info.path = list[3];
        m_tipsList << info;
    }
    reader.endGroup();

    reader.beginGroup("AutoWorkList");
    QStringList workList = reader.allKeys();

    for(int i=0; i<workList.size(); i++) {
        QString key = "step" + QString::number(i);
        QStringList list = reader.value(key, QVariant("")).toString().split("~");
        if(list.size() != 2) break;

        AutoWorkIndex info;
        info.type = list[0].toInt();
        info.index = list[1].toInt();
        m_worklist << info;
        qDebug() << info.type << " " << info.index;
    }
    reader.endGroup();
}

void ConfigInfoElecLock::SetImagePath(const QString path)
{
    QMutexLocker lock(&m_mutex);
    m_imgPath = path;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));

    writer.beginGroup("SystemInfo");
    writer.setValue("imgPath", m_imgPath);
    writer.endGroup();
}

void ConfigInfoElecLock::GetImagePath(QString& path)
{
    QMutexLocker lock(&m_mutex);
    path = m_imgPath;
}

void ConfigInfoElecLock::SetSysInfo(const SystemInfo& systemInfo)
{
    QMutexLocker lock(&m_mutex);
    m_sysInfo = systemInfo;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));
    writer.beginGroup("SystemInfo");
    writer.setValue("readCom", m_sysInfo.readCom);
    writer.setValue("writeCom", m_sysInfo.writeCom);
    writer.setValue("ioCom", m_sysInfo.ioCom);
    writer.setValue("selectCom", m_sysInfo.selectCom);
    writer.setValue("ioSwitch", m_sysInfo.ioSwitch);
    writer.setValue("selectSwitch", m_sysInfo.selectSwitch);
    writer.endGroup();
}

void ConfigInfoElecLock::GetSysInfo(SystemInfo& systemInfo)
{
    QMutexLocker lock(&m_mutex);
    systemInfo = m_sysInfo;
}

void ConfigInfoElecLock::SetLockInfo(const QList<LockPosInfo> list)
{
    QMutexLocker lock(&m_mutex);
    m_posList = list;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));
    writer.remove("LockPosInfo");
    writer.beginGroup("LockPosInfo");
    for(int i=0; i<list.size(); i++) {
        QString value = QString("%1~%2~%3~%4~%5~%6~%7")
                        .arg(list[i].name)
                        .arg(list[i].cfgIndex)
                        .arg(list[i].x)
                        .arg(list[i].y)
                        .arg(list[i].limit)
                        .arg(list[i].imgPath)
                        .arg(list[i].screwModel);
        writer.setValue(QString("pos%1").arg(i), value);
    }
    writer.endGroup();
}

void ConfigInfoElecLock::GetLockInfo(QList<LockPosInfo> &list)
{
    list = m_posList;
}

void ConfigInfoElecLock::SetLockCfgInfo(const int index, const LockCfgInfo info)
{
    QMutexLocker lock(&m_mutex);
    m_lockCfgList[index] = info;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));
    QString node = QString("LockCfgInfo_%1").arg(index);
    writer.remove(node);
    writer.beginGroup(node);
    writer.setValue("mnm", info.mnm);//N.m
    writer.setValue("mnmLimit", info.mnmLimit);
    writer.setValue("step1Circle", info.step1Circle);
    writer.setValue("step1Speed", info.step1Speed);
    writer.setValue("step2Circle", info.step2Circle);
    writer.setValue("step2Speed", info.step2Speed);
    writer.setValue("step3Circle", info.step3Circle);
    writer.setValue("step3Speed", info.step3Speed);
    writer.setValue("circleFugao", info.circleFugao);
    writer.setValue("circleHuaya", info.circleHuaya);
    writer.endGroup();
}

void ConfigInfoElecLock::GetLockCfgInfo(const int index, LockCfgInfo &info)
{
    info = m_lockCfgList[index];
}

void ConfigInfoElecLock::SetTipsList(const QList<CustomWorkList> list)
{
    QMutexLocker lock(&m_mutex);
    m_tipsList = list;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));

    writer.remove("TipsWorkList");
    writer.beginGroup("TipsWorkList");
    for(int i=0; i<list.size(); i++) {
        QString value = QString("%1~%2~%3~%4").arg(list[i].index).arg(list[i].txt).arg(list[i].tips).arg(list[i].path);
        writer.setValue(QString("tips%1").arg(i), value);
    }
    writer.endGroup();
}

void ConfigInfoElecLock::GetTipsList(QList<CustomWorkList> &list)
{
    QMutexLocker lock(&m_mutex);
    list = m_tipsList;
}

void ConfigInfoElecLock::SetWorkList(const QList<AutoWorkIndex> list)
{
    qDebug() << "SetWorkList " << m_worklist.size();
    QMutexLocker lock(&m_mutex);
    m_worklist = list;

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));

    writer.remove("AutoWorkList");
    writer.beginGroup("AutoWorkList");
    for(int i=0; i<list.size(); i++) {
        QString value = QString("%1~%2").arg(list[i].type).arg(list[i].index);
        writer.setValue(QString("step%1").arg(i), value);
    }
    writer.endGroup();
}

void ConfigInfoElecLock::GetWorkList(QList<AutoWorkIndex> &list)
{
    QMutexLocker lock(&m_mutex);
    list = m_worklist;
}

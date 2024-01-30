#include "configinfo.h"
#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include "memcache.h"
ConfigInfo::ConfigInfo(){}

ConfigInfo::~ConfigInfo(){}

ConfigInfo* ConfigInfo::getInstance()
{
    static ConfigInfo instance;
    return &instance;
}

void ConfigInfo::ReadAllConfig()
{
    QMutexLocker lock(&m_mutex);

    
    QFileInfo fileInfo(m_preInfo);
    if(fileInfo.isFile()){
        //标签前置信息
        m_listProject = m_iniSettings_pre.getValueList("PreInfo","Project");
        m_listSence = m_iniSettings_pre.getValueList("PreInfo","Sence");
        m_listTimeQuantum = m_iniSettings_pre.getValueList("PreInfo","TimeQuantum");
        m_listWeather = m_iniSettings_pre.getValueList("PreInfo","Weather");
        m_saveDir = m_iniSettings_pre.getValue("PreInfo","SaveDir","/media/xy/raid0");


        //标签信息
        m_senceInfo._Time=QDateTime::currentDateTime().toString("yyyy_MM_dd");
        m_senceInfo._Wet = m_iniSettings_pre.getValue("SenceInfo","wet","22.1").toFloat();
        m_senceInfo._Temp = m_iniSettings_pre.getValue("SenceInfo","temp","25.0").toFloat();
        m_senceInfo._Scene = m_iniSettings_pre.getValue("SenceInfo","scene","高架桥下");
        m_senceInfo._Weather = m_iniSettings_pre.getValue("SenceInfo","weather","晴天");
        m_senceInfo._TimeQuantum = m_iniSettings_pre.getValue("SenceInfo","timequantum","白天");
        for(auto i = 0;i < MAX_PANLE_NUM;i++){
            QString str = QString("Ch%1Type").arg(i+1);
            m_senceInfo._Project[i] = m_iniSettings_pre.getValue("SenceInfo",str,"XD01A");
        }

        m_listIIC_JsonFile = m_iniSettings_pre.getValueList("CtrlInfo","IIC_Camera_hw");
        m_listIIC_JsonFile_light = m_iniSettings_pre.getValueList("CtrlInfo","IIC_Camera_light");
        m_listVideoType = m_iniSettings_pre.getValueList("CtrlInfo","VideoType");
        m_listColorType = m_iniSettings_pre.getValueList("CtrlInfo","ColorType");
        m_listItemType = m_iniSettings_pre.getValueList("CtrlInfo","ItemType");
        m_otherInfo.m_CameraIP = m_iniSettings_pre.getValue("CtrlInfo","CameraIP","192.168.1.10");
        m_otherInfo.m_lidarIP = m_iniSettings_pre.getValue("CtrlInfo","LidarIP","192.168.2.201");
        m_listDisType = m_iniSettings_pre.getValueList("CtrlInfo","DisType");
        QString value_lidarSaveTime= m_iniSettings_pre.getValue("CtrlInfo","LidarSaveInterval","10");//激光雷达保存文件时间间隔RadarSaveInterval
        QString value_radarSaveTime= m_iniSettings_pre.getValue("CtrlInfo","RadarSaveInterval","10");//激光雷达保存文件时间间隔
        MemCache::instance()->setValue("LidarSaveInterval",value_lidarSaveTime);
        MemCache::instance()->setValue("RadarSaveInterval",value_radarSaveTime);

    }else{
        qDebug() << "[Error]can`t find iniFile:" + m_preInfo;
    }

}


void ConfigInfo::SetSenceInfo(const CSenceInfo& info)
{
    QMutexLocker lock(&m_mutex);

    m_senceInfo = info;
    m_iniSettings_pre.setValue("SenceInfo","wet",QString::number(m_senceInfo._Wet,'f',2));
    m_iniSettings_pre.setValue("SenceInfo","temp", QString::number(m_senceInfo._Temp,'f',2));
    m_iniSettings_pre.setValue("SenceInfo","scene", m_senceInfo._Scene);
    m_iniSettings_pre.setValue("SenceInfo","weather", m_senceInfo._Weather);
    m_iniSettings_pre.setValue("SenceInfo","timequantum", m_senceInfo._TimeQuantum);
    m_iniSettings_pre.setValue("SenceInfo","curTime", m_senceInfo._Time);
    for(auto i = 0;i < MAX_PANLE_NUM;i++){
        QString str = QString("Ch%1Type").arg(i+1);
        m_iniSettings_pre.setValue("SenceInfo",str, m_senceInfo._Project[i]);
    }

}

void ConfigInfo::GetSenceInfo(CSenceInfo& info)
{
    QMutexLocker lock(&m_mutex);
    info = m_senceInfo;
}
void ConfigInfo::GetSavePath(QString& path)
{
    QMutexLocker lock(&m_mutex);
    path = m_saveDir;
}
void ConfigInfo::SetSavePath(QString& path)
{
    QMutexLocker lock(&m_mutex);
    m_saveDir = path;
    m_iniSettings_pre.setValue("PreInfo","SaveDir",m_saveDir);
}

void ConfigInfo::Init(QString &&filename)
{
    QString _path = QCoreApplication::applicationDirPath() + "/IniConfig/";
    QDir dir;
    if(!dir.exists(_path)){
        dir.mkpath(_path);
    }
    m_preInfo = _path + filename;

    // m_preInfo = _path + "preConfig.ini";

    m_iniSettings_pre.settings_init(m_preInfo);
    // m_iniSettings_Info.settings_init(m_configPath);
    ReadAllConfig();
}

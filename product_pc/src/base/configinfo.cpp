#include "configinfo.h"
#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>
#include <QFile>
#include <CPropertyXml.h>
#include <QDebug>
#include <QFileInfo>
#include <util.h>
QString g_enumComport = "COM1,COM2,COM3,COM4,COM5,COM6,COM7,COM8,COM9,"
                        "COM10,COM11,COM12,COM13,COM14,COM15,COM16,COM17,COM18,COM19,"
                        "COM20,COM21,COM22,COM23,COM34,COM25,COM26,COM27,COM28,COM29,"
                        "COM30,COM31,COM32,COM33,COM44,COM35,COM36,COM37,COM38,COM39,"
                        "COM40,COM41,COM42,COM43,COM54,COM45,COM46,COM47,COM48,COM49,"
                        "COM50,COM51,COM52,COM53,COM64,COM55,COM56,COM57,COM58,COM59";

// {"GLOBAL", "MaxLifeTime", "2000", "String", LOGIN_ADMIN,"2000"},
static XmlConfigInfo g_xmlInfo[MaxConfigNum] = {
    {"GLOBAL","盒子IP",  "192.168.1.10", "String", LOGIN_NORMAL,""},
    {"GLOBAL","MES服务IP", "192.168.110.23", "String", LOGIN_NORMAL,""},
    {"GLOBAL","MES服务端口",  "32380", "String", LOGIN_NORMAL,""},
    {"GLOBAL","当前组件编码", "DE1000", "String", LOGIN_OWN,""},
    {"GLOBAL","是否设置过工序", "0", "String", LOGIN_OWN,""}, //4
    {"GLOBAL","LogPath",  "D:/data", "DirPath", LOGIN_ADMIN,""},
    {"GLOBAL","SN长度",  "15", "String", LOGIN_ADMIN,""},
    {"GLOBAL","SN前缀",  "XD01A", "String", LOGIN_ADMIN,""},
    {"GLOBAL","压测模式",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"GLOBAL","启动防锁屏",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"GLOBAL","调试模式",  "生产模式", "Enum", LOGIN_NORMAL,"生产模式,点检模式,调试模式"},
    {"GLOBAL","获取SN",  "手动扫码", "Enum", LOGIN_NORMAL,"手动扫码,产品读取,MES获取,自动扫码"},
    {"GLOBAL","项目选择",  "XD01A", "Enum", LOGIN_ADMIN,"XD01A,XD01B,XD02A,XD03A,XD05A,Sensor,Motan"},
    {"GLOBAL", "ReadCom", "COM2", "Enum", LOGIN_ADMIN,g_enumComport},
    {"GLOBAL", "WriteCom", "COM3", "Enum", LOGIN_ADMIN,g_enumComport},
    {"GLOBAL","SEMAC",  "", "String", LOGIN_ADMIN,""},
    {"GLOBAL","按钮选择器",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"GLOBAL", "按钮选择器端口", "COM3", "Enum", LOGIN_ADMIN,g_enumComport},
    {"GLOBAL", "耗材寿命管控",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"CE1001", "McuCom",  "COM1", "String", LOGIN_ADMIN,""},
    {"DE1001", "条码打印模板路径",  "D:/template.dsw", "FilePath", LOGIN_ADMIN,""},
    {"DE1013", "hotElecLimit", "50", "String", LOGIN_ADMIN,""},
    {"DE1013", "供电方式",  "Arm接口", "Enum", LOGIN_ADMIN,"Arm接口,Sensor接口"},
    {"DE1013", "McuCom", "COM3", "String", LOGIN_ADMIN,g_enumComport},
    {"DE1010", "offsetX", "0", "String", LOGIN_ADMIN,""},
    {"DE1010", "offsetY", "19", "String", LOGIN_ADMIN,""},
    {"CB1004", "盒子2IP",  "192.168.1.11", "String", LOGIN_ADMIN,""},
    {"CB1004", "USB检测盘符", "F", "String", LOGIN_ADMIN,""},
    {"CB1004","McuCom",  "COM3", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CB1004","PlcCom",  "COM3", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CB1004","ScannerCom",  "COM3", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CB1004","ProductCom",  "COM3", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CB1004","OEM软件版本",  "1.0.0.7", "String", LOGIN_ADMIN, ""},
    {"CB1004","OEM硬件版本",  "1.0.0.7", "String", LOGIN_ADMIN, ""},
    {"DE1006","LeakageCom",  "COM1", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE1006","PLCCom",  "COM1", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE1006","PressureFailLimit",  "30", "String", LOGIN_ADMIN,""},
    {"DE1006","LeakageFailLimit",  "50", "String", LOGIN_ADMIN,""},
    {"DE1006", "新气密设备",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1006", "LeakageScanCom", "COM3", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE1007", "IV3Support", "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1007", "IV3Address", "192.168.10.16", "String",LOGIN_ADMIN, ""},
    {"DE1008","FramePerSecond", "30", "String", LOGIN_ADMIN,""},
    {"DE1008","HvSupport",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","KSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","BPSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","TECSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","PLCSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","SFFCSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1008","TecCom",  "COM5", "Enum", LOGIN_ADMIN,g_enumComport},
    {"DE1008","TecTargetTemp", "50", "String", LOGIN_ADMIN,""},
    {"DE1008","焦温限制", "35", "String", LOGIN_ADMIN,""},
    {"DE1008","BPfilePath", "E:/data", "FilePath", LOGIN_ADMIN,""},
    {"DE1008","BpFileGetMethod", "本地获取", "Enum", LOGIN_ADMIN,"MES获取,本地获取,无坏点文件"},
    {"DE1008","DUT1IP", "192.168.1.11", "String", LOGIN_NORMAL,""},
    {"DE1008","DUT2IP", "192.168.1.12", "String", LOGIN_NORMAL,""},
    {"DE1008","DUT3IP", "192.168.1.13", "String", LOGIN_NORMAL,""},
    {"DE1008","DUT4IP", "192.168.1.14", "String", LOGIN_NORMAL,""},
    {"DE1009","FramePerSecond", "30", "String", LOGIN_ADMIN,""},
    {"DE1009","DUT1IP", "192.168.1.11", "String", LOGIN_NORMAL,""},
    {"DE1009","DUT2IP", "192.168.1.12", "String", LOGIN_NORMAL,""},
    {"DE1009","DUT3IP", "192.168.1.13", "String", LOGIN_NORMAL,""},
    {"DE1009","DUT4IP", "192.168.1.14", "String", LOGIN_NORMAL,""},
    {"DE1009","PitsSaveAllImage",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1009","PLCSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1009","PitsSupport",  "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"DE1009","PitsMaxValue",  "80", "String", LOGIN_ADMIN,""},
    {"DE1009","焦温限制", "35", "String", LOGIN_ADMIN,""},
    {"CB1100","JLinkPath", "E:/data", "DirPath", LOGIN_ADMIN,""},
    {"DE1100","ASICPath", "E:/data", "DirPath", LOGIN_ADMIN,""},
    {"DE1100","ASICCom", "COM4", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE1100","PythonPath", "D:/tool/python31", "DirPath", LOGIN_ADMIN,""},
    {"DE1100","PowerEXP", "000001", "String", LOGIN_ADMIN,""},
    {"DE1100","ASICEXP", "000002", "String", LOGIN_ADMIN,""},
    {"DE1100","SingleEXP", "000003", "String", LOGIN_ADMIN,""},
    {"DE1100","模板路径",  "btw/x7btw6d8.btw", "FilePath", LOGIN_ADMIN,""},
    {"DE1100","McuCom",  "COM3", "Enum", LOGIN_ADMIN,g_enumComport},
    {"DE1100","是否网络获取SN",  "1", "String", LOGIN_ADMIN,""},
    {"DE1100","是否烧录",  "1", "String", LOGIN_ADMIN,""},
    {"CL1003","CL1003点胶固化IP", "192.168.2.250", "String", LOGIN_NORMAL,""},
    {"CL1004","PLCSupport",  "False", "Enum", LOGIN_ADMIN,"True,False"},
    {"CL1005","点胶机串口",  "COM4", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1005","固化机串口",  "COM9", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1005","标准固化时间", "15", "String", LOGIN_ADMIN,""},
    {"CL1006","MCU串口",  "COM1", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1006","工装串口",  "COM2", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1006", "扫码串口", "COM3", "Enum", LOGIN_ADMIN,g_enumComport},
    {"CL1102","MCU串口",  "COM1", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1102","工装串口",  "COM2", "Enum", LOGIN_ADMIN, g_enumComport},
    {"CL1102", "扫码串口", "COM3", "Enum", LOGIN_ADMIN,g_enumComport},
    {"DE1013", "Spec", "10", "String", LOGIN_ADMIN,""},
    {"CB1005", "PacketMaximum", "10", "String", LOGIN_ADMIN,""},
    {"CB1005", "WorkOrder", "10", "String", LOGIN_ADMIN,""},
    {"CB1005", "PackageNumber", "10", "String", LOGIN_ADMIN,""},
    {"CB1005", "包装打印模板", "E:/data", "FilePath", LOGIN_ADMIN,""},
    {"CB1005", "是否在线模板", "True", "Enum", LOGIN_ADMIN,"True,False"},
    {"CL1007", "PacketMaximum", "10", "String", LOGIN_ADMIN,""},
    {"CL1007", "WorkOrder", "10", "String", LOGIN_ADMIN,""},
    {"CL1007", "PackageNumber", "10", "String", LOGIN_ADMIN,""},
    {"CL1007", "包装打印模板", "E:/data", "FilePath", LOGIN_ADMIN,""},
    {"CL1007", "是否在线模板", "True", "Enum", LOGIN_ADMIN,"True,False"},

    {"IsPackage", "包装产品总数", "24", "String", LOGIN_ADMIN, "1"},
    {"IsPackage", "包装产品层数", "2", "Enum", LOGIN_ADMIN, "1,2,3,4,5,6,7,8,9,10"},
//    {"IsPackage", "当前工单信息", "XYSC00000000001", "String", LOGIN_ADMIN, ""},
//    {"IsPackage", "产品AA码", "AA00000001", "String", LOGIN_ADMIN, ""},
    {"IsPackage", "包装打印模板", "E:/data", "FilePath", LOGIN_ADMIN, ""},
    {"IsPackage", "是否在线模板", "False", "Enum", LOGIN_ADMIN, "True,False"},
    {"IsPackage", "电子秤串口", "COM1", "Enum", LOGIN_ADMIN, g_enumComport},
    {"IsPackage", "单台产品重量下限与上限", "1;0.5;0.05", "String", LOGIN_ADMIN, ""},
    {"IsPackage", "内衬泡棉重量下限与上限", "0;0.5;0.05", "String", LOGIN_ADMIN, ""},
    {"IsPackage", "泡棉盖重量下限与上限", "0;0.5;0.05", "String", LOGIN_ADMIN, ""},
    {"IsPackage", "裸机总重量下限与上限", "1;0.5;0.05", "String", LOGIN_ADMIN, ""},
    {"IsPackage", "包装总重量下限与上限", "1;0.5;0.05", "String", LOGIN_ADMIN, ""},
    // XD05A线体新添加 DE2001-前后壳点胶 DE2002-前壳窗片点胶
    {"DE2001","点胶机串口",  "COM4", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE2001","固化机串口",  "COM9", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE2001","标准固化时间", "15", "String", LOGIN_ADMIN,""},
    {"DE2002","点胶机串口",  "COM4", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE2002","固化机串口",  "COM9", "Enum", LOGIN_ADMIN, g_enumComport},
    {"DE2002","标准固化时间", "15", "String", LOGIN_ADMIN,""},
    {""}
};

static QString getXmlValue(CPropertyXml * xml, int idx, QString sCodeId = "")
{
    QString value;
    QString group = g_xmlInfo[idx].group;
    if(group != "GLOBAL") {
        group = sCodeId;
    }
    xml->getValue(group,
                  g_xmlInfo[idx].name,
                  "value",
                  value);
    qInfo("get name %s value %s",
          g_xmlInfo[idx].name.toStdString().data(),
          value.toStdString().data());
    return value;
}

ConfigInfo::ConfigInfo()
{
    qDebug() << "ConfigInfo()" << sysInfo().curProduce;
    qDebug() << "ConfigInfo()" << cacheInfo().sCodeId;

    Util::createFileDir(Util::getAllPath("config"));
    m_configPath = QCoreApplication::applicationDirPath() + "/config/config.xml";
    QFile file(m_configPath);
    CPropertyXml &xml = m_xml;
    QFileInfo fileInfo(m_configPath);
    qInfo()<<"file is exists:" << file.exists() << " isFile:" << fileInfo.isFile();
    if(file.exists()) {
        qInfo()<<"load file " << m_configPath;
        if(xml.load(m_configPath)) {
            qInfo("load xml config ok!!! ");
        }else {
            qInfo("load xml config failed");
        }
    }else {
        qInfo()<<m_configPath<<" is not exist";
    }
    qInfo("init config");
    for(int i = 0; i < MaxConfigNum; i ++) {
        if(g_xmlInfo[i].group.length() <= 0) {
            break;
        }
        if(!xml.groupExists(g_xmlInfo[i].group)) {
            xml.addGroup(g_xmlInfo[i].group);
            qInfo("xml add group %s", g_xmlInfo[i].group.toStdString().data());
        }
        xml.setAttribute(g_xmlInfo[i].group,
                         g_xmlInfo[i].name,
                         "type",
                         g_xmlInfo[i].type,
                         true);
        xml.setValue(g_xmlInfo[i].group,
                         g_xmlInfo[i].name,
                         "value",
                         g_xmlInfo[i].value,
                     true);

        xml.setValue(g_xmlInfo[i].group,
                         g_xmlInfo[i].name,
                         "permission",
                         QString::number(g_xmlInfo[i].pro),
                     true);
        if(g_xmlInfo[i].type == "Enum") {
            xml.setAttribute(g_xmlInfo[i].group,
                             g_xmlInfo[i].name,
                             "EnumNames",
                             g_xmlInfo[i].enumNames,
                             true);
        }
    }
    xml.save(m_configPath);
    ReadAllConfig(1);
}

ConfigInfo::~ConfigInfo()
{

}

ConfigInfo* ConfigInfo::getInstance()
{
    static ConfigInfo instance;
    return &instance;
}

void ConfigInfo::ReadAllConfig(int iInit)
{
    if(!iInit) {
        m_mutex.lock();
        QFile file(m_configPath);
        CPropertyXml &xml = m_xml;
        if(file.exists()) {
            qInfo("load file ");
            if(xml.load(m_configPath)) {
            }
        }
        m_mutex.unlock();
    }
    QString mode = getValueString("调试模式");
    if(mode == "点检模式") {
        m_sysInfo.mode = 1;
    }else if(mode == "调试模式") {
        m_sysInfo.mode = 2;
    }else if(mode == "离线模式") {
        m_sysInfo.mode = 3;
    }else {
        m_sysInfo.mode = 0;
    }

    if(!iInit) {
        m_mutex.lock();
    }

    m_sysInfo.ip = getXmlValue(&m_xml, 0);
    m_sysInfo.mesIp = getXmlValue(&m_xml, 1);
    m_sysInfo.mesPort = getXmlValue(&m_xml, 2);
    m_sysInfo.curProduce = getXmlValue(&m_xml, 3);
    m_sysInfo.iFirst = getXmlValue(&m_xml, 4).toInt();
    m_sysInfo.logPath = getXmlValue(&m_xml, 5);
    m_sysInfo.mcuCom = getXmlValue(&m_xml, 7, m_cacheInfo.sCodeId);
    m_sysInfo.readCom = getXmlValue(&m_xml, 8, m_cacheInfo.sCodeId);
    m_sysInfo.writeCom = getXmlValue(&m_xml, 9, m_cacheInfo.sCodeId);
    m_sysInfo.printerModelPath = getXmlValue(&m_xml, 10, m_cacheInfo.sCodeId);
    m_sysInfo.hotElecLimit = getXmlValue(&m_xml, 13, m_cacheInfo.sCodeId);
    m_sysInfo.picDataPath = getXmlValue(&m_xml, 15, m_cacheInfo.sCodeId);
    m_sysInfo.sOffsetX = getXmlValue(&m_xml, 16, m_cacheInfo.sCodeId);
    m_sysInfo.sOffsetY = getXmlValue(&m_xml, 17, m_cacheInfo.sCodeId);
    m_sysInfo.lvdsIp = getXmlValue(&m_xml, 20, m_cacheInfo.sCodeId);

    if(!iInit) {
        m_mutex.unlock();
    }
    qInfo("ReadAllConfig %d", iInit);
}

bool ConfigInfo::getValueString(QString group, QString key, QString &value)
{
    if (!m_xml.getValue("GLOBAL", key, "value", value))
        return m_xml.getValue(group, key, "value", value);
    return true;
}

bool ConfigInfo::getValueBoolean(QString group, QString key, bool &value)
{
    QString val;
    if (getValueString(group, key, val)) {
        value = (val.compare("True", Qt::CaseInsensitive) == 0);
        return true;
    }

    return false;
}

bool ConfigInfo::getValueInt32(QString group, QString key, int &value)
{
    QString val;
    bool brt = false;
    if (getValueString(group, key, val)) {
        value = val.toInt(&brt);
        return brt;
    }

    return false;
}

QString ConfigInfo::getValueString(QString key)
{
    QString value;
    QMutexLocker lock(&m_mutex);
    if (!m_xml.getValue("GLOBAL", key, "value", value)) {
        m_xml.getValue(m_cacheInfo.sCodeId, key, "value", value);
    }
    return value;
}

void ConfigInfo::addConfig(XmlConfigInfo cInfo, bool bSave)
{
    CPropertyXml &xml = m_xml;
    if(cInfo.group.length() <= 0) {
        return;
    }
    QMutexLocker lock(&m_mutex);
    if(!xml.groupExists(cInfo.group)) {
        xml.addGroup(cInfo.group);
        qInfo("xml add group %s", cInfo.group.toStdString().data());
    }
    xml.setAttribute(cInfo.group,
                     cInfo.name,
                     "type",
                     cInfo.type,
                     true);
    xml.setValue(cInfo.group,
                     cInfo.name,
                     "value",
                     cInfo.value,
                 true);

    xml.setValue(cInfo.group,
                     cInfo.name,
                     "permission",
                     QString::number(cInfo.pro),
                 true);
    if(cInfo.type == "Enum") {
        xml.setAttribute(cInfo.group,
                         cInfo.name,
                         "EnumNames",
                         cInfo.enumNames,
                         true);
    }
    if(bSave) {
        xml.save(m_configPath);
    }
}

QString ConfigInfo::configPath() const
{
    return m_configPath;
}

CacheInfo ConfigInfo::cacheInfo()
{
    QMutexLocker lock(&m_mutex);
    return m_cacheInfo;
}

void ConfigInfo::setCacheInfo(CacheInfo &info)
{
    QMutexLocker lock(&m_mutex);
    m_cacheInfo = info;
}

SysInfo ConfigInfo::sysInfo()
{
    QMutexLocker lock(&m_mutex);
    return m_sysInfo;
}

void ConfigInfo::setSysInfo(SysInfo &info)
{

    QMutexLocker lock(&m_mutex);
//    if(m_sysInfo.curProduce == info.curProduce && m_sysInfo.iFirst == info.iFirst) {
//        return;
//    }
    m_sysInfo = info;
    CPropertyXml &xml = m_xml;
    xml.setValue(g_xmlInfo[3].group,
            g_xmlInfo[3].name,
            "value",
            m_sysInfo.curProduce, false);
    xml.setValue(g_xmlInfo[4].group,
            g_xmlInfo[4].name,
            "value",
    QString::number(m_sysInfo.iFirst), false);
    QString sCode;
    xml.getValue(g_xmlInfo[3].group,
            g_xmlInfo[3].name,
            "value", sCode);
    qInfo("init config code %s sCode %s", m_sysInfo.curProduce.toStdString().data(),
          sCode.toStdString().data());

    xml.save(m_configPath);
    QFile file(m_configPath);
    qInfo()<<m_configPath<<" "<<file.exists();
    return ;
}


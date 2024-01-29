#include "autofpccheck.h"
#include <QDateTime>
#include "configinfo.h"
#include "util.h"
#include "StaticSignals.h"
#include <QThread>
#include <QSettings>
#include <QTextCodec>
#include <QTimer>

#define CURCODE ConfigInfo::getInstance()->getValueString("当前组件编码")
#define ADDCONFIG(name,value,type,pro,enumNames,bSave) \
{ \
    XmlConfigInfo info={CURCODE,name,value,type,pro,enumNames}; \
    ConfigInfo::getInstance()->addConfig(info, bSave); \
}

#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)


AutoFpcCheck::AutoFpcCheck()
{
    ADDCONFIG("FPC检测开关", "True", "Enum", LOGIN_ADMIN, "True,False", true);
    initStateMachine();

}

void AutoFpcCheck::addLog(QString log, const int  result)
{
    qDebug() << log;
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
//        showPass();
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

void AutoFpcCheck::updateTable(QString name, qint64 workTime, bool result)
{
    if(result) {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "PASS");
        emit tableUpdateTestStatus(0);
    } else {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "FAIL");
        emit tableUpdateTestStatus(1);
    }
}

void AutoFpcCheck::slotConnectBtnClicked()
{

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    qDebug() << m_StationName;

//    if (projectName() != "XD01A") {
//        addLog("项目编号错误，当前工位只有XD01A项目支持", -1);
//        return;
//    }

    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "FPC检测开关", m_fpcUsing);

    addLog("当前需要使用FPC? is " + m_fpcUsing ? "Using" : "Not using");

    if (m_fpcUsing) {
        FpcHandler* fpcTmp = new FpcHandler;
        if(!fpcTmp->initPlcWrite("192.168.1.250", 502)) {
            addLog("fpc写端口连接异常", -1);
            return;
        }
        if(!fpcTmp->initPlcRead("192.168.1.250", 501)) {
            addLog("fpc读端口连接异常", -1);
            return;
        }
        if(!fpcTmp->initCamera("192.168.1.63", 1000)) {
            addLog("CCD相机连接异常, 请检查网络连接", -1);
            return;
        }
        fpcTmp->deInit();
    }
    BaseProduce::slotConnectBtnClicked();
}


void AutoFpcCheck::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &AutoFpcCheck::slotsStoped);
    add(0, "check next", std::bind(&AutoFpcCheck::checkNext, this));
    add(10, "check life", std::bind(&AutoFpcCheck::jugdeCustomLifeTime, this, true));
    add(20, "init", std::bind(&AutoFpcCheck::init, this));
    add(30, "get sn", std::bind(&AutoFpcCheck::getSn, this));
    add(40, "enter mes", std::bind(&AutoFpcCheck::enterMes, this));
    add(50, "getLenSn", std::bind(&AutoFpcCheck::getLenSn, this));
    add(51, "getSignalSn", std::bind(&AutoFpcCheck::getSignalSn, this));
    add(60, "tips work", std::bind(&AutoFpcCheck::tipsWork, this));

//    add(70, "check product", std::bind(&AutoFpcCheck::checkProduct, this));
//    add(80, "check Shutter", std::bind(&AutoFpcCheck::checkShutter, this));
    add(90, "auto process", std::bind(&AutoFpcCheck::autoProcess, this));
    add(100, "mes upload", std::bind(&AutoFpcCheck::mesUpload, this));
    add(110, "out mes", std::bind(&AutoFpcCheck::outMes, this));
    add(120, "report", std::bind(&AutoFpcCheck::stopWork, this));
}

void AutoFpcCheck::slotsStoped()
{
    addLog("停止工作");
    if(!isWorking()) {
        return;
    }
    startProcess();
}


void AutoFpcCheck::checkNext()
{
    ComGuideCfg::getGuideCfgList(m_tipsList);
    m_bResultCode = true;
    m_errorCode=0;

    QString produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    QString configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(produceName);
    QSettings reader(configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));
    reader.beginGroup("config");
    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();

}

void AutoFpcCheck::init()
{ 
    addLog("初始化连接");
    if (m_fpcUsing) {
        if(m_fpc == nullptr) {
            m_fpc = new FpcHandler;
        }

        if(!m_fpc->initPlcWrite("192.168.1.250", 502)) {
            addLog("fpc写端口连接异常", -1);
            return;
        }
        if(!m_fpc->initPlcRead("192.168.1.250", 501)) {
            addLog("fpc读端口连接异常", -1);
            return;
        }
        if(!m_fpc->initCamera("192.168.1.63", 1000)) {
            addLog("CCD相机连接异常, 请检查网络连接", -1);
            return;
        }
        m_fpc->plcSetResult(true);
        m_fpc->plcAutoMode();
    }

    emit tableAddHeader(QStringList() << "检测项" << "检测时间(ms)" << "结果");
    emit tableClearData();
    emit logClearData();
    m_csvList.clear();
}

void AutoFpcCheck::getSn()
{
    if(!isWorking()) {
        return;
    }
    addLog("开始扫码 SN");

    QString path = m_imgPath + "/SN0.png";
    StaticSignals::getInstance()->statusUpdateImg(path);
    m_sn = msgBox("请扫码 SN");
    if(!isWorking()) {
        return;
    }

    if (ConfigInfo::getInstance()->cacheInfo().sCodeId == "GP1000") {
        m_sn = m_sn.left(19);
        m_sn = m_sn.insert(15, '*');
        m_sn = m_sn.insert(15, '*');
    }

    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
    }

    addLog("SN: " + m_sn);

    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    m_logPath = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd")
            + "/" + dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
    QDir dir(m_logPath);
    if (!dir.exists()) {
        dir.mkpath(m_logPath);
    }
}

void AutoFpcCheck::getLenSn()
{
    if(!isWorking()) {
        return;
    }
    addLog("扫镜头组件SN");
    QString lenSn = msgBox("扫镜头组件SN");
    MesCheckItem item;
    item.sItem = "sensor_complete_sn";
    if(lenSn.size()< 11) {
        logFail("镜头组件编码失败");
        item.sResult = MESFAILED;
        m_errorCode = -2;
    }else {
        lenSn = lenSn.left(11);
    }
    addLog("获取镜头编码 " + lenSn);

    item.sValue = lenSn;
    item.sResult = MESPASS;
    MesCom::instance()->addItemResultEx(item);
}

void AutoFpcCheck::getSignalSn()
{
    if(!isWorking()) {
        return;
    }
    addLog("扫信号板SN编码");
    QString signalSn = msgBox("扫信号板SN编码");
    MesCheckItem item;
    item.sItem = "SignalPCBA_SN";
    if(signalSn.size()< 15) {
        logFail("镜头组件编码失败");
        item.sResult = MESFAILED;
        m_errorCode = -2;
    }else {
        signalSn = signalSn.left(15);
    }
    addLog("获取信号板编码 " + signalSn);

    item.sValue = signalSn;
    item.sResult = MESPASS;
    MesCom::instance()->addItemResultEx(item);
}

void AutoFpcCheck::tipsWork()
{
    if(m_errorCode != 0) return;
    if(!isWorking()) {
        return;
    }

    bool useSelector = false;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "按钮选择器", useSelector);
    for(int i=0; i<m_tipsList.size(); i++) {
        QTime timer;
        timer.start();

        addLog(m_tipsList[i].tips);
        StaticSignals::getInstance()->statusUpdateImg(m_tipsList[i].path);
        if(GETCONFIG("弹框格式") == "确认框") {
            msgBox(m_tipsList[i].msgTips, 3);
        } else if (GETCONFIG("弹框格式") == "选择框") {
            QString result = msgBox(m_tipsList[i].msgTips, 2);
            if(result != "PASS") {
                logFail(m_tipsList[i].msgTips + " FAILED");
                m_errorCode = -13;
            }
        } else {
            msgBox(m_tipsList[i].msgTips, 3);
        }
        if(!isWorking()) {
            return;
        }
        updateTable(m_tipsList[i].tips, timer.elapsed(), m_errorCode == 0);
        if (useSelector) {
            QThread::msleep(500);
        }
    }
}

//void AutoFpcCheck::checkProduct()
//{
//    if(m_errorCode != 0) return;
//    if(!isWorking()) {
//        return;
//    }
//    addLog("请将产品放置在胎具中");
//}

//void AutoFpcCheck::checkShutter()
//{
//    if(m_errorCode != 0) return;
//    if(!isWorking()) {
//        return;
//    }
//    addLog("窗片方向检测");
//}

void AutoFpcCheck::autoProcess()
{
    if (!m_fpcUsing) return;

    // 只有XD01A才需要这个操作，非XD01A在此工序不需要这个操作
    if (!projectName().contains("XD01A")) return;

    if(m_errorCode != 0) return;
    int fpcCounts = 0;
    int cameraCounts = 0;
    int checkCounts = 0;
    int workStep = 0;
    bool workResult = true;
    QString errTips;
    while(isWorking()) {
        QTime timer;
        timer.start();
        switch(workStep) {
        case 0:{
            qDebug() << "count is " << fpcCounts << cameraCounts;
            if(fpcCounts >= 3 || cameraCounts >= 3){
                workStep = 110;
                m_errorCode = -21;
            } else {
                workStep = 10;
            }
            m_fpc->plcClearRead();
            break;
        }
        case 10:{
            addLog(errTips + "扣合盖板, 按压启动按钮");
            m_fpc->plcReadyOk();
            int checkResult = m_fpc->plcWaitStart();
            if(checkResult == 1) {
                checkCounts = 0;
                workStep = 20;
                updateTable("锗玻璃检测", timer.elapsed(), true);
            } else if(checkResult == 0) {
                updateTable("锗玻璃检测", timer.elapsed(), false);
                checkCounts++;
                if(checkCounts >= 3) {
                    workStep = 110;
                }
            }
            break;
        }
        case 20:{
            addLog("开始FPC检测");
            m_fpc->plcMoveTo(PLC_MOVE_FPC_CHECK);
            FpcCheckResult fpcResult = m_fpc->cameraCheck();
            if(fpcResult.result) {
                workStep = 30;
                fpcCounts = 0;
                workResult = true;
                updateTable("FPC检测", timer.elapsed(), true);
            } else {
                m_fpc->plcSetResult(false);
                workResult = false;
                workStep = 0;
                fpcCounts++;
                if(fpcCounts <= 3) {
                    updateTable("FPC检测", timer.elapsed(), false);
                    addLog("请按NG按钮回原点");
                    errTips = "重新装配FPC排线, ";
                    m_fpc->plcWaitOrigin();
                }
            }
            m_csvList.append(m_sn);
            m_csvList.append(fpcResult.result ? "pass" : "fail");
            m_csvList.append(QString::number(fpcCounts));
            m_csvList.append(QString::number(fpcResult.mm));
            m_csvList.append(QString::number(fpcResult.jdc));
            break;
        }
        case 30:{
            addLog("开始快门检测");
            m_fpc->plcMoveTo(PLC_MOVE_SHUTTER_CHECK);
            if(m_fpc->plcCheck()) {
                workStep = 100;
                cameraCounts = 0;
                workResult = true;
                updateTable("快门检测", timer.elapsed(), true);
            } else {
                m_fpc->plcSetResult(false);
                workResult = false;
                workStep = 0;
                cameraCounts++;
                if(cameraCounts <= 3) {
                    updateTable("快门检测", timer.elapsed(), false);
                    addLog("请按NG按钮");
                    errTips = "重新装配快门线, ";
                    m_fpc->plcWaitOrigin();
                }
            }
            addLog("快门检测完成");
            break;
        }
        case 100:{
            addLog("检测完成，长按启动按钮回原点");
            m_fpc->plcSetResult(true);
            m_fpc->plcWaitOrigin();
            QString path = m_imgPath + "/fpc.png";
            StaticSignals::getInstance()->statusUpdateImg(path);
            addLog("请粘贴FPC排线和快门线无纺布");
            msgBox("请粘贴FPC排线和快门线无纺布", 3);
            addLog("PASS", 1);
            return;
        }   
        case 110:{
            addLog("测试Fail", -1);
            m_fpc->plcSetResult(false);
//            m_fpc->plcWaitOrigin();
            return;
        }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void AutoFpcCheck::mesUpload()
{
    if(!isWorking()) {
        return;
    }
    if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
        return;
    }

    if(m_errorCode != 0) {
        m_bResultCode = false;
        showFail(m_errMsg);
    } else {
        m_bResultCode = true;
    }
}

void AutoFpcCheck::stopWork()
{
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath") + QDateTime::currentDateTime().toString("/yyyy_MM_dd/");
    Util::createFileDir(logPath);
    QString reportPath = logPath + "/data.csv";
    m_csvLogger.setCsvLogPath(reportPath);
    QFile file(reportPath);
    if(!file.exists()) {
        QStringList headList;
        headList << "SN码" << "次数" << "result" << "mm" << "jdc";
        m_csvLogger.addCsvTitle(headList);
    }
    m_csvLogger.addCsvLog(m_csvList);

    if (!m_bResultCode) {
        showFail(m_errMsg);
    } else {
        showPass();
    }

    msgBox("请更换产品进行下一轮测试", 4);
    stopProcess();
}

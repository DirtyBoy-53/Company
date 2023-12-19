#include "autoprocesscheckSensor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QStateMachine>
#include <QDateTime>
#include <QFinalState>
#include <QMessageBox>

#include "configinfo.h"
#include "asiccontrol.h"
#include "mcuserialmanager.h"
#include "util.h"
#include "CMyPrinter.h"
#include "inputmessagebox.h"
#include "basecomponent.h"
#include "messageid.h"
#include "csvlogger.h"
#include "util.h"
#include "imagecheck.h"
#include <threadparty.h>
#include "imgCalY8.h"
#include "yuv2rgb.h"
#include "mapping.h"
#include "mappingnew.h"
#include "Frock_cal.h"


#define CURCODE ConfigInfo::getInstance()->getValueString("当前组件编码")
#define ADDCONFIG(name,value,type,pro,enumNames,bSave) \
{ \
    XmlConfigInfo info={CURCODE,name,value,type,pro,enumNames}; \
    ConfigInfo::getInstance()->addConfig(info, bSave); \
}

#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)

AutoProcessCheckSensor::AutoProcessCheckSensor()
{
    QString modulePath = QCoreApplication::applicationDirPath() + "/res/XD02A摄像头标签.btw";
    ADDCONFIG("打印模板文件", modulePath, "FilePath",LOGIN_ADMIN,"", true);

    initStateMachine();
}

void AutoProcessCheckSensor::slotConnectBtnClicked()
{
    QString ip = ConfigInfo::getInstance()->getValueString("盒子IP");
    if(Util::testPing(ip)) {
        if(m_webSocketIvi == nullptr) {
            m_webSocketIvi = new WebSocketClientManager();
            m_webSocketIvi->connectToUrl(ip, 6000);
            connect(m_webSocketIvi,&WebSocketClientManager::signalRecvVideoFrame,this,&AutoProcessCheckSensor::slotRecvVideoFrameWsIvi);
        }
    } else {
        addLog("网络异常, 请检查工装盒ip及网络连接", -1);
        return;
    }

    QString mcuCom = ConfigInfo::getInstance()->getValueString("McuCom");
    McuSerialManager testSerial;
    int ret = testSerial.openSerial(mcuCom);
    if(ret) {
        //addLog("请点击开始工作");
        //updateStatus(1);
        testSerial.closeSerial();
    } else {
        addLog("串口异常, 请检查接线", -1);
        return;
    }
    BaseProduce::slotConnectBtnClicked();
}

bool AutoProcessCheckSensor::init()
{
    addLog("初始化连接");
    m_start = 0;
    m_seqImage = 0;
    QString ip = ConfigInfo::getInstance()->getValueString("盒子IP");
    if(m_mcuSerial == nullptr) {
        m_mcuSerial = new McuSerialManager();
    }
    if(m_asic == nullptr) {
        m_asic = new AsicControl(ip);
    }

    m_reportList.clear();
    emit tableAddHeader(QStringList() << "检测项" << "检测期望值" << "当前值" << "检测时间(ms)" << "结果");
    emit tableClearData();
    emit logClearData();

    m_endTime = 0;
    m_errorCode = 0;
    m_errMsg.clear();

    QString mcuCom;
    ConfigInfo::getInstance()->getValueString("DE1013", "McuCom", mcuCom);
    int ret = m_mcuSerial->openSerial(mcuCom);
    if(!ret) {
        m_errorCode = -99;
        return false;
    }
    m_ioState = m_mcuSerial->pressed();
    logNormal(QString("当前工装MCU版本位: %1").arg(m_mcuSerial->version(), 2, QLatin1Char('0')));

    QString psWay = "Arm供电";
    ConfigInfo::getInstance()->getValueString("DE1013", "供电方式", psWay);
    if (psWay.toUpper().contains("ARM")) {
        m_powerSupply = PowerSupply_Arm;
        logNormal("当前采用Arm口供电");
    } else if (psWay.toUpper().contains("SENSOR")) {
        m_powerSupply = PowerSupply_Sensor;
        logNormal("当前采用Sensor口供电");
    } else {
        m_errorCode = -99;
        m_errMsg = "没有对应的供电方式";
        return false;
    }
    addLog("开始检测");

    return true;
}

void AutoProcessCheckSensor::slotRecvVideoFrameWsIvi(QByteArray frameData, FrameInfo_S frameInfo)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);
    info.width = 640;
    info.height = 512;
    info.frameLen = frameInfo.uY16Len + frameInfo.uYuvLen;
    info.nvsLen = frameInfo.uNvsLen;
    info.paramLen = frameInfo.uParamLen;
    if(frameInfo.uFormat == 1) {
        info.format = VIDEO_FORMAT_UYUV;
    } else if(frameInfo.uFormat == 3) {
        info.format = VIDEO_FORMAT_Y16;
    } else if(frameInfo.uFormat == 5) {
        info.format = VIDEO_FORMAT_Y16_Y8;
    } else if(frameInfo.uFormat == 7) {
        info.format = VIDEO_FORMAT_X16;
    }
    VideoInfo::getInstance()->setVideoInfo(0, info);

    if(m_videoQueue.size() < 300) {
        m_videoQueue.put(frameData.mid(frameInfo.uNvsLen));
    }
    emit videoUpdate(0, frameData, frameInfo.uTimeTm);
    m_iviFrameCount++;
//    qDebug() << "ivi count " << m_iviFrameCount;
}

void AutoProcessCheckSensor::slotsStoped()
{
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath") + QDateTime::currentDateTime().toString("/yyyy_MM_dd/");
    Util::createFileDir(logPath);

    QString csvPath = logPath + "/" + m_sn + ".csv";
    tableSaveCsv(csvPath);

    QString reportPath = logPath + "/data.csv";
    m_csvLogger.setCsvLogPath(reportPath);

    QFile file(reportPath);
    if(!file.exists()) {
        QStringList headList;
        headList << "SN码" << "版本号" << "功率检测" << "窗口加热" << "内参校验" << "快门检测" << "宽电压" << "错误码"<<"横纹波动";
        m_csvLogger.addCsvTitle(headList);
    }
    for(int i= m_reportList.size(); i<8; i++) {
        m_reportList.append("-");
    }

    m_reportList.append(QString::number(m_errorCode));
    m_csvLogger.addCsvLog(m_reportList);

    startProcess();
}

void AutoProcessCheckSensor::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &AutoProcessCheckSensor::slotsStoped);
    add(0, "check next", std::bind(&AutoProcessCheckSensor::checkNext, this));
    add(5, "init", std::bind(&AutoProcessCheckSensor::init, this));
    add(10, "check next", std::bind(&AutoProcessCheckSensor::jugdeCustomLifeTime, this, true));
    add(20, "check io", std::bind(&AutoProcessCheckSensor::checkIoState, this));
    add(30, "power on", std::bind(&AutoProcessCheckSensor::workStepPowerOn, this));
//    add(104, "wide voltage", std::bind(&AutoProcessCheckSensor::workStepVoltageWide, this));
//    return;

    add(40, "get sn", std::bind(&AutoProcessCheckSensor::getSn, this));
    add(41, "enter mes", std::bind(&AutoProcessCheckSensor::enterMes, this));
    add(50, "compare version", std::bind(&AutoProcessCheckSensor::workStepCompareVersion, this));
    add(51, "compare age flag", std::bind(&AutoProcessCheckSensor::workCheckAgeFlag, this));
    add(60, "check power", std::bind(&AutoProcessCheckSensor::workStepCheckPower, this));
    add(80, "window hot", std::bind(&AutoProcessCheckSensor::workStepWindowHot, this));


    //add(90, "wide voltage", std::bind(&AutoProcessCheckSensor::workStepVoltageWide, this));

    add(100, "check video auto", std::bind(&AutoProcessCheckSensor::workCheckVideoAuto, this));
    add(101, "check video manual", std::bind(&AutoProcessCheckSensor::workCheckVideoManual, this));
    add(102, "check shutter", std::bind(&AutoProcessCheckSensor::workStepCheckShutter, this));
    add(103, "check param", std::bind(&AutoProcessCheckSensor::workCheckCalibIn, this));//内参读取要靠后
    //add(101, "check video", std::bind(&AutoProcessCheckSensor::EvaluateHorStripe, this));

    add(104, "wide voltage", std::bind(&AutoProcessCheckSensor::workStepVoltageWide, this));
    add(110, "printer", std::bind(&AutoProcessCheckSensor::workPrinter, this));
    add(120, "compare sn", std::bind(&AutoProcessCheckSensor::workCompareSn, this));
    add(130, "reset", std::bind(&AutoProcessCheckSensor::workReset, this));
    add(131, "upload mes", std::bind(&AutoProcessCheckSensor::mesUpload, this));
    add(132, "out mes", std::bind(&AutoProcessCheckSensor::outMes, this));
    add(140, "report", std::bind(&AutoProcessCheckSensor::stopWork, this));
}

void AutoProcessCheckSensor::checkNext()
{
    if(m_errorCode != 0) {
        msgBox("NG 请将不良品放入不良品盒", 3);
    }
	m_bResultCode = true;
    tClear();
    logClear();
}

void AutoProcessCheckSensor::updateTableResult(QStringList list)
{
    emit tableAddRowData(list);
    QString end = list[list.size() -1];
    if(end == "PASS") {
        emit tableUpdateTestStatus(0);
    } else {
        emit tableUpdateTestStatus(1);
    }
}

void AutoProcessCheckSensor::stopWork()
{
    reportResult();

    m_endTime = 0;
    // todo 要改板，暂时用控制盒供电端口
    if (m_powerSupply == PowerSupply_Sensor) {
        m_mcuSerial->controlSensorPower(0, true, false);
    } else {
        m_mcuSerial->controlArmPower(0, true, false);
    }
    m_mcuSerial->closeSerial();

    stopProcess();
}

void AutoProcessCheckSensor::workStepPowerOn()
{
    if(m_errorCode != 0) return;

    addLog("开始上电初始化");

    m_startTime = QDateTime::currentMSecsSinceEpoch();
    QString proj = projectName();
    int vol = 12;
    if (proj.contains("XD01A")) {
        vol = 15;
    } else {
        vol = 12;
    }
    m_dataVoltage = vol;

    // todo 要改板，暂时用控制盒供电端口
    if (m_powerSupply == PowerSupply_Sensor) {
        qDebug() << "Sensor口上电";
        m_mcuSerial->controlSensorPower(vol, true, false);
        QThread::msleep(2000);// 等待稳定
        m_dataVoltage = m_mcuSerial->getSensorVoltage();
    } else {
        qDebug() << "ARM口上电";
        m_mcuSerial->controlArmPower(vol);
        QThread::msleep(2000);// 等待稳定
        m_dataVoltage = m_mcuSerial->getArmVoltage();
    }

    addLog("获取到实际输入电压值为(V)：" + QString::number(m_dataVoltage, 'f', 4));

    if (m_dataVoltage > (vol + 0.5) || m_dataVoltage < (vol - 0.5)) {
        m_errorCode = -1;
        addLog("上电电压比对失败, 超门限", -1);
        m_endTime = QDateTime::currentMSecsSinceEpoch();
        QString useTime = QString::number(m_endTime - m_startTime);
        updateTableResult(QStringList() << "供电测试" << QString("%1V ~ %2V").arg(QString::number(vol-0.5)).arg(QString::number(vol+0.5))
                          << QString("%1V").arg(QString::number(m_dataVoltage)) << useTime << "FAIL");
        return;
    }

    addLog("等待产品上电完成...");
    showProcess("等待产品上电完成...");
    QElapsedTimer timer;
    timer.start();
    bool brst = false;
    addLog("出图检测...");
    while(timer.elapsed() < 20000) {
        int iviFlag = m_iviFrameCount;
        QThread::sleep(1);
        int iviOffset = m_iviFrameCount - iviFlag;
        if(iviOffset > 10) {
            brst = true;
            break;
        }
    }

    if (!brst) {
        m_errorCode = -17;
        addLog("产品出图失败...", -1);
        return;
    }

    QThread::msleep(5000);

    brst = false;
    addLog("切伪彩：白热...");
    for(int foo = 0; foo < 3; foo++) {
        if (m_asic->SetPattle(0)) { // 切白热
            brst = true;
            break;
        }
        QThread::msleep(500);
    }

    addLog("关自适应补偿...");
    for(int foo = 0; foo < 3; foo++) {
        if (m_asic->AutoAdapteComp(0)) { // 关了自适应补偿
            brst &= true;
            break;
        }
        QThread::msleep(500);
    }

    QString rst = "PASS";
    if (!brst) {
        vol = 0;
        m_errorCode = -10;
        rst = "FAIL";
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "供电测试" << QString("%1V ~ %2V").arg(QString::number(vol-0.5)).arg(QString::number(vol+0.5))
                      << QString("%1V").arg(QString::number(vol)) << useTime << rst);
    m_startTime = m_endTime;

    addLog("初始化完成");
}

void AutoProcessCheckSensor::checkIoState()
{
    if(m_errorCode != 0) return;

    addLog("等待开始工作信号，请按下启动按钮...");
    while(true) {
        bool ioState = m_mcuSerial->pressed();
        if(m_ioState != ioState) {
            m_ioState = ioState;
            if(m_ioState) {
                break;
            }
        }
        QThread::msleep(200);
    }
}

void AutoProcessCheckSensor::getSn()
{
    if(m_errorCode != 0) return;

    addLog("读取SN...");
    for (int foo = 0; foo < 3; foo++) {
        m_sn = m_asic->GetSn();
        if (!m_sn.isEmpty()) {
            break;
        }
        QThread::msleep(500);
    }

    if(m_sn.isEmpty()) {
        m_errorCode = -18;
        m_errMsg = "读取SN为空";
        return;
    }

    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
    }
    m_reportList.append(m_sn);
    addLog("读取SN : " + m_sn);

    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    m_logPath.clear();
    m_logPath = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd") + "/" + dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
    QDir dir(m_logPath);
    if (!dir.exists()) {
        dir.mkpath(m_logPath);
    }
}

void AutoProcessCheckSensor::workStepCompareVersion()
{
    if(m_errorCode != 0) return;

    addLog("开始ASIC版本号测试...");

    QString asicVersion = "-";
    m_asic->GetAsicVersion(asicVersion);
    //v1.0.12
//    if(asicVersion.lastIndexOf(".") >= 0) {
//        asicVersion = asicVersion.left(asicVersion.lastIndexOf("."));
//    }
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString expVersion;
    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        MesCom::instance()->getUpgradeFileVersion(expVersion, "asic");
        if(expVersion.indexOf("_") >= 0) {
            expVersion = expVersion.left(expVersion.indexOf("_"));
        }
    } else {
        expVersion = asicVersion;
    }

    QString resultStr = "NG";
    if (expVersion.contains(asicVersion)) {
        resultStr = "PASS";
    } else {
        if (isOnlineMode()) {
            m_errorCode = -10;
            addLog("版本比对失败，请人工比对确认", -1);
            return;
        }
    }

    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "AISC版本号" << expVersion <<
                         asicVersion << useTime << resultStr);
    m_startTime = m_endTime;

    MesCheckItem item;
    item.sItem = "asic_version";
    item.sValue = asicVersion;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);

    addLog("ASIC版本号测试完成");
    m_reportList.append(asicVersion);
}

void AutoProcessCheckSensor::workCheckAgeFlag()
{
    if(m_errorCode != 0) return;

    QString deviceName;
    m_asic->GetDeviceName(deviceName);
    if (deviceName == "XD01A") {
        addLog("开始老化标记位错误码检测...");
        int ageErrorCode = 0;
        if (!m_asic->GetAgeErrorCode(ageErrorCode)) {
            m_errorCode = -105;
            logFail("检测老化标记位失败!!!");
        } else {
            logNormal("检测老化标记位：" + QString::number(ageErrorCode));
            if (ageErrorCode != 0) {
                m_errorCode = -105;
                logFail("老化标记位包含错误码，失败!!!");
            }
        }

        MesCheckItem item;
        item.sItem = "age_flag";
        item.sValue = QString(ageErrorCode);
        item.sResult = ageErrorCode == 0 ? "PASS" : "NG";
        m_mesPackage.append(item);

        addLog("老化标记位测试完成");
        m_reportList.append(QString(ageErrorCode));
    } else {
        addLog("非XD01A项目暂不支持老化错误码检测...");
    }
}

void AutoProcessCheckSensor::workReset()
{
    if(m_errorCode != 0) return;

    addLog("恢复出厂设置");

    m_asic->Reset();

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "恢复出厂设置" << "-" << "-" << useTime << "PASS");
    m_startTime = m_endTime;

    MesCheckItem item;
    item.sItem = "reset_factory";
    item.sValue = "PASS";
    item.sResult = "PASS";
    item.tCheckTm = useTime;
    m_mesPackage.append(item);

    addLog("恢复出厂设置完成");
}


void AutoProcessCheckSensor::workStepCheckShutter()
{
    if(m_errorCode != 0) return;

    addLog("开始快门检测...");

    bool result = false;
    int checkCount = 10;

    for (int foo = 0; foo < 3; foo++) {
        if (m_asic->changeSensorFormat(3)) // Y16
        {
            break;
        }
        if (foo == 2) {
            addLog(QString("Y16模式切换失败").arg(10 - checkCount + 1));
            return;
        }
    }
    QThread::msleep(1000);
    int nOkTimes = 0;
    while(checkCount--) {
        if(checkShutter()) {
            nOkTimes++;
            addLog(QString("第%1轮快门检测 PASS").arg(10 - checkCount + 1));
        }
    }

    if (nOkTimes < 9) {
        addLog("快门检测 FAIL");
        result = false;
    } else {
        result = true;
    }

    QString resultStr = result ? "PASS" : "NG";
    if (result) {
        resultStr = "PASS";
        addLog("快门检测OK");
    } else {
        m_errorCode = -12;
        resultStr = "NG";
        addLog("快门检测NG", -1);
    }
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "快门检测" << "-" << "-" << useTime << resultStr);
    m_startTime = m_endTime;

    MesCheckItem item;
    item.sItem = "shutter_check";
    item.sValue = resultStr;
    item.sResult = resultStr;
    m_mesPackage.append(item);

    addLog("快门检测完成");
    m_reportList.append(resultStr);
}

bool AutoProcessCheckSensor::checkShutter()
{
    QByteArray Y16_1, Y16_2, Y16_3;
    // 1.
    m_asic->ChangeWorkMode(false);
    QThread::msleep(800);
    m_videoQueue.clear();
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    addLog("获取快门检测Y16图片1!!");
    Y16_1 = m_videoQueue.take();
    if(!dutSaveImage("快门检测1", Y16_1)) {
        return false;
    }

    addLog("关闭快门，获取快门检测Y16图片2!!");
    m_asic->ChangeWorkMode(true);
    QThread::msleep(800);
    m_videoQueue.clear();
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    Y16_2 = m_videoQueue.take();
    if(!dutSaveImage("快门检测2", Y16_2)) {
        return false;
    }

    addLog("开启快门，获取快门检测Y16图片3!!");
    m_asic->ChangeWorkMode(false);
    QThread::msleep(800);
    m_videoQueue.clear();
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    Y16_3 = m_videoQueue.take();
    if(!dutSaveImage("快门检测3", Y16_3)) {
        return false;
    }

    unsigned short* img_y1 = new unsigned short[WIDTH * HEIGHT];
    unsigned short* img_y2 = new unsigned short[WIDTH * HEIGHT];
    unsigned short* img_y3 = new unsigned short[WIDTH * HEIGHT];
    memset(img_y1, 0, sizeof(unsigned short) * WIDTH * HEIGHT);
    memset(img_y2, 0, sizeof(unsigned short) * WIDTH * HEIGHT);
    memset(img_y3, 0, sizeof(unsigned short) * WIDTH * HEIGHT);

    memcpy(img_y1, Y16_1.data(), WIDTH * HEIGHT * 2);
    memcpy(img_y2, Y16_2.data(), WIDTH * HEIGHT * 2);
    memcpy(img_y3, Y16_3.data(), WIDTH * HEIGHT * 2);



    Frock_cal algCheck;
    int high_val = 0, low_val = 0;
    algCheck.Check_uniform(img_y1, WIDTH, HEIGHT, high_val, low_val);
    int rang1 = high_val - low_val;
    algCheck.Check_uniform(img_y2, WIDTH, HEIGHT, high_val, low_val);
    int rang2 = high_val - low_val;
    algCheck.Check_uniform(img_y3, WIDTH, HEIGHT, high_val, low_val);
    int rang3 = high_val - low_val;

    int avg_val = 0;
    algCheck.Check_SFFCEx(img_y2, img_y1, WIDTH, HEIGHT, avg_val);
    int avg_diff2 = avg_val;
    algCheck.Check_SFFCEx(img_y3, img_y1, WIDTH, HEIGHT, avg_val);
    int avg_diff3 = avg_val;

    delete []img_y1;
    delete []img_y2;
    delete []img_y3;
    img_y1 = nullptr;
    img_y2 = nullptr;
    img_y3 = nullptr;

    bool bRst = true;
    bRst &= (rang1 > 0 && rang1 <= 150);
    bRst &= (rang2 > 0 && rang2 <= 150);
    bRst &= (rang3 > 0 && rang3 <= 150);
    bRst &= (avg_diff2 >= 800);
    bRst &= (avg_diff3 > 0 && avg_diff3 <= 200);

    QString log;
    log.sprintf("快门检测结果，rang1: %d, rang2: %d, rang3: %d, avg_diff2: %d, avg_diff3: %d",
                rang1, rang2, rang3, avg_diff2, avg_diff3);
    addLog(log);

    return bRst;
}

void AutoProcessCheckSensor::workCheckVideoAuto()
{
    if(m_errorCode != 0) return;

    addLog("开始图像检测，摄像头对准黑体检测图像!!");
    QString result = msgBox("对准黑体检测图像!!", 4);

    m_asic->changeSensorFormat(3);
    QThread::sleep(2);
    m_asic->ShutterComp();
    m_videoQueue.clear();
    // 等待出图
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    QByteArray imgY16 = m_videoQueue.take();
    if (!dutSaveImage("自动检测图像Y16数据", imgY16)) {
        addLog("自动检测Y16图像保存失败", -1);
        m_errorCode = -13;
        return;
    }

    unsigned char* y8 = new unsigned char[WIDTH * HEIGHT];
    memset (y8, 0, WIDTH * HEIGHT);
    getBit8FromY16(imgY16, y8);

    imgCalY8 imgY8;
    addLog("横线检测。。。");
    std::vector<badLineData> vectorRst;
    imgY8.checkHorBadLine(y8, vectorRst, WIDTH, HEIGHT);
    foreach(badLineData line, vectorRst) {
        QString log = QString("Hor line: %1, starIndex: %2, endIndex: %3").arg(line.line).arg(line.startIndex).arg(line.endIndex);
        addLog(log);
    }

    addLog("竖线检测。。。");
    vectorRst.clear();
    imgY8.checkVerBadLine(y8, vectorRst, WIDTH, HEIGHT);
    foreach(badLineData line, vectorRst) {
        QString log = QString("Ver line: %1, starIndex: %2, endIndex: %3").arg(line.line).arg(line.startIndex).arg(line.endIndex);
        addLog(log);
    }

    addLog("灰尘检测。。。");
    std::vector<tRect> outRect;
    imgY8.checkDirtHalcon(y8, outRect, WIDTH, HEIGHT);
    foreach(tRect rect, outRect) {
        QString log = QString("Dirty (%1, %2), width: %3, height: %4").arg(rect.x).arg(rect.y).arg(rect.w).arg(rect.h);
        addLog(log);
    }

//    imgY8.checkDirtNew(y8, outRect, WIDTH, HEIGHT);
//    foreach(tRect rect, outRect) {
//        QString log = QString("Dirty (%1, %2), width: %3, height: %4").arg(rect.x).arg(rect.y).arg(rect.w).arg(rect.h);
//        addLog(log);
//    }
}

void AutoProcessCheckSensor::workCheckVideoManual()
{
    if(m_errorCode != 0) return;

    m_asic->changeSensorFormat(1);
    QThread::sleep(2);

    m_videoQueue.clear();

    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    addLog("开始图像检测，摄像头对准黑体检测图像!!");

    m_asic->ShutterComp();
    QString result = msgBox("对准黑体检测, 图像无坏线点击\'成功\'!!", 2);
    addLog("图像检测坏线结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.take();
        if (!dutSaveImage("ERR_坏线图片", imgY8)) {
            addLog("ERR_坏线保存失败", -1);
            m_errorCode = -13;
            return;
        }
    }


    m_asic->ShutterComp();
    result = msgBox("对准黑体检测, 图像无灰尘点击\'成功\'!!", 2);
    addLog("图像检测灰尘结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.take();
        if (!dutSaveImage("ERR_灰尘图片", imgY8)) {
            addLog("ERR_灰尘图片保存失败", -1);
            m_errorCode = -13;
            return;
        }
    }



    m_asic->ShutterComp();
    result = msgBox("对准黑体检测, 图像无其他问题点击\'成功\'!!", 2);
    addLog("图像检测结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.take();
        if (!dutSaveImage("ERR_其他问题图片", imgY8)) {
            addLog("ERR_其他问题图片保存失败", -1);
            m_errorCode = -13;
            return;
        }
    }
}

void AutoProcessCheckSensor::checkStepCheckVideo(bool result)
{
    if(result) {
        m_waitResult = 1;
    } else {
        m_waitResult = 2;
    }
}

void AutoProcessCheckSensor::workStepCheckPower()
{
    if(m_errorCode != 0) return;

    addLog("开始功率检测...");
	
    float expectValueMin = 0.8;
    float expectValueMax = 2;
    float current = .0f;

    // todo 要改板，暂时用控制盒电流

    if (m_powerSupply == PowerSupply_Sensor) {
        current = m_mcuSerial->getSensorCurrent();
        m_dataVoltage = m_mcuSerial->getSensorVoltage();
    } else {
        current = m_mcuSerial->getArmCurrent();
        m_dataVoltage = m_mcuSerial->getArmVoltage();
    }
    float resultValue = current * m_dataVoltage;

    QString execptValue = QString("%1 ~ %2").arg(QString::number(expectValueMin)).arg(QString::number(expectValueMax));

    logNormal(QString("功率范围 %1").arg(execptValue));
    QString resultStr;
    if(resultValue > expectValueMin && resultValue < expectValueMax) {
        resultStr = "PASS";
    } else {
        resultStr = "NG";
        m_errorCode = -14;
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "功率检测" << execptValue << QString::number(resultValue) << useTime << resultStr);
    m_startTime = m_endTime;;

    MesCheckItem item;
    item.sItem = "power_check";
    item.sValue = resultValue;
    item.sResult = resultStr;
    item.sMin = QString::number(expectValueMin);
    item.sMax = QString::number(expectValueMax);
    item.tCheckTm = QString::number(m_endTime - m_startTime);
    m_mesPackage.append(item);

    addLog(QString("功率检测完成，当前电流%1, 功率%2").arg(QString::number(current)).arg(QString::number(resultValue)));
    m_reportList.append(QString::number(resultValue));
}


bool AutoProcessCheckSensor::widePowerTest(float targetVol, float &power)
{
    float current = .0f, voltage = .0f;
    for (int foo = 0; foo < 8; foo++) {

        if (m_powerSupply == PowerSupply_Sensor) {
            m_mcuSerial->controlSensorPower(targetVol);
            QThread::msleep(300);
            current = m_mcuSerial->getSensorCurrent();
            voltage = m_mcuSerial->getSensorVoltage();
        } else {
            m_mcuSerial->controlArmPower(targetVol);
            QThread::msleep(200);
            current = m_mcuSerial->getArmCurrent();
            voltage = m_mcuSerial->getArmVoltage();
        }

//        if (!m_mcuSerial->controlArmPower(targetVol)) {
//            addLog(QString("等待%1V电压上电完成!").arg(targetVol));
//            QThread::msleep(200);
//            continue;
//        }
//        current = m_mcuSerial->getArmCurrent();
//        voltage = m_mcuSerial->getArmVoltage();
        addLog(QString("当前读取到电流%1, 电压%2").arg(QString::number(current)).arg(QString::number(voltage)));
        if ((voltage < (targetVol - 0.5)) || (voltage > (targetVol + 0.5))) {
            addLog(QString("等待 %1 V电压上电完成，当前电压 %2 V!").arg(targetVol).arg(voltage));
            QThread::msleep(200);
            continue;
        }
        // addLog(QString("%1V电压测试正常, 电压为: %2").arg(targetVol).arg(voltage));

        power = current * voltage;
        if ((power <= 0) || (power > 2)) { //
            addLog(QString("%1V功率测试失败，功率%2超门限(0, 2)，重试!").arg(targetVol).arg(power));
            QThread::msleep(200);
            continue;
        }
        return true;
    }

    return false;
}

void AutoProcessCheckSensor::workStepVoltageWide()
{
    if(m_errorCode != 0) return;

    QString proj = projectName();
    // XD01不测试宽电压
    if (proj.contains("XD01A")) return;

    bool result = true;
    int maxVol = 16;
    addLog("开始宽电压测试...");
    // float current = .0f, voltage = .0f;
    float power = .0f;
    for(int i = 9; i <= maxVol; i++) {
        addLog(QString("开始%1V电压测试...").arg(i));
/*        if (m_powerSupply == PowerSupply_Sensor) {
            m_mcuSerial->controlSensorPower(i);
            QThread::msleep(300);
            current = m_mcuSerial->getSensorCurrent();
            voltage = m_mcuSerial->getSensorVoltage();
        } else {
            m_mcuSerial->controlArmPower(i);
            QThread::msleep(200);
            current = m_mcuSerial->getArmCurrent();
            voltage = m_mcuSerial->getArmVoltage();
        }

        power = current * voltage;

        if (power > 2 || power < 0) { //
            result = false;
            m_errorCode = -17;
            addLog(QString("%1V电压测试失败, 功率超门限%2 (0, 2)").arg(QString::number(i).arg(power)), -1);
            break;
        }
        */
        int iviFlag = m_iviFrameCount;
        if (m_powerSupply == PowerSupply_Sensor) {
            m_mcuSerial->controlSensorPower(i);
        } else {
            m_mcuSerial->controlArmPower(i);
        }

        if (!widePowerTest(i, power)) {
            m_errorCode = -19;
            m_errMsg = QString("%1V电压测试，功率失败").arg(QString::number(i));
            addLog(m_errMsg, -1);
            break;
        }
        addLog(QString("%1V电压测试正常, 功率为: %2").arg(QString::number(i)).arg(power));

        // 额外加入图片出图是否正常的判断...
        // QThread::msleep(300);
        int iviOffset = m_iviFrameCount - iviFlag;
        if(iviOffset < 2) {
            m_errorCode = -17;
            power = .0f;
            addLog(QString("%1V电压测试失败, 出图失败").arg(QString::number(i)), -1);
            result = false;
            break;
        }
    }


    QString resultStr = result ? "PASS" : "FAIL";
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    updateTableResult(QStringList() << "宽电压测试" << QString("0 ~ 2W").arg(maxVol) <<
                         QString("%1W").arg(power) << QString::number(m_endTime - m_startTime) << resultStr);
    m_startTime = m_endTime;

    addLog("宽电压测试完成");
    m_reportList.append(resultStr);
}

void AutoProcessCheckSensor::workStepWindowHot()
{
    if(m_errorCode != 0) return;

    addLog("开始窗口加热检测...");

    float realElecUpValue;
    int restartTimes = 3;
    bool result = false;
    int continueTimes = 0;
    while(restartTimes-- > 0) {
        // todo 要改板，暂时用控制盒电流
        logAddNormalLog("------------------------------");
        m_asic->StartWindowHot();
        QThread::msleep(2000);

        QElapsedTimer timer;
        timer.start();
        while(timer.elapsed() < 2500) {
            // todo 要改板，暂时用控制盒电流

            float current = .0f, voltage = .0f, power = .0f;

            if (m_powerSupply == PowerSupply_Sensor) {
                current = m_mcuSerial->getSensorCurrent();
                voltage = m_mcuSerial->getSensorVoltage();
            } else {
                current = m_mcuSerial->getArmCurrent();
                voltage = m_mcuSerial->getArmVoltage();
            }

            power = current * voltage;
            qInfo() << "power is " << power;
            QString log;
            log = QString("%1ms: 窗口加热测试, 电压: %2, 电流: %3, 功率: %4").arg(timer.elapsed()).arg(voltage).arg(current).arg(power);
            logAddNormalLog(log);

            if (power > 6.048 || power < 3.648) { // 门限合入
                continueTimes = 0;
                logAddFailLog("窗口加热测试失败，重试! " + QString(" 功率 %1 W").arg(QString::number(power, 'f', 2)));
                // qDebug() << "窗口加热检测: " << QString("3.648 ~ 6.048") << QString(" 功率 %1 W").arg(QString::number(power, 'f', 2));
                break;
            } else {
                QString useTime = QString::number(m_endTime - m_startTime);
                updateTableResult(QStringList() << "窗口加热检测"
                                     << QString("3.648 ~ 6.048") << QString("功率 %1 W").arg(QString::number(power, 'f', 2))
                                     << useTime << "PASS");
                continueTimes++;

            }

            if (continueTimes >= m_timesCheckHot) {
                result = true;
                break;
            }

            QThread::msleep(200);
            m_startTime = m_endTime;
        }

        if (result) break;
    }

    QThread::msleep(3000); // 等窗口加热结束
    QString resultStr = result ? "PASS" : "NG";
    if(!result) {
        m_errorCode = -11;
        logAddFailLog("窗口加热测试失败!");
        QString useTime = QString::number(m_endTime - m_startTime);
        updateTableResult(QStringList() << "窗口加热检测"
                             << QString("3.648 ~ 6.048") << "NG"
                             << useTime << resultStr);
    }

    MesCheckItem item;
    item.sItem = "windows_hot";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = QString::number(m_endTime - m_startTime);
    m_mesPackage.append(item);

    addLog("窗口加热完成");
    m_reportList.append(QString::number(realElecUpValue));
}

void AutoProcessCheckSensor::workPrinter()
{
    if(m_errorCode != 0) return ;
    if(isGoldenMode()) {
        return;
    }
    addLog("打开打印机");
    CMyPrinter printer;
    QString printerName = printer.defalutPrinterName();
    if(printerName.isEmpty()) {
        addLog("获取打印机失败", -1);
        m_errorCode = -8;
        return;
    }

    QString modulePath;
    if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
//        modulePath = QCoreApplication::applicationDirPath() + "/res/XD02A摄像头标签.btw";
        modulePath = GETCONFIG("打印模板文件");
    } else {
        // MesCom::instance()->getproductPrintTemplate(modulePath);
        modulePath = GETCONFIG("打印模板文件");
    }
    qDebug() << "file path is " << modulePath;

    if(!printer.load(printer.defalutPrinterName(), modulePath, 1, 1)) {
        addLog("加载打印机模板失败", -1);
        m_errorCode = -8;
        return;
    }
    QStringList itemList = printer.items();
    qDebug() << "-------------------  get item is " << itemList;

    m_snPrint = m_sn;
    for(int i=0; i<itemList.size(); i++) {
        if(itemList[i].contains("ID017")) {
            QString sn = m_sn;
            sn = sn.replace("*", "");
            sn = sn.replace("*", "");
            printer.setItem("ID017", sn.mid(5, 12));
        }
        if(itemList[i].contains("ID018")) {
            QString sn = m_sn;
            sn = sn.replace("**", "");
            sn += "*DW3602231-E2501*";
            printer.setItem("ID018", sn);
            m_snPrint = sn;
        }
        if(itemList[i].contains("ID007")) {
            printer.setItem("ID007", m_sn);
        }
        if(itemList[i].contains("ID001")) {
            QString data= "20" + m_sn.mid(7, 2) + ".";
            data += m_sn.mid(9, 2) + ".";
            data += m_sn.mid(11, 2);
            printer.setItem("ID001", data);
        }
        if(itemList[i].contains("ID008")) {
            int number = Util::stringConvert34to10(m_sn.mid(15, 4));
            QString numberCode = QString("%1").arg(number, 7, 10, QLatin1Char('0'));
            printer.setItem("ID008", numberCode);
            qDebug() << "number is "  << numberCode << "-----------------";
        }
        if(itemList[i].contains("ID002")) {
            printer.setItem("ID002", m_sn);
        }
        if(itemList[i].contains("ID009")) {
            QString dataStr = m_sn.mid(11, 4);
            QString year = dataStr.left(1);
            if(year == "N") {
                year = "2022";
            } else if(year == "P") {
                year = "2023";
            } else if(year == "R") {
                year = "2024";
            } else if(year == "S") {
                year = "2025";
            } else if(year == "T") {
                year = "2026";
            }
            int month = Util::stringConvert34to10(dataStr.mid(1, 1));
            int day = dataStr.right(2).toInt();
            qDebug() << "data is "  << year << month << day << "-----------------";
            printer.setItem("ID009", QString("%1/%2/%3").arg(year).arg(QString::number(month), 2, QLatin1Char('0')).arg(QString::number(day), 2, QLatin1Char('0')));
        }
    }

    qDebug() << printer.items();

    addLog("开始打印");
    printer.print();
    addLog("打印完成");
}

void AutoProcessCheckSensor::workCompareSn()
{
    if(m_errorCode != 0) return ;
    if(isGoldenMode()) {
        return;
    }
    if(m_errorCode == 0) {
        addLog("请扫码比较SN");
        QString sn = msgBox("请扫码比较SN");
        addLog("请扫码比较SN: " + sn);
        if(sn != m_snPrint) {
            m_errMsg = "SN对比失败";
            m_errorCode = -9;
        }
    }

    QString resultStr = m_errorCode == 0 ? "PASS" : "NG";
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "打印标贴"
                         << "-" << resultStr
                         << useTime << resultStr);
    m_startTime = m_endTime;

    addLog("SN比较完成");

    MesCheckItem item;
    item.sItem = "printer";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = QString::number(m_endTime - m_startTime);
    m_mesPackage.append(item);
}

void AutoProcessCheckSensor::workCheckCalibIn()
{
    if(m_errorCode != 0) return ;

    QString param = m_asic->GetParamIn();
    QStringList paramList = param.split(",");
    QString resultStr;
    if(param.isEmpty()) {
        resultStr = "NG";
        m_errorCode = -22;
    } else {
        if(paramList.size() != 16 || paramList[0].toFloat() < 730 || paramList[0].toFloat() > 790) {
            resultStr = "NG";
            m_errMsg = "内参测试失败";
            m_errorCode = -22;
        } else {
            resultStr = "PASS";
        }
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "内参检测" << "-" << param << useTime << resultStr);
    m_startTime = m_endTime;

    addLog("内参校验完成");

    MesCheckItem item;
    item.sItem = "check_cailb_param";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = QString::number(m_endTime - m_startTime);
    m_mesPackage.append(item);

    m_reportList.append(param);
}

void AutoProcessCheckSensor::reportResult()
{
    if(m_errorCode == 0) {
        addLog("工作成功 SN: " + m_sn, 1);
        emit resultShowPass();
    } else {
        QString log = "工作失败 SN: " + m_sn + ",错误码 " + QString::number(m_errorCode) + "," + m_errMsg;
        addLog(log, -1);
        showFail(log);
    }

    QString path = m_logPath + (m_errorCode == 0 ? "PASS_" : "FAIL_");
    logSaveTxt(path + m_sn + ".txt");
    tableSaveCsv(path + m_sn + ".csv");

    // saveCsvLog();
}

void AutoProcessCheckSensor::saveCsvLog()
{
    CsvLogger logger;
    QString fileName = QDate::currentDate().toString("yyyyMMdd") + ".csv";
    QString path = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd") + "/";
    logger.setCsvLogPath(path  + fileName);
}

void AutoProcessCheckSensor::EvaluateHorStripe()
{
    if(m_errorCode != 0) return;

    addLog("开始横纹检测，摄像头对准黑体检测图像!!");
    QString result = msgBox("对准黑体检测图像!!",2);
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    if(result != "PASS") {
        m_errorCode = -13;
        logFail("摄像头对准黑体失败");
        return;
    }

    float reasultValue=0.0;
    float expectValueMin=0.0;
    float expectValueMax=0.0;
    m_videoQueue.clear();
    m_start = 1;
    uint16_t * input_data = new uint16_t[640 * 512];

    for(int i=0;i<100;i++)
    {
       QByteArray arr = m_videoQueue.take();
       memset(input_data,0,640*512*2);
       uint8_t *pData = (uint8_t *)arr.data();
       for(int i=0;i<640*512;i++)
       {
           input_data[i]=pData[2*i+1];
       }
       qInfo() <<"recv "<< i <<" "<< arr.left(4).toHex();

       double value1[1]={0.0};
       double value2[1]={0.0};
       double value3[1]={0.0};
       imagecheck Imagecheck;
       Imagecheck.Evaluate_HorStripe(value1, value2, value3, input_data, 640, 512);
       reasultValue += value1[0];
       expectValueMin += value3[0];
       expectValueMax += value2[0];

       addLog(QString("图像检测完成，当前value1=%1, value2=%2,value3=%3").arg(QString::number(value1[0])).arg(QString::number(value2[0])).arg(QString::number(value3[0])));
    }
    delete []input_data;
    m_start = 0;
    m_videoQueue.clear();
    reasultValue = reasultValue/100.0;
    expectValueMin = expectValueMin/100.0;
    expectValueMax = expectValueMax/100.0;
    MesCheckItem item;
    item.sItem = "横纹检测";
    double value= reasultValue * 0.2 + (abs(expectValueMin) + abs(expectValueMax)) * 0.8 / 2;

    item.sValue = value;
    QString spec;
    spec = GETCONFIG("Spec");
    double specvalue=spec.toFloat();
    item.sExp = spec;
    if(value<=specvalue)
    {
       item.sResult = MESPASS;
    }
    else
    {
        item.sResult = MESFAILED;
        m_errorCode = -13;
        logFail("横纹检测失败");
    }
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << item.sItem<< item.sExp <<QString::number(value) << useTime << item.sResult);

    item.tCheckTm = useTime;
    m_mesPackage.append(item);
    m_reportList.append(item.sValue);
    m_startTime = m_endTime;
}

void AutoProcessCheckSensor::addLog(QString log, const int result)
{
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
//        showPass(log);
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

bool AutoProcessCheckSensor::dutSaveImage(QString name, QByteArray data)
{
    // 给每张图片一个序号，方便排序
    QString dirName = name.replace("*", "_");
    QString jpgPicName = QString("%1_%2").arg(m_seqImage++).arg(dirName);
    QByteArray btArray = data;
//    if (data.isEmpty()) {
//        if (!getImageData(btArray)) {
//            setErrString(-4, "获取图片数据失败");
//            return false;
//        }
//    } else {
//        btArray = data;
//    }
//    if (btArray.size()  == 0) {
//        writeLog("save data length: " + QString::number(btArray.size()), -1);
//        setErrString(-3, "保存图片失败，图片数据为空，检查产品是否连接OK");
//        return false;
//    }

    QString path = m_logPath + jpgPicName;

    VideoFormatInfo vinfo;
    VideoInfo::getInstance()->getVideoInfo(0, vinfo);

    unsigned char* m_frameData = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_rgbData = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_y8Data = new unsigned char[WIDTH * HEIGHT * 4];
    memset (m_frameData, 0, WIDTH * HEIGHT * 4);
    memset (m_rgbData, 0, WIDTH * HEIGHT * 4);
    memset (m_y8Data, 0, WIDTH * HEIGHT * 4);

    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(0, videoInfo);
    memcpy(m_frameData, btArray.mid(videoInfo.nvsLen, videoInfo.frameLen), videoInfo.frameLen);
    if(videoInfo.format == VIDEO_FORMAT_NV21) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_NV21", mSlotId - 1);
        Yuv2Rgb::YuvToRgbNV12(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_UYUV) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_UYUV", mSlotId - 1);
        Yuv2Rgb::YuvToRgbUYVY(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_YUV420) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_YUV420", mSlotId - 1);
        Yuv2Rgb::YuvToRgb420(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_Y16) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_Y16", mSlotId - 1);
        MappingNew map;
        map.DRC_Mix(m_y8Data, (short*)m_frameData, videoInfo.width, videoInfo.height, 255, 80, 128, 200, 90, 50, 5, 5, 1);
        map.y8ToRgb(m_y8Data, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_X16) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_X16", mSlotId - 1);
        Mapping map;
        map.Data16ToRGB24((short*)m_frameData, m_rgbData, videoInfo.width*videoInfo.height, 0);
    } else if(videoInfo.format == VIDEO_FORMAT_Y16_Y8) {
        //emit mProduce->logNormal("Current IS VIDEO_FORMAT_Y16_Y8", mSlotId - 1);
        Yuv2Rgb::YuvToRgbUYVY(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    }

    QImage image(m_rgbData, videoInfo.width, videoInfo.height, QImage::Format_RGB888);
    if (!image.isNull()) {
        // writeLog("image is ok");
        if (image.save(path + ".jpg", "jpg")) {

            QFile file(path + ".raw");
            if (file.open(QIODevice::WriteOnly)) {
                file.write(btArray.mid(640 * 4, 640 * 512 * 2));
                file.close();

                delete []m_rgbData;
                delete []m_frameData;
                delete []m_y8Data;
                return true;
            } else {
                addLog("保存原始图像数据失败 " + path, -1);
            }
        }
    }
    addLog("image save to " + path + " fail");
    delete []m_rgbData;
    delete []m_frameData;
    delete []m_y8Data;
    return false;
}

void AutoProcessCheckSensor::getBit8FromY16(QByteArray src, unsigned char *&dest)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);
    unsigned char* m_frameData = new unsigned char[info.width * info.height * 2];
    unsigned char* m_y8Data = new unsigned char[info.width * info.height * 4];
    unsigned char* m_rgbData = new unsigned char[info.width * info.height * 4];
    memset (m_frameData, 0, sizeof (m_frameData));
    memcpy(m_frameData, src.data(), info.width * info.height * 2);
    MappingNew map;
    map.DRC_Mix(m_y8Data, (short*)m_frameData, info.width, info.height, 255, 80, 128, 200, 90, 50, 5, 5, 1);
    map.y8ToRgb(m_y8Data, m_rgbData, info.width, info.height);

    for (int foo = 0; foo < info.width * info.height; foo++) {
        dest[foo] = m_rgbData[3 * foo];
    }

    delete []m_frameData;
    delete []m_y8Data;
    delete []m_rgbData;
}

void AutoProcessCheckSensor::mesUpload()
{
    for (int i=0; i<m_mesPackage.size(); i++ ) {
        MesCom::instance()->addItemResultEx(m_mesPackage[i]);
    }
    if(m_errorCode != 0) {
        m_bResultCode = false;
    } else {
        m_bResultCode = true;
    }
}

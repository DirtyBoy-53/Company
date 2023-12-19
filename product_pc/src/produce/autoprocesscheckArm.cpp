#include "autoprocesscheckArm.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include "configinfo.h"
#include "messagedispacher.h"

#include "inputmessagebox.h"
#include "videoinfo.h"
#include "zprinter/CMyPrinter.h"
#include "basecomponent.h"
#include "util.h"

#include "checkbox.h"

const int g_powerOnVol = 12;

AutoProcessCheckArm::AutoProcessCheckArm()
{
    initStateMachine();

    m_handlePlc = new CSerialDirect;
    m_handleScanner = new CSerialDirect;
    m_handleProduct = new CSerialDirect;
    m_mcuSerial = new McuSerialManager();
    m_dataImg = nullptr;
}

AutoProcessCheckArm::~AutoProcessCheckArm()
{
    if (m_dataImg) {
        delete []m_dataImg;
        m_dataImg = nullptr;
    }
}


bool AutoProcessCheckArm::checkComport(CSerialDirect* handle, QString port)
{
    if(handle == nullptr) {
        return false;
    }

    QString mcuCom;
    ConfigInfo::getInstance()->getValueString("CB1004", "PlcCom", mcuCom);
    handle->close();
    int ret = handle->open(port);
    if(ret) {
        handle->close();
        return true;
    } else {
        addLog("串口异常, 请检查接线", -1);
    }

    return false;
}

bool AutoProcessCheckArm::checkMcuPort(QString port)
{
    if (!m_mcuSerial) return false;

    m_mcuSerial->closeSerial();
    int ret = m_mcuSerial->openSerial(port);
    if(ret) {
        m_mcuSerial->closeSerial();
        return true;
    }

    return false;
}

bool AutoProcessCheckArm::getAutoBarcode(QString &sn)
{
    if (!m_handleScanner) {
        return false;
    }

    qDebug() << "open scanner";
    m_handleScanner->close();
    if (!m_handleScanner->open(m_portScanner, 115200, false)) {
        return false;
    }

    char szBarcode[32] = {0};
    int nLen = 0;
    char* pBarcode = (char*)szBarcode;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        m_handleScanner->write((unsigned char*)"<a>\r\n", 5);
        if (m_handleScanner->get(nullptr, 0, nullptr, 2, pBarcode, nLen, 3000)) {
            sn = QString::fromLatin1(pBarcode, nLen);
            qDebug() << "size: " << nLen << " code: " << pBarcode;
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

QString AutoProcessCheckArm::scanAutoBarcode()
{
    qDebug() << "waitting press";
    if (!watiForButtonPress(90000)) {
        return "";
    }

    qDebug() << "get barcode";
    QString sn;
    getAutoBarcode(sn);
    return sn;
}

bool AutoProcessCheckArm::writePlc(unsigned char cmd[], int len)
{
    if (!m_handlePlc) {
        return false;
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 8000) {
        m_handlePlc->write(cmd, len);
        QThread::msleep(200);
        if (m_handlePlc->find(cmd, len, 2000)) {
            return true;
        }
        QThread::msleep(200);
    }

    return false;
}

bool AutoProcessCheckArm::openFixture(int timeout)
{
    addLog("请拉出托盘，取出产品!", m_errorCode == 0 ? 1 : -1);
//    if (!watiForButtonPress(timeout)) {
//        return false;
//    }

    unsigned char cmdStop1[] = {0x01, 0x05, 0x00, 0x02, 0xFF, 0x00, 0x2D, 0xFA};
    unsigned char cmdStop2[] = {0x01, 0x05, 0x00, 0x02, 0x00, 0x00, 0x6C, 0x0A};
    if (!writePlc(cmdStop1, LENGTH_8)) {
        return false;
    }

    if (!writePlc(cmdStop2, LENGTH_8)) {
        return false;
    }

    return true;
    // return waitForOpenFixture(timeout);
}

bool AutoProcessCheckArm::waitForOpenFixture(int timeout)
{
    if (!m_mcuSerial) return false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        if (!m_mcuSerial->pressed()) {
            return true;
        }
        QThread::msleep(200);
    }

    return false;
}

bool AutoProcessCheckArm::waitForCloseFixture(int timeout)
{
    if (!m_mcuSerial) return false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        if (m_mcuSerial->pressed()) {
            return true;
        }
        QThread::msleep(200);
    }
    logNormal(QString("当前工装MCU版本位: %1").arg(m_mcuSerial->version(), 2, QLatin1Char('0')));
    return false;
}

bool AutoProcessCheckArm::watiForButtonPress(int timeout)
{
    if (!m_handlePlc) {
        return false;
    }

    unsigned char cmdDetect[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0xB9, 0xCA};
    unsigned char cmdFind[] = {0x01, 0x02, 0x01, 0x01, 0x60, 0x48};
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        m_handlePlc->write(cmdDetect, LENGTH_8);
        QThread::msleep(200);
        if (m_handlePlc->find(cmdFind, LENGTH_6, 2000)) {
            return true;
        }
        QThread::msleep(200);
    }

    return false;
}

QString AutoProcessCheckArm::waitForOkNgSignal(int timeout)
{
    if (!m_handlePlc) {
        return "";
    }

    unsigned char uRet[16] = {0x00};
    char *pRet = (char*)uRet;
    int nRet = 0;
    unsigned char cmdDetect[] = {0x01, 0x02, 0x00, 0x01, 0x00, 0x03, 0x69, 0xCB};
    // unsigned char cmdFind[] = {0x01, 0x02, 0x01, 0x01, 0x60, 0x48};
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        m_handlePlc->write(cmdDetect, LENGTH_8);
        QThread::msleep(200);

        if (m_handlePlc->get((char*)cmdDetect, 2, nullptr, 6, pRet, nRet, 1000)) {
            if (uRet[2] == 0x01 && uRet[3] == 0x02 && uRet[4] == 0x20) {
                return "OK";
            } else if (uRet[2] == 0x01 && uRet[3] == 0x04 && uRet[4] == 0xA0) {
                return "NG";
            }
        }
        //if (m_handlePlc->find(cmdFind, LENGTH_6, 2000)) {
        //    return true;
        //}
        QThread::msleep(200);
    }

    return "";
}

bool AutoProcessCheckArm::checkProductRs232()
{
    m_handleProduct->close();
    if (!m_handleProduct->open(m_portProduct, 921600, false)) {
        return false;
    }

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        m_handleProduct->write((unsigned char*)"\r", 1);
        if (m_handleProduct->find((unsigned char*)"login:", 6, 3000)) {
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

void AutoProcessCheckArm::slotConnectBtnClicked()
{
//    if (!checkAlgoBox()) {
//        m_errorCode = -10;
//        m_errMsg = "自动检测算法框失败";
//        addLog("自动检测算法框失败", -1);
//    }
//    return;
#if 1
    QString ip = ConfigInfo::getInstance()->getValueString("盒子IP");
    if(Util::testPing(ip)) {
        if(m_webSocketIvi == nullptr) {
            m_webSocketIvi = new WebSocketClientManager();
            m_webSocketIvi->connectToUrl(ip, 6000);
            connect(m_webSocketIvi,&WebSocketClientManager::signalRecvVideoFrame,this,&AutoProcessCheckArm::slotRecvVideoFrameWsIvi);
        }
    } else {
        addLog("网络异常, 请检查工装盒1 ip及网络连接", -1);
        return;
    }

    QString ip2;
    ConfigInfo::getInstance()->getValueString("CB1004", "盒子2IP", ip2);
    if(Util::testPing(ip2)) {
        if(m_webSocketLvds == nullptr) {
            m_webSocketLvds = new WebSocketClientManager();
            m_webSocketLvds->connectToUrl(ip2, 6000);
            connect(m_webSocketLvds,&WebSocketClientManager::signalRecvVideoFrame,this,&AutoProcessCheckArm::slotRecvVideoFrameWsLvds);
        }
    } else {
        addLog("网络异常, 请检查工装盒2 ip及网络连接", -1);
        return;
    }
#endif
    ConfigInfo::getInstance()->getValueString("CB1004", "McuCom", m_portMcu);
    ConfigInfo::getInstance()->getValueString("CB1004", "PlcCom", m_portPlc);
    ConfigInfo::getInstance()->getValueString("CB1004", "ScannerCom", m_portScanner);
    ConfigInfo::getInstance()->getValueString("CB1004", "ProductCom", m_portProduct);
    if (!checkMcuPort(m_portMcu)) {
        addLog("串口异常, 请检查接线", -1);
        return;
    }
    if (!checkComport(m_handlePlc, m_portPlc)) {
        addLog("工装串口异常，检查接线", -1);
        return;
    }
    if (!checkComport(m_handleScanner, m_portScanner)) {
        addLog("扫码枪异常，检查接线", -1);
        return;
    }
    if (!checkComport(m_handleProduct, m_portProduct)) {
        addLog("产品调试串口异常，检查接线", -1);
        return;
    }

    BaseProduce::slotConnectBtnClicked();
}

void AutoProcessCheckArm::slotRecvVideoFrameWsIvi(QByteArray frameData, FrameInfo_S frameInfo)
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

    m_imgBuffer = QByteArray(frameData);
    emit videoUpdate(0, frameData, frameInfo.uTimeTm);
    m_iviFrameCount++;
//    qDebug() << "ivi count " << m_iviFrameCount;
}

void AutoProcessCheckArm::slotRecvVideoFrameWsLvds(QByteArray frameData, FrameInfo_S frameInfo)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(1, info);
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
    VideoInfo::getInstance()->setVideoInfo(1, info);

    emit videoUpdate(1, frameData, frameInfo.uTimeTm);
    m_lvdsFrameCount++;
//    qDebug() << "lvds count " << m_lvdsFrameCount;
}

void AutoProcessCheckArm::slotsStoped()
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
        headList << "SN码" << "CAN检测" << "RS232检测" << "OEM软件版本号" << "OEM硬件版本号" << "lvds检测" << "功率检测" << "宽电压" << "错误码";
        m_csvLogger.addCsvTitle(headList);
    }

    m_reportList.append(QString::number(m_errorCode));
    m_csvLogger.addCsvLog(m_reportList);

    startProcess();
}

void AutoProcessCheckArm::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &AutoProcessCheckArm::slotsStoped);

    add(0, "check next", std::bind(&AutoProcessCheckArm::checkNext, this));
    add(1, "init", std::bind(&AutoProcessCheckArm::init, this));
    add(10, "judgeCustomLifeTime", std::bind(&AutoProcessCheckArm::jugdeCustomLifeTime, this, true));
    add(20, "check io", std::bind(&AutoProcessCheckArm::checkIoState, this));
    add(30, "power on", std::bind(&AutoProcessCheckArm::workStepPowerOn, this));
    add(40, "get sn", std::bind(&AutoProcessCheckArm::getSn, this));
    add(50, "enter mes", std::bind(&AutoProcessCheckArm::enterMes, this));
    add(60, "check can", std::bind(&AutoProcessCheckArm::workCheckCan, this));
    //#if 0
    //    add(110, "wide voltage", std::bind(&AutoProcessCheckArm::workStepVoltageWide, this));
    //    return;
    //#endif
    add(70, "check rs232", std::bind(&AutoProcessCheckArm::workCheckRs232, this)); // 修改
    add(80, "compare OEM sw version", std::bind(&AutoProcessCheckArm::workStepCompareVersion, this));
    add(81, "compare OEM hw version", std::bind(&AutoProcessCheckArm::workStepOemVersion, this));  
    add(90, "check lvds", std::bind(&AutoProcessCheckArm::workCheckLvds, this));
    add(100, "check power", std::bind(&AutoProcessCheckArm::workStepCheckPower, this));
    add(110, "wide voltage", std::bind(&AutoProcessCheckArm::workStepVoltageWide, this));
    add(111, "check usb", std::bind(&AutoProcessCheckArm::workStepCheckUsb, this));// 验证
    add(112, "check Algo", std::bind(&AutoProcessCheckArm::workStepCheckAlgo, this));
    add(120, "printer", std::bind(&AutoProcessCheckArm::workPrinter, this));
    add(130, "compare sn", std::bind(&AutoProcessCheckArm::workCompareSn, this));
    add(140, "upload mes", std::bind(&AutoProcessCheckArm::mesUpload, this));
    add(150, "out mes", std::bind(&AutoProcessCheckArm::outMes, this));
    add(160, "report", std::bind(&AutoProcessCheckArm::stopWork, this));
}

void AutoProcessCheckArm::checkNext()
{
    m_bResultCode = true;
	m_errorCode=0;
}

void AutoProcessCheckArm::init()
{
//    bool bUseLifeTime = false;
//    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "耗材寿命管控", bUseLifeTime);
//    if (bUseLifeTime) {
//        if (m_errorCode != 0) {
//            return;
//        }
//    }
    addLog("初始化连接");

    emit tableAddHeader(QStringList() << "检测项" << "检测期望值" << "当前值" << "检测时间(ms)" << "结果");
    emit tableClearData();
    emit logClearData();

    m_endTime = 0;

    m_skipMesUpload = false;
    m_lvdsFrameCount = 0;
    m_iviFrameCount = 0;
    m_sn = "";
    m_errorCode = 0;
    m_reportList.clear();

    addLog("开启MCU串口");
    m_mcuSerial->closeSerial();
    if(!m_mcuSerial->openSerial(m_portMcu)) {
        m_errorCode = -99;
        m_errMsg = "开启MCU串口失败";
        return;
    }
    m_ioState = m_mcuSerial->pressed();

    addLog("开启PLC串口");
    m_handlePlc->close();
    if (!m_handlePlc->open(m_portPlc, 9600, true)) {
        m_errorCode = -99;
        m_errMsg = "开启PLC串口失败";
        return;
    }
    m_handleProduct->close();
    m_handleScanner->close();
}

void AutoProcessCheckArm::updateTableResult(QStringList list)
{
    emit tableAddRowData(list);
    QString end = list[list.size() -1];
    if(end == "PASS") {
        emit tableUpdateTestStatus(0);
    } else {
        emit tableUpdateTestStatus(1);
    }
}

void AutoProcessCheckArm::stopWork()
{
    m_mcuSerial->changeMcuWorkMode(MCU_WORK_MODE_DEFAULT);
    m_mcuSerial->controlArmPower(0);
    if (!openFixture()) {
        logAddFailLog("夹具打开动作失败，请检查夹具情况!");
    }
    m_mcuSerial->closeSerial();
    m_endTime = 0;

    reportResult();

    msgBox("请更换产品执行下一轮测试", 4);
    stopProcess();
}

void AutoProcessCheckArm::checkIoState()
{
    if(m_errorCode != 0) return;

    addLog("请按住气缸闭合按钮，等待气缸闭合后，手动推进产品开始测试");
    if (!waitForCloseFixture(3600000)) {
        m_errorCode = -1;
        m_errMsg = "气缸闭合失败";
        addLog(m_errMsg, -1);
    }

//    while(!m_manualStoped) {
//        bool ioState = m_mcuSerial->pressed();
//        if(m_ioState != ioState) {
//            m_ioState = ioState;
//            if(m_ioState) {
//                break;
//            }
//        }
//        QThread::msleep(200);
//    }
//    if (m_manualStoped) {
//        addLog("手动停止了测试!", -1);
//        m_skipMesUpload = true;
//        m_errorCode = -12;
//    }
}

void AutoProcessCheckArm::workStepPowerOn()
{
    if(m_errorCode != 0) return;

    addLog("正在上电...");
    int vol = g_powerOnVol;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_mcuSerial->changeMcuWorkMode(MCU_WORK_MODE_CHECK);
    m_mcuSerial->controlArmPower(vol);

    QThread::sleep(10);

    m_endTime = QDateTime::currentMSecsSinceEpoch();

    float voltage = m_mcuSerial->getArmVoltage();
    if (voltage < (vol - 0.8) || voltage > (vol + 0.8)) {
        m_errorCode = -10;
        m_errMsg = "开机电压超门限，失败";
    }

    QString rstStr = (m_errorCode == 0) ? "PASS" : "FAIL";
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "供电测试" << QString("%1V").arg(QString::number(vol))
                      << QString("%1V").arg(QString::number(voltage)) << useTime << rstStr);

    m_startTime = m_endTime;

    addLog("设备启动");
    qInfo("power on!!");
}

bool AutoProcessCheckArm::widePowerTest(float targetVol, float &power)
{
    float current = .0f, voltage = .0f;
    for (int foo = 0; foo < 8; foo++) {
        if (!m_mcuSerial->controlArmPower(targetVol)) {
            addLog(QString("等待%1V电压上电完成!").arg(targetVol));
            QThread::msleep(200);
            continue;
        }
        current = m_mcuSerial->getArmCurrent();
        voltage = m_mcuSerial->getArmVoltage();
        addLog(QString("当前读取到电流%1, 电压%2").arg(QString::number(current)).arg(QString::number(voltage)));
        if ((voltage < (targetVol - 0.5)) || (voltage > (targetVol + 0.5))) {
            addLog(QString("等待 %1 V电压上电完成，当前电压 %2 V!").arg(targetVol).arg(voltage));
            QThread::msleep(200);
            continue;
        }
        // addLog(QString("%1V电压测试正常, 电压为: %2").arg(targetVol).arg(voltage));

        power = current * voltage;
        if ((power < 3) || (power > 9)) { //
            addLog(QString("%1V功率测试失败，功率%2超门限(3, 9)，重试!").arg(targetVol).arg(power));
            QThread::msleep(200);
            continue;
        }
        return true;
    }

    return false;
}


void AutoProcessCheckArm::workStepVoltageWide()
{
    if(m_errorCode != 0) return;

    bool result = false;
    addLog("开始宽电压测试...");
    float power = .0f;
    int maxPower = 32, minPower = 9;
    int lastVol = 0;
    for(int currentVol = minPower; currentVol <= maxPower; currentVol++) {
        addLog(QString("开始%1V电压测试...").arg(currentVol));
        lastVol = currentVol;
        m_mcuSerial->controlArmPower(currentVol);

        if (!widePowerTest(currentVol, power)) {
            m_errorCode = -19;
            m_errMsg = QString("%1V电压测试，功率失败").arg(QString::number(currentVol));
            addLog(m_errMsg, -1);
            break;
        }
        addLog(QString("%1V功率测试正常, 功率为: %2").arg(currentVol).arg(power));

        int lvdsFlag = m_lvdsFrameCount;
        int iviFlag = m_iviFrameCount;
        QThread::msleep(500);
        int lvdsOffset = m_lvdsFrameCount - lvdsFlag;
        int iviOffset = m_iviFrameCount - iviFlag;
        if(lvdsOffset > 2 && iviOffset > 2) {
            addLog(QString("%1V电压测试，出图检测完成").arg(QString::number(currentVol)));
        } else {
            m_errorCode = -19;
            m_errMsg = QString("%1V电压测试，出图检测失败").arg(QString::number(currentVol));
            addLog(m_errMsg, -1);
            break;
        }

        if (currentVol == maxPower) {
            result = true;
        }
    }

    QString resultStr = result ? "PASS" : "FAIL";
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    updateTableResult(QStringList() << "宽电压测试" << "9~32V" <<
                         QString("9~%1V").arg(lastVol) << QString::number(m_endTime - m_startTime) << resultStr);
    m_startTime = m_endTime;

    addLog("宽电压测试完成");
    m_reportList.append(resultStr);
}

void AutoProcessCheckArm::workStepCheckUsb()
{
    if(m_errorCode != 0) return;

    QString keyDisk = ConfigInfo::getInstance()->getValueString("USB检测盘符");
	if (keyDisk.isEmpty()) {
    	addLog("USB测试已跳过...");
		return;
	}

    addLog("开始USB测试...");
    keyDisk = keyDisk.toUpper();
    if (keyDisk.indexOf(":") == -1) {
        keyDisk += ":";
    }

    QProcess ps;
    QStringList param;
    param << "/C" << "chkdsk" << keyDisk;
    ps.start("cmd.exe", param);
    ps.waitForBytesWritten(1000);
    ps.waitForFinished(1000);
    QByteArray output = ps.readAllStandardOutput();

    QTextCodec* codec = QTextCodec::codecForName("gbk");
    QString log = codec->toUnicode(output);

    QByteArray errput = ps.readAllStandardError();
    QString logErr = codec->toUnicode(errput);

    logAddNormalLog(log);
    logAddFailLog(logErr);

    if (!log.contains("FAT32")) {
        m_errorCode = -1;
        addLog("USB 测试失败!", -1);
    }

    QString resultStr = (m_errorCode == 0 ? "PASS" : "NG");
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "USB测试"
                         << "-" << resultStr
                         << useTime << resultStr);
    m_startTime = m_endTime;
    addLog("USB测试完成");
}

void AutoProcessCheckArm::workStepCheckAlgo()
{
    if(m_errorCode != 0) return;
    addLog("开始检测算法功能...");
//    QString result = msgBox("请确认视频是否存在行人和汽车算法框", 2);
//    if (result != "PASS") {
//        m_errorCode = -10;
//        m_errMsg = "图像算法功能测试失败";
//        addLog("图像算法功能测试失败", -1);
//    } else {
//        addLog("图像算法功能测试完成");
//    }
    if (!checkAlgoBox()) {
        m_errorCode = -10;
        m_errMsg = "自动检测算法框失败";
        addLog("自动检测算法框失败", -1);
    }

    showMsgBox("请确认视频是否存在行人和汽车算法框", 5);
    QString rst = waitForOkNgSignal();
    if (rst.contains("OK")) {
    } else if (rst.contains("NG")) {
        m_errorCode = -9;
        m_errMsg = "算法功能测试失败";
    } else {
        m_errorCode = -9;
        m_errMsg = "算法功能测试失败[未知错误]";
    }
    hideMsgBox();

    QString resultStr = (m_errorCode == 0 ? "PASS" : "NG");
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "算法功能"
                         << "-" << resultStr
                         << useTime << resultStr);
}

void AutoProcessCheckArm::workStepOemVersion()
{
    if(m_errorCode != 0) return;

    addLog("开始OEM硬件版本测试...");

    ArmWorkInfo info;
    if (!m_mcuSerial->getArmWorkInfo(info)) {
        m_errorCode = -2;
        m_errMsg = "获取ARM Work info 失败";
        return;
    }
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString expVersion = "";
    ConfigInfo::getInstance()->getValueString("CB1004", "OEM硬件版本", expVersion);
    if (expVersion != info.oemHwVersion) {
        m_errorCode = -1;
        m_errMsg = "OEM 硬件版本号比对失败";
    }

    QString resultStr = (m_errorCode == 0) ? "PASS" : "NG";
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "OEM硬件版本" << expVersion << info.oemHwVersion << useTime << resultStr);
    m_startTime = m_endTime;
    m_oemHwVersion = info.oemHwVersion;
    MesCheckItem item;
    item.sItem = "oem_hw_version";
    item.sValue = info.oemHwVersion;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);

    addLog("OEM硬件版本测试完成");
    m_reportList.append(item.sValue);
}

void AutoProcessCheckArm::workStepCompareVersion()
{
    if(m_errorCode != 0) return;

    addLog("开始OEM软件版本号测试...");

    ArmWorkInfo info;
    m_mcuSerial->getArmWorkInfo(info);
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString expVersion = "";
    ConfigInfo::getInstance()->getValueString("CB1004", "OEM软件版本", expVersion);
    if (expVersion != info.oemSwVersion) {
        m_errorCode = -1;
        m_errMsg = "OEM 软件版本号比对失败";
    }

    QString resultStr = (m_errorCode == 0) ? "PASS" : "NG";
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "OEM软件版本号" << expVersion << info.oemSwVersion << useTime << resultStr);
    m_startTime = m_endTime;

    m_oemSwVersion = info.oemSwVersion;
    MesCheckItem item;
    item.sItem = "oem_sw_version";
    item.sValue = info.oemSwVersion;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);

    addLog("OEM软件版本号测试完成");
    m_reportList.append(item.sValue);
}

void AutoProcessCheckArm::workStepCheckPower()
{
    if(m_errorCode != 0) return;

    addLog("开始功率测试...");

    ArmWorkInfo info;
    m_mcuSerial->getArmWorkInfo(info);


    float resultValue = info.elecValue * g_powerOnVol;
    float expectValueMin = 4.75;
    float expectValueMax = 7.25;
    QString resultStr = "PASS";

    if(resultValue < expectValueMin || resultValue > expectValueMax) {
        m_errorCode = -3;
        resultStr = "FAIL";
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "功率检测"
                         << QString("%1 ~ %2").arg(QString::number(expectValueMin)).arg(QString::number(expectValueMax))
                         << QString::number(resultValue)
                         << useTime << resultStr);
    m_startTime = m_endTime;;

    MesCheckItem item;
    item.sItem = "power_check";
    item.sValue = resultValue;
    item.sResult = resultStr;
    item.sMin = QString::number(expectValueMin);
    item.sMax = QString::number(expectValueMax);
    item.tCheckTm = QString::number(m_endTime - m_startTime);
    m_mesPackage.append(item);

    addLog(QString("功率检测完成，当前电流%1, 功率%2").arg(QString::number(info.elecValue)).arg(QString::number(resultValue)));
    m_reportList.append(QString::number(resultValue));
}

void AutoProcessCheckArm::workCheckLvds()
{
    if(m_errorCode != 0) return;

    addLog("开始LVDS测试...");

    bool result = false;
    int lvdsFlag = m_lvdsFrameCount;
    int iviFlag = m_iviFrameCount;
    int checkCount = 50;
    while(checkCount > 0) {
        checkCount--;
        if(lvdsFlag != m_lvdsFrameCount && iviFlag != m_iviFrameCount) {
            result = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    if(iviFlag == m_iviFrameCount) {
        m_errorCode = -4;
    } else if(lvdsFlag == m_lvdsFrameCount) {
        m_errorCode = -5;
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString resultStr = result ? "PASS" : "FAIL";
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "LVDS检测" << "-" << "-"
                         << useTime << resultStr);
    m_startTime = m_endTime;

    MesCheckItem item;
    item.sItem = "lvds_check";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);
    addLog("LVDS测试完成");
    m_reportList.append(resultStr);
}

void AutoProcessCheckArm::workCheckCan()
{
    if(m_errorCode != 0) return;
    addLog("开始CAN测试...");

    bool checkResult = false;
    int checkCount = 30;
    while(checkCount-- > 0) {
        ArmWorkInfo info;
        m_mcuSerial->getArmWorkInfo(info);
        if(info.canWorkFlag == 1) {
            if(info.canWorkEnable) {
                checkResult = true;
                break;
            } else {
                checkResult = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    if(!checkResult) {
        m_errorCode = -6;
    }

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString resultStr = checkResult ? "PASS" : "FAIL";
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "CAN检测" << "-" << "-"
                      << useTime << resultStr);
    m_startTime = m_endTime;

    MesCheckItem item;
    item.sItem = "can_check";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);
    addLog("CAN测试完成");
    m_reportList.append(resultStr);
}

void AutoProcessCheckArm::workCheckRs232()
{
    if(m_errorCode != 0) return;
    addLog("开始RS232测试...");

    if (!checkProductRs232()) {
        m_errorCode = -7;
        m_errMsg = "产品RS232检查失败";
    }
    m_handleProduct->close();

#if 0
    bool checkResult = false;
    int checkCount = 30;
    while(checkCount > 0) {
        checkCount--;
        ArmWorkInfo info;
        m_mcuSerial->getArmWorkInfo(info);
        if(info.rs232CheckFlag == 1) {
            if(info.rs232CheckResult) {
                checkResult = true;
            } else {
                checkResult = false;
            }
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    if(!checkResult) {
        m_errorCode = -7;
        m_errMsg = "产品RS232检查失败";
    }
#endif
    QString resultStr = m_errorCode == 0 ? "PASS" : "FAIL";

    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "RS232检测" << "-" << "-" << useTime << resultStr);
    m_startTime = m_endTime;;

    MesCheckItem item;
    item.sItem = "rs232_check";
    item.sValue = resultStr;
    item.sResult = resultStr;
    item.tCheckTm = useTime;
    m_mesPackage.append(item);
    addLog("RS232测试完成");
    m_reportList.append(resultStr);
}

void AutoProcessCheckArm::getSn()
{
    if(m_errorCode != 0) return;

    m_mcuSerial->changeMcuWorkMode(MCU_WORK_MODE_SET);
    bool result = m_mcuSerial->readSn(m_sn);
    QString resultStr = result ? "PASS" : "FAIL";
    m_endTime = QDateTime::currentMSecsSinceEpoch();


    updateTableResult(QStringList() << "获取SN" << "-" << m_sn <<
                         QString::number(m_endTime - m_startTime) << resultStr);
    m_startTime = m_endTime;;
    addLog("获取SN :" + m_sn);

    m_mcuSerial->changeMcuWorkMode(MCU_WORK_MODE_CHECK);
    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
    }
    m_reportList.append(m_sn);
}

void AutoProcessCheckArm::workPrinter()
{
    if(m_errorCode != 0) return;

    addLog("打开打印机");
    do {
        CMyPrinter printer;
        QString printerName = printer.defalutPrinterName();
        if(printerName.isEmpty()) {
            logFail("获取打印机失败");
            m_errorCode = -8;
            break;
        }

        QString modulePath;
        //if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
            modulePath = QCoreApplication::applicationDirPath() + "/res/控制盒标贴.btw";
        //} else {
        //    MesCom::instance()->getproductPrintTemplate(modulePath);
        //}
        qDebug() << "file path is " << modulePath;


        if(!printer.load(printer.defalutPrinterName(), modulePath, 1, 1)) {
            logFail("加载打印机模板失败");
            m_errorCode = -8;
            break;
        }

        QStringList itemList = printer.items();
        for(int i=0; i<itemList.size(); i++) {
            if(itemList[i].contains("ID006")) {
                printer.setItem("ID006", m_sn);
            }
            if(itemList[i].contains("ID004")) {
                printer.setItem("ID004", m_oemSwVersion);
            }
            if(itemList[i].contains("ID005")) {
                printer.setItem("ID005", m_oemHwVersion);
            }
            if(itemList[i].contains("ID003")) {
                printer.setItem("ID003", QDateTime::currentDateTime().toString("yyyyMMdd"));
            }
        }

        qDebug() << printer.items();

        addLog("开始打印");
        printer.print();
        addLog("打印完成");
    } while(false);

    QString resultStr = m_errorCode == 0 ? "PASS" : "NG";
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "打印标贴"
                         << "-" << resultStr
                         << useTime << resultStr);
    m_startTime = m_endTime;

}

void AutoProcessCheckArm::workCompareSn()
{
    if(m_errorCode != 0) return;

    QString sn;
    if (snGetMethod() == SnGetMothod_Manual) {
        addLog("请贴好铭牌标贴，并按按钮继续下一步作业");
        sn = scanAutoBarcode();
    } else {
        addLog("请手动扫码铭牌SN");
        sn = msgBox("请扫码比较SN");
    }
    addLog("扫码到SN: " + sn);
    if(!sn.contains(m_sn)) {
        m_errorCode = -9;
        m_errMsg = "SN对比失败";
        addLog(m_errMsg, -1);
    }

    QString resultStr = (m_errorCode == 0 ? "PASS" : "NG");
    m_endTime = QDateTime::currentMSecsSinceEpoch();
    QString useTime = QString::number(m_endTime - m_startTime);
    updateTableResult(QStringList() << "对比标贴"
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

void AutoProcessCheckArm::reportResult()
{
    if(m_errorCode == 0) {
        addLog("工作成功 SN: " + m_sn, 1);
        showPass();
    } else {
        addLog("工作失败 SN: " + m_sn + QString::number(m_errorCode) + "," + m_errMsg, -1);
    }
}

bool AutoProcessCheckArm::checkAlgoBox()
{
#if 1
    if (m_imgBuffer.isEmpty()) {
        return false;
    }
#endif

    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);
    if (m_dataImg == nullptr) {
        m_dataImg  = new uchar[info.frameLen];
    }
    memset (m_dataImg, 0, info.frameLen);
#if 1
    memcpy(m_dataImg, m_imgBuffer.data() + info.nvsLen, info.frameLen);
#else
    QFile fileYuvX("D:/data/2023_11_07_19_27_37.yuv");
    fileYuvX.open(QIODevice::ReadWrite);
    m_imgBuffer = fileYuvX.readAll();
    qDebug() << m_imgBuffer.size();
    qDebug() << 640 * 512 * 2;
    memcpy(m_dataImg, m_imgBuffer.data(), 640 * 512 * 2);
    fileYuvX.close();
#endif

    bool brst = false;
    qDebug() << "执行算法库判断";
    // std::vector<Rectangle> findColorRectangles(const unsigned char* uyvyData, int width, int height, int stride, int colorThreshold);
    std::vector<MyRectangle> rst = findColorRectangles(m_dataImg, info.width, info.height, info.width * 2, 10);
    if (rst.size() > 0) {
        addLog("检测到算法框：");
        foreach(MyRectangle rct, rst) {
            QString log;
            log.sprintf("x: %04d, y: %04d, width: %04d, height: %04d", rct.x, rct.y, rct.width, rct.height);
            addLog(log);
            qDebug() << log;
        }
        brst = true;
    }

    qDebug() << "保存算法图片";
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath") + QDateTime::currentDateTime().toString("/yyyy_MM_dd/");
    Util::createFileDir(logPath);
    QFile fileYuv(logPath + "/Algo_" + m_sn + (brst ? "_PASS_" : "_FAIL_") + ".yuv");
    if (!fileYuv.open(QIODevice::ReadWrite)) {
        addLog("创建本地快门图像失败", -1);
        return false;
    }
    fileYuv.write((char*)m_dataImg, info.frameLen);
    fileYuv.close();

    if (brst) {
        addLog("图像算法框检测OK");
    } else {
        addLog("图像算法框检测失败", -1);
    }
    qDebug() << "算法检测完成";
    return brst;
}

void AutoProcessCheckArm::addLog(QString log, const int result)
{
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
        showProcess(log);
        // showPass();
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

void AutoProcessCheckArm::mesUpload()
{
    for (int i=0; i<m_mesPackage.size(); i++ ) {
        MesCom::instance()->addItemResultEx(m_mesPackage[i]);
    }
    if(m_errorCode != 0) {
        m_bResultCode = false;
    }
}

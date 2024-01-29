#include "autoprocesseleclock.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QImage>
#include <QThread>

#include "util.h"
#include "StaticSignals.h"
#include "configinfo.h"
#include "CSerialDirect.h"
#include "CMyPrinter.h"

AutoProcessElecLock::AutoProcessElecLock()
{
    m_serial = new ElecLockSerialManager;
    m_ioSerial = new IoSerialManager;
    m_screwSelectSerial = new ScrewSelectSerial;

    connect(this, &AutoProcessElecLock::signalInitCfg, this, &AutoProcessElecLock::initScrewCfg);
    connect(this, &AutoProcessElecLock::signalScrewPosClamped, this, &AutoProcessElecLock::slotScrewSeletState);

    initStateMachine();
}

bool AutoProcessElecLock::initConnect()
{
    addLog("初始化连接");
    m_index = 0;
    m_reWorkTimes = 0;
    m_autoWorkInfo.workState = 0;

    tableClearData();
    showProcess("");

    QString mTemplatePath;
    if (stationName() == "DE1001") {
        if (!getPrintFile(mTemplatePath)) {
            showFail("打印模板文件不存在");
            return false;
        }
        addLog("打印模板文件路径：" + mTemplatePath);
    }

    ConfigInfoElecLock::getInstance()->GetSysInfo(m_sysInfo);
    if(!m_serial->openSerial(m_sysInfo.writeCom, m_sysInfo.readCom)) {
        QMessageBox::information(nullptr, "tips", "电批端口打开失败！");
        return false;
    }

    if(m_sysInfo.ioSwitch) {
        qDebug() << m_sysInfo.ioCom;
        if(!m_ioSerial->openSerial(m_sysInfo.ioCom)) {
            QMessageBox::information(nullptr, "tips", "气缸开关端口打开失败！");
            return false;
        }
        connect(m_ioSerial, &IoSerialManager::signalUpdateIoState, this, &AutoProcessElecLock::slotCheckIoState);
        connect(this, &AutoProcessElecLock::signalOperate, m_ioSerial, &IoSerialManager::slotOperate);
    }

    if(m_sysInfo.selectSwitch) {
        if(!m_screwSelectSerial->openSerial(m_sysInfo.selectCom)) {
            QMessageBox::information(nullptr, "tips", "批头选择器端口打开失败！");
            return false;
        }
        m_screwSelectSerial->registerInfoCb((ScrewSelectFunc)ScrewSelectCb, this);
    }

    m_serial->registerRealPosInfoCb((realPosInfoFunc)realPosInfoCb, this);
    m_serial->registerRealDataCb((realDataFunc)realDataCb, this);
    StaticSignals::getInstance()->statusUpdateEnable(false);
    m_serial->setEnable(false);

    QString stationCode = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    if(stationCode == "DE1007") {
        ConfigInfo::getInstance()->getValueBoolean("DE1007", "IV3Support", m_iv3Support);
        ConfigInfo::getInstance()->getValueString("DE1007", "IV3Address", m_iv3Address);
        if (m_iv3Support) {
            addLog("开始初始化IV3设备");
            CIv3Socket iv3;
            if (!iv3.init(m_iv3Address)) {
                QMessageBox::information(nullptr, "tips", "IV3初始化失败");
                return false;
            }
            //
            if (projectName().compare("XD01A") == 0) {
                m_iv3ProjectCode = "002";
            } else if (projectName().compare("XD01B") == 0) {
                m_iv3ProjectCode = "003";
            } else if (projectName().compare("XD03A") == 0) {
                m_iv3ProjectCode = "004";
            } else {
                addLog("当前项目无对应IV3配置，请重新适配上位机完成配置", -1);
                QMessageBox::information(nullptr, "tips", "当前项目无对应IV3配置，请重新适配上位机完成配置");
                return false;
            }
        }
    }

    addLog("设备连接成功，请点击开始按钮");
    return true;

}

void AutoProcessElecLock::slotConnectBtnClicked()
{
    if(!initConnect()) {
        return;
        //updateStatus(1);
    }

    BaseProduce::slotConnectBtnClicked();
}

void AutoProcessElecLock::slotStartWorkBtnClicked()
{
    qDebug() << "slotStartWorkBtnClicked =============";
    m_isRunning = true;
    emit tableClearData();
    emit logClearData();
    logClear();

    m_errMsg.clear();
    startProcess();
}

void AutoProcessElecLock::slotStopWorkBtnClicked()
{
//    qDebug() << "slotStopWorkBtnClicked =============";
    m_isRunning = false;
    stopProcess();
    addLog("停止工作");
    m_signal.signal();
    hideMsgBox();
    m_index = 0;
}

void AutoProcessElecLock::ScrewSelectCb(ScrewSelectInfo info, void* param)
{
    AutoProcessElecLock *window = reinterpret_cast<AutoProcessElecLock *>(param);
    if(window == nullptr) {
        return;
    }
    window->m_screwSelectInfo = info;
}

void AutoProcessElecLock::realPosInfoCb(ElecLockPosInfo info, void *param)
{
    AutoProcessElecLock *window = reinterpret_cast<AutoProcessElecLock *>(param);
    if(window == nullptr) {
        return;
    }
    window->prasePosInfo(info);
}

void AutoProcessElecLock::realDataCb(ElecLockWorkInfo info, void *param)
{
    AutoProcessElecLock *window = reinterpret_cast<AutoProcessElecLock *>(param);
    if(window == nullptr) {
        return;
    }
    window->praseWorkInfo(info);
}

void AutoProcessElecLock::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &AutoProcessElecLock::slotsStoped);
    add(0, "check next", std::bind(&AutoProcessElecLock::checkNext, this));
    add(10, "check work mode", std::bind(&AutoProcessElecLock::checkWorkMode, this));
    add(20, "get sn code", std::bind(&AutoProcessElecLock::getSn, this));
    add(21, "enter mes", std::bind(&AutoProcessElecLock::enterMes, this));
    if (stationName() == "DE1003") {
        add(22, "get src code", std::bind(&AutoProcessElecLock::getSourceSn, this));
    }
    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1002") {
        add(23, "getLenSn", std::bind(&AutoProcessElecLock::getSensorSn, this));
    }
    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1005") {
        add(24, "getBackSN", std::bind(&AutoProcessElecLock::getBackSN, this));
    }
    if(stationName() == "CL1001" || stationName() == "AA1007" ||
        stationName() == "AA1008" || stationName() == "AA1009") {
        add(25, "getShutterCode", std::bind(&AutoProcessElecLock::getShutterCode, this));
    }
    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1011") {
        add(26, "setScrewSelectInit", std::bind(&AutoProcessElecLock::initScrewSelect, this));
    }

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1007") {
        add(35, "判断是否到了原点位置", std::bind(&AutoProcessElecLock::moveToOriginal, this));
        add(36, "判断是否到了原点位置", std::bind(&AutoProcessElecLock::pullProduct, this));
        add(37, "判断是否到了原点位置", std::bind(&AutoProcessElecLock::pullCoverages, this));
    }

    add(40, "auto work", std::bind(&AutoProcessElecLock::autoWork, this));

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1011") {
        add(41, "setScrewSelectInit", std::bind(&AutoProcessElecLock::initScrewSelect, this));
    }

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1007") {
        add(45, "判断是否到了CCD位置", std::bind(&AutoProcessElecLock::moveToCCD, this));
        add(46, "判断是否到了CCD位置", std::bind(&AutoProcessElecLock::iv3Test, this));
    }

    if (stationName() == "DE1001") {
        add(47, "打印整机SN", std::bind(&AutoProcessElecLock::print, this));
        add(48, "比对整机SN", std::bind(&AutoProcessElecLock::compare, this));
    }

    add(50, "mes upload", std::bind(&AutoProcessElecLock::mesUpload, this));
    add(60, "out mes", std::bind(&AutoProcessElecLock::outMes, this));
    add(61, "out mes", std::bind(&AutoProcessElecLock::overTest, this));
    add(70, "report", std::bind(&AutoProcessElecLock::stopProcess, this));
}

void AutoProcessElecLock::checkWorkMode()
{
    if(ConfigInfo::getInstance()->sysInfo().mode == 1) {
        addLog("进入点检模式...");
    } else {
        addLog("进入工作模式");
//        emit signalWorkFinish();
//        stopProcess();
        return;
    }

    m_serial->setEnable(true);
    StaticSignals::getInstance()->statusUpdateEnable(true);
    while(m_isRunning) {
        bool checkResult = true;
        for(int i=0; i<3; i++) {
            addLog("点检次数 - " + QString::number(i+1));
            QString mnm = msgBox("输入扭力测试仪上扭力，单位N.m");
            qDebug() << "mnm " << mnm << m_result.workMnm << "----------------------";
            if(fabs(mnm.toFloat() - m_result.workMnm) > m_result.workMnm/10) {
                QString tips = QString("第%1次点检失败, 输入扭力%2N.m, 实际扭力%3N.m").
                        arg(QString::number(i+1)).arg(mnm).arg(m_result.workMnm);

                addLog(tips, -1);
                msgBox(tips, 3);
                checkResult = false;
                break;
            }
        }
        if(checkResult) {
            addLog("点检成功");
            // emit signalWorkFinish();
            stopProcess();
            break;
        }
        QThread::msleep(1000);
    }
    m_serial->setEnable(false);
    StaticSignals::getInstance()->statusUpdateEnable(false);
}

void AutoProcessElecLock::slotCheckIoState(const int state)
{
    qDebug() << "m_switchStatus =========== " << m_switchStatus << state;
    m_switchStatus = (IoSerialState)state;
}

void AutoProcessElecLock::slotsStoped()
{
    if(ConfigInfo::getInstance()->sysInfo().mode != 1) {
        startProcess();
    }
}

void AutoProcessElecLock::checkNext()
{
    if(m_errorCode != 0) {
        msgBox("FAIL 请将不良品放入不良品盒", 3);
    }

    // 每次开始前更新一下配置
    ConfigInfoElecLock::getInstance()->GetSysInfo(m_sysInfo);
    ConfigInfoElecLock::getInstance()->GetLockInfo(m_currenWorkList);

    QString product = ConfigInfo::getInstance()->getValueString("项目选择");
    ConfigInfoElecLock::getInstance()->changeProduce(product);

    addLog("开始初始化配置");
    emit tableAddHeader(QStringList() << "螺丝编号" << "扭力" << "扭力范围" << "圈数"
                        << "圈数范围" << "工作时间(ms)" << "返工次数" << "结果");
    m_isAutoWorking = false;
    m_isScrewWorking = false;
    m_index = 0;
    m_reWorkTimes = 0;
    m_autoWorkInfo.workState = 0;
    m_errorCode = 0;
    m_errMsg.clear();
    m_switchStatus = IO_SERIAL_STATE_INVALID;
	
	// 2023年10月24日 此部分初始化为true，防止后面异常
    m_isRunning = true;
}

void AutoProcessElecLock::getSensorSn()
{
    if(m_errorCode != 0) return;

    addLog("开始扫码 探测器编码");

    StaticSignals::getInstance()->statusUpdateImg("SN1.png");

    QString code = msgBox("请扫码 探测器编码");
    addLog("获取探测器编码 " + code);

    MesCheckItem item;
    item.sItem = "sensor_code";
    if(code.size()< 11) {
        addLog("探测器编码失败", -1);
        m_errorCode = -18;
        item.sResult = MESFAILED;
    } else {
        item.sResult = MESPASS;
    }
    item.sValue = code;
    MesCom::instance()->addItemResultEx(item);
}

void AutoProcessElecLock::getBackSN()
{
    if(m_errorCode != 0) return;
    addLog("开始扫码 上盖编码");

    StaticSignals::getInstance()->statusUpdateImg("SN1.png");

    QString code = msgBox("请扫码 上盖编码");
    addLog("获取上盖编码 " + code);

    MesCheckItem item;
    item.sItem = "camera_back_comp";
    if(code.size() != 15) {
        addLog("上盖编码失败,不是15位数", -1);
        m_errMsg = "上盖编码失败,不是15位数";
        m_errorCode = -18;
        item.sResult = MESFAILED;
    } else {
        item.sResult = MESPASS;
    }
    item.sValue = code;
    MesCom::instance()->addItemResultEx(item);
}

void AutoProcessElecLock::getShutterCode()
{
    if(m_errorCode != 0) return;

    addLog("开始扫码 快门编码");

    StaticSignals::getInstance()->statusUpdateImg("sn_shutter.png");

    QString code = msgBox("请扫码 快门编码");
    addLog("快门编码 " + code);

    MesCheckItem item;
    item.sItem = "shutter_code";
    if(code.size()<= 0) {
        addLog("快门SN码失败", -1);
        m_errorCode = -18;
        item.sResult = MESFAILED;
    } else {
        item.sResult = MESPASS;
    }
    item.sValue = code;
    MesCom::instance()->addItemResultEx(item);
}

void AutoProcessElecLock::initScrewSelect()
{
    if(!m_sysInfo.selectSwitch) return;

    int index = -1;
    while(m_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if(index == m_screwSelectInfo.posCheckState) continue;
        index = m_screwSelectInfo.posCheckState;

        if(m_screwSelectInfo.posCheckState == 3) {
            emit signalScrewPosClamped(1, true);
            emit signalScrewPosClamped(2, true);
            return;
        } else {
            addLog("将两个批头放入选择器");
            emit signalScrewPosClamped(1, false);
            emit signalScrewPosClamped(2, false);
            continue;
        }
    }
}

void AutoProcessElecLock::getSn()
{
    QString title = "请扫码 SN";
    if (stationName() == "DE1001") {
        title = "请扫码PCB SN";
    }
    QString sn = msgBox(title);
    addLog("扫码到SN: " + sn + " 工序：" + stationName());
    m_sn = sn;

    // 2024年1月12日 向华
    // 新加入DE1001工序获取整机SN的方法：1. 在线通过PCB SN获取 2. 离线通过输入整机SN
    if (stationName() == "DE1001") {
        if (isOnlineMode()) {
            if(0 != MesClient::instance()->getSnByMaterialSn(sn, m_sn)){
                addLog("通过PCB SN获取整机SN失败", -1);
                m_errMsg = "通过PCB SN获取整机SN失败";
                m_errorCode = -2;
                return;
            }
        } else {
            m_sn = msgBox("请输入整机SN");
        }
    }

    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);

    addLog("整机SN: " + m_sn);
    tClear();
    logClear();
    m_errorCode = 0;
}

void AutoProcessElecLock::getSourceSn()
{
    addLog("扫电源板SN编码");
    QString srcSn = msgBox("扫电源板SN编码");
    MesCheckItem item;
    item.sItem = "PowerPCBA_SN";
    if(srcSn.size()< 15) {
        logFail("镜头组件编码失败");
        item.sResult = MESFAILED;
        m_errorCode = -2;
    }else {
        srcSn = srcSn.left(15);
    }
    addLog("获取电源板编码 " + srcSn);

    item.sValue = srcSn;
    item.sResult = MESPASS;
    MesCom::instance()->addItemResultEx(item);
}

void AutoProcessElecLock::praseWorkInfo(ElecLockWorkInfo info)
{
    if(m_sysInfo.ioSwitch) {
        if(!m_produceStart) return;
    }

    // 发生一次工作状态从0-1-0的变化，说明拧紧或者拧松了一次
    if(m_autoWorkInfo.workState != info.workState) {
        m_autoWorkInfo.workState = info.workState;
        if(m_autoWorkInfo.workState == 0) {
            m_autoWorkInfo = info;
            // 此时检查电批返回结果
            checkWorkResult();
        }
    }
}

void AutoProcessElecLock::prasePosInfo(ElecLockPosInfo info)
{
    LockPosInfo cfgInfo;
    if(m_index < m_currenWorkList.size()) {
        cfgInfo = m_currenWorkList[m_screwIndex];
    }
    m_autoPosInfo.isCrc = info.flag;
    m_autoPosInfo.x = info.x;
    m_autoPosInfo.y = info.y;
    m_autoPosInfo.offset = 0;
    m_autoPosInfo.imgPath = cfgInfo.imgPath;
    m_autoPosInfo.name = cfgInfo.name;
    qint64 offset = sqrt(pow(info.x-cfgInfo.x.toInt(), 2) + pow(info.y-cfgInfo.y.toInt(), 2));
    m_autoPosInfo.offset = abs(offset);
    StaticSignals::getInstance()->statusUpdatePos(m_autoPosInfo.x, m_autoPosInfo.y);

    // 没开始打螺丝不比较偏差也不判断治具状态
    if(!m_isScrewWorking) return;

    StaticSignals::getInstance()->statusUpdateOffset(m_autoPosInfo.offset);
    setScrewDeviceEnable(false);
    // 如果开启io检测开关，开始打螺丝后，每次都判断治具的夹紧状态
    if(m_sysInfo.ioSwitch) {
        QString produceId = ConfigInfo::getInstance()->cacheInfo().sCodeId;
        if(produceId == "DE1011") {
            int curIndex = m_index+1;
            qDebug() << "m_switchStatus is " << curIndex << m_switchStatus;
            if(curIndex == 5 && m_switchStatus != IO_SERIAL_STATE_LEFT) {
                addLog("将治具向左翻转，打侧面螺丝");
                return;
            } else if(curIndex == 6 && m_switchStatus != IO_SERIAL_STATE_RIGHT) {
                addLog("将治具向右翻转，打侧面螺丝");
                return;
            }
        }
        if(m_produceStart && m_switchStatus == IO_SERIAL_STATE_OPEN) {
            addLog("作业中...盖板被打开", -1);
            return;
        } else if(m_switchStatus == IO_SERIAL_STATE_ERROR) {
            addLog("未放置物料", -1);
            return;
        } else if(m_switchStatus == IO_SERIAL_STATE_INVALID) {
            addLog("请闭合盖板，开始工作");
            return;
        }
    }

    // 批头选择器开关打开，每个螺丝都检测劈头选择器状态
    if(m_sysInfo.selectSwitch) {
        int pos = cfgInfo.screwModel.toInt();
//        qDebug() << "pos state " << pos << m_screwSelectInfo.posCheckState << m_screwSelectInfo.posWorkState << (m_screwSelectInfo.posWorkState & m_screwSelectInfo.posCheckState);
        if(m_screwSelectInfo.posCheckState == 0) {
            addLog("批头选择器未放置批头");
            emit signalScrewPosClamped(1, false);
            emit signalScrewPosClamped(2, false);
            return;
        } else if(m_screwSelectInfo.posCheckState == pos || m_screwSelectInfo.posCheckState == 3) {
            addLog(QString("未取出正确批头,请更换批头%1").arg(pos));
            emit signalScrewPosClamped(pos, false);
            if(m_screwSelectInfo.posCheckState == 3) {
                emit signalScrewPosClamped(pos == 1 ? 2 : 1, true);
            }
            return;
        } else {
            if(m_screwSelectInfo.posWorkState & m_screwSelectInfo.posCheckState) {
                emit signalScrewPosClamped(m_screwSelectInfo.posCheckState, true);
                return;
            }
        }
    }

    m_produceStart = true;
    StaticSignals::getInstance()->statusUpdateImg(m_autoPosInfo.imgPath);

    if(abs(offset) < cfgInfo.limit.toInt()) {
        setScrewDeviceEnable(true);
        addLog(QString("已到达点位%1,请下压电批").arg(m_index+1));
    } else {
        setScrewDeviceEnable(false);
        addLog(QString("请移动到点位%1").arg(m_index+1));
    }
}

void AutoProcessElecLock::checkWorkResult()
{
    m_result.workPos = QString("%1, %2").arg(m_autoPosInfo.x).arg(m_autoPosInfo.y);
    m_result.workCircle = m_autoWorkInfo.circleNum;
    m_result.workMnm = m_autoWorkInfo.mnmMax / 1000.0f; // mN.m => N.m
    m_result.workTime = m_autoWorkInfo.workTime;
    m_result.errorCode = m_autoWorkInfo.errorCode;
    m_result.index = m_index;
    m_result.reworkTimes = m_reWorkTimes;
    m_result.name = m_autoPosInfo.name;
    StaticSignals::getInstance()->statusUpdateWorkResult(QString::number(m_result.workMnm, 'f', 3), QString::number(m_result.workCircle));
    if(!m_isScrewWorking) return;
    addLog(QString("direction: %1, workResult: %2").arg(m_autoWorkInfo.direction).arg(m_autoWorkInfo.workResult));

    addLog(QString("点位[%1,%2], 圈数[%3], 扭力[%4]").arg(m_autoPosInfo.x).arg(m_autoPosInfo.y).arg(m_result.workCircle).arg(m_result.workMnm));
    if(m_autoWorkInfo.direction == 1) {          // 拧紧
        m_result.result = m_autoWorkInfo.workResult; // 拧紧才使用电批的成功失败结果
        if(m_autoWorkInfo.workResult == 1) {    // 成功
            addLog(QString("点位%1完成").arg(m_result.index+1), 1);
//            if(m_index >= m_currenWorkList.size()-1) {
//                showPass();
//            }
            updateMesPackage(m_result, true);
            m_isScrewWorking = false;
        } else if(m_autoWorkInfo.workResult == -1){
            QString errorMsg = checkErrorCode(m_result);
            addLog(QString("点位%1" + errorMsg).arg(m_index+1), -1);
        } else {
            addLog(QString("点位%1异常%2").arg(m_index+1).arg(m_autoWorkInfo.workResult), -1);
        }
        updateTable(m_result, m_autoWorkInfo.workResult == 1);
    } else if(m_autoWorkInfo.direction == 2) { // 拧松
        m_result.result = -1;                  // 拧松全是失败，算返工次数
        addLog("已拧松螺丝,电批旋转开关拨到下方,重新拧紧螺丝");
        m_reWorkTimes++;
        m_result.reworkTimes = m_reWorkTimes;
        if(m_reWorkTimes >= 3) {
            QString errorMsg = checkErrorCode(m_result);
            addLog(QString("点位%1" + errorMsg).arg(m_index+1), -1);
            m_errorCode = -20;
            updateMesPackage(m_result, false);
            m_isScrewWorking = false;
        }
        updateTable(m_result, false);
    } else {
        addLog(QString("错误：%1").arg(m_autoWorkInfo.errorCode));
    }
}

// 实时显示需要用N.m
void AutoProcessElecLock::updateTable(AutoWorkResult info, bool result)
{
    LockCfgInfo lockInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(m_screwCfgIndex, lockInfo);//N.m
    float mnmMax = lockInfo.mnm.toFloat() + lockInfo.mnmLimit.toFloat();
    float mnmMin = lockInfo.mnm.toFloat() - lockInfo.mnmLimit.toFloat();
    QString mnmExps = QString("%1 ~ %2").arg(mnmMin).arg(mnmMax);//N.m

    float circle = lockInfo.step1Circle.toFloat() + lockInfo.step2Circle.toFloat() +lockInfo.step3Circle.toFloat();
    float circleMax = circle + lockInfo.circleHuaya.toFloat();
    float circleMin = circle - lockInfo.circleFugao.toFloat();
    QString circleExps = QString("%1 ~ %2").arg(circleMin).arg(circleMax);

    if(result) {
        emit tableUpdateRowData(QStringList() << info.name << QString::number(info.workMnm, 'f', 3) << mnmExps
                             << QString::number(info.workCircle) << circleExps << QString::number(info.workTime)
                             << QString::number(info.reworkTimes) << "PASS", info.index);
        emit tableUpdateTestStatus(0);
    } else {
        emit tableUpdateRowData(QStringList() << info.name << QString::number(info.workMnm, 'f', 3) << mnmExps
                             << QString::number(info.workCircle) << circleExps << QString::number(info.workTime)
                             << QString::number(info.reworkTimes) << "FAIL", info.index);
        emit tableUpdateTestStatus(1);
    }

}

QString AutoProcessElecLock::checkErrorCode(AutoWorkResult workInfo)
{
    if(workInfo.errorCode == 2) {
        return "工作异常：螺丝滑牙,电批旋转开关拨到上方,拧松螺丝";
    } else if(workInfo.errorCode == 1) {
        return "工作异常：螺丝浮高,电批旋转开关拨到上方,拧松螺丝";
    } else {
        return "工作异常,电批旋转开关拨到上方,拧松螺丝";
    }
}

void AutoProcessElecLock::addLog(QString log, const int  result)
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

void AutoProcessElecLock::mesUpload()
{
    if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
        return;
    }
    for (int i=0; i<m_mesPackage.size(); i++ ) {
        MesCom::instance()->addItemResultEx(m_mesPackage[i]);
    }
    if(m_errorCode != 0) {
        m_bResultCode = false;
    } else {
        m_bResultCode = true;
    }
}

void AutoProcessElecLock::overTest()
{
    if (m_errorCode != 0) {
        showFail(m_errMsg);
    } else {
        showPass();
    }

    setScrewDeviceEnable(false);
    m_isAutoWorking = false;
    m_isScrewWorking = false;
    m_index = 0;
    m_reWorkTimes = 0;

    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath");
    Util::createFileDir(logPath);
    //logPath += "/" + m_sn + ".csv";
    tableSaveCsv(logPath +"/" + m_sn + ".csv");
    logSaveTxt(logPath +"/" + m_sn + ".txt");

    msgBox("请更换产品，进行下一轮作业", 4);
}

void AutoProcessElecLock::updateMesPackage(AutoWorkResult info, bool result)
{
    logNormal("添加mes数据");
    int index = info.index +1;
    LockCfgInfo cfgInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(m_index, cfgInfo);
    MesCheckItem mesItem1;
    mesItem1.sItem = QString("screw_no_%1").arg(index);
    mesItem1.sValue = info.name;
    mesItem1.sResult = "PASS";
    m_mesPackage.append(mesItem1);

    MesCheckItem mesItem2;
    mesItem2.sItem = QString("screw_torque_force_%1").arg(index);
    mesItem2.sValue = QString::number(info.workMnm, 'f', 3);
    mesItem2.sExp = QString("%1~%2").arg(cfgInfo.mnm.toFloat() - cfgInfo.mnmLimit.toFloat()).
            arg(cfgInfo.mnm.toFloat() + cfgInfo.mnmLimit.toFloat());
    mesItem2.sResult = result ? "PASS" : "NG";
    m_mesPackage.append(mesItem2);

    MesCheckItem mesItem3;
    mesItem3.sItem = QString("screw_turns_nums_%1").arg(index);
    mesItem3.sValue = QString::number(info.workCircle);
    mesItem3.sResult = result ? "PASS" : "NG";
    m_mesPackage.append(mesItem3);

    MesCheckItem mesItem4;
    mesItem4.sItem = QString("screw_returns_nums_%1").arg(index);
    mesItem4.sValue = QString::number(info.reworkTimes);
    mesItem4.sResult = "PASS";
    m_mesPackage.append(mesItem4);

    MesCheckItem mesItem5;
    mesItem5.sItem = QString("screw_result_%1").arg(index);
    mesItem5.sValue = result ? "PASS" : "NG";
    mesItem5.sResult = result ? "PASS" : "NG";
    m_mesPackage.append(mesItem5);
}

void AutoProcessElecLock::initScrewCfg(const int index)
{
    // 每次断开连接，重新连接后设置一次参数
    LockCfgInfo lockInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(index, lockInfo);
    qDebug() << "initScrewCfg code " << index;
    m_serial->writeValue(TASK_ADDR_ALARM_ENABLE, 1);
    m_serial->writeValue(TASK_ADDR_ALARM_FLOAT_CIRCLE, lockInfo.circleFugao.toFloat()*100);
    m_serial->writeValue(TASK_ADDR_ALARM_SCREWLOOSE_CIRCLE, lockInfo.circleHuaya.toFloat()*100);
    m_serial->writeValue(TASK_ADDR_MNM_MAX, lockInfo.mnm.toFloat() * 1000 + lockInfo.mnmLimit.toFloat() * 1000);// N.m => mN.m
    m_serial->writeValue(TASK_ADDR_MNM_MIN, lockInfo.mnm.toFloat() * 1000 - lockInfo.mnmLimit.toFloat() * 1000);// N.m => mN.m
    m_serial->writeValue(TASK_ADDR_BACK_UP_MNM, lockInfo.mnm.toFloat() * 1000);// N.m => mN.m
    m_serial->writeValue(TASK_ADDR_STEP0_CIRCLE, lockInfo.step1Circle.toFloat()*100);
    m_serial->writeValue(TASK_ADDR_STEP0_SPEED, lockInfo.step1Speed.toInt());
    m_serial->writeValue(TASK_ADDR_STEP1_CIRCLE, lockInfo.step2Circle.toFloat()*100);
    m_serial->writeValue(TASK_ADDR_STEP1_SPEED, lockInfo.step2Speed.toInt());
    m_serial->writeValue(TASK_ADDR_STEP2_CIRCLE, lockInfo.step3Circle.toFloat()*100);
    m_serial->writeValue(TASK_ADDR_STEP2_SPEED, lockInfo.step3Speed.toInt());
    int backoutCircle = lockInfo.step1Circle.toFloat()*100 + lockInfo.step2Circle.toFloat()*100 +lockInfo.step3Circle.toFloat()*100;
    m_serial->writeValue(TASK_ADDR_BACK_OUT_CIRCLE, backoutCircle);

    addLog(QString("配置完成,请移动到螺丝点位%1").arg(m_index+1));
}

void AutoProcessElecLock::autoWork()
{
    if(m_errorCode != 0) return;

    ConfigInfoElecLock::getInstance()->GetWorkList(m_workList);
    ConfigInfoElecLock::getInstance()->GetLockInfo(m_currenWorkList);
    QList<CustomWorkList> tipsList;
    ConfigInfoElecLock::getInstance()->GetTipsList(tipsList);
    m_isAutoWorking = true;
    m_index = 0;

    if (m_workList.size() <= 0) {
        m_errorCode = -1;
        m_errMsg = "当前螺丝批配置加载失败，请重启加载螺丝配置";
        msgBox(m_errMsg, 1);
        return;
    }

    bool useSelector = false;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "按钮选择器", useSelector);
    for(int i=0; i<m_workList.size(); i++) {
        if(m_errorCode != 0) break;

        int index = m_workList[i].index - 1;
        qDebug() << "current type is " << m_workList[i].type << index << "screw index is " << m_index;
        addLog(QString("当前类型：%1，螺丝序号：%2").arg(m_workList[i].type).arg(m_index));
        if(m_workList[i].type == 0) {
            m_screwIndex = index;
            m_screwCfgIndex = m_currenWorkList[index].cfgIndex.right(1).toInt() - 1;
            emit signalInitCfg(m_screwCfgIndex);
            m_isScrewWorking = true;
            while(m_isRunning) {
                QThread::msleep(100);
                if(!m_isScrewWorking) break;
            }
            m_index++;
            m_reWorkTimes = 0;
        } else if(m_workList[i].type == 1) {
            addLog(tipsList[index].txt);
            StaticSignals::getInstance()->statusUpdateImg(tipsList[index].path);
            msgBox(tipsList[index].tips, 3);
            if (useSelector) {
                QThread::msleep(500);
            }
        }
    }
    m_isAutoWorking = false;
}

void AutoProcessElecLock::waitFixtureStatus(int status)
{
    // 等待夹具的某种状态到位
    OperaterStatusID ids;
    QString tips = "";
    switch (status) {
    case IO_SERIAL_STATE_FRONT:
        ids = OID_GetStatusIN3;
        tips = "请将夹具移动到原点位置";
        addLog("请将夹具移动到原点位置");
        break;
    case IO_SERIAL_STATE_BACK:
        ids = OID_GetStatusIN4;
        tips = "请将夹具移动到CCD位置";
        addLog("请将夹具移动到CCD位置");
        break;
    case IO_SERIAL_STATE_PRODUCT_ON:
        tips = "请放入产品";
        ids = OID_GetStatusIN2;
        break;
    case IO_SERIAL_STATE_CLOSE:
        tips = "请扣上盖板";
        ids = OID_GetStatusIN1;
        break;
    }

    int tryCount = 0;
    while (status != m_switchStatus) {
        showProcess(tips);
        qDebug() << m_switchStatus;
        QCoreApplication::processEvents();
        QThread::msleep(20);
        if (tryCount++ >= 25) {
            signalOperate(ids, true);
            tryCount = 0;
        }
    }
    showProcess("夹具移动到位");
    addLog("夹具移动到位");

}

void AutoProcessElecLock::pullProduct()
{
    if (m_errorCode != 0) {
        return;
    }
    waitFixtureStatus(IO_SERIAL_STATE_PRODUCT_ON);
    //msgBox("确认产品放置到位", 4);
}

void AutoProcessElecLock::pullCoverages()
{
    if (m_errorCode != 0) {
        return;
    }


    QString path = QCoreApplication::applicationDirPath() + "/res/DE1007/DE1007_P0.png";
    StaticSignals::getInstance()->statusUpdateImg(path);
    showProcess("请按提示安装好产品连接器");
    addLog("请按提示安装好产品连接器");

    msgBox("请按提示安装好产品连接器", 4);

    waitFixtureStatus(IO_SERIAL_STATE_CLOSE);
    //msgBox("确认盖板放置到位", 4);
    // signalOperate(OID_Out1, true);
}

void AutoProcessElecLock::moveToOriginal()
{
    if (m_errorCode != 0) {
        return;
    }
    waitFixtureStatus(IO_SERIAL_STATE_FRONT);
    signalOperate(OID_Out1, true);
    //msgBox("确认前移到位，磁铁已吸附", 4);
}

void AutoProcessElecLock::moveToCCD()
{
    if (!m_iv3Support) {
        return;
    }
    if (m_errorCode != 0) {
        return;
    }
    // 按照要求，延时2s后下磁，此处勿要删除
    QThread::msleep(2000);

    // 打螺丝位置控制磁铁下磁，如果只有打螺丝则不需要控制螺丝下磁
    signalOperate(OID_Out1, false);

    // 等待移动到IV3相机位置
    waitFixtureStatus(IO_SERIAL_STATE_BACK);

    // IV3相机位置磁铁控制上磁
    signalOperate(OID_Out4, true);

    // 按照要求，上磁后延时2s，此处勿要删除
    QThread::msleep(2000);
   // msgBox("确认后移到位，磁铁已吸附", 4);
}

void AutoProcessElecLock::iv3Test()
{
    if (!m_iv3Support) {
        return;
    }

    if (m_errorCode != 0) {
        return;
    }
    addLog("执行IV3产品型号识别");
    bool bRst = false;
    QString code;
    CIv3Socket iv3Socket;
    if (!iv3Socket.init(m_iv3Address)) {
        m_errorCode = -99;
        m_errMsg = "连接IV3失败";
    } else {
        addLog("配置IV3项目号，当前项目号为：" + projectName() + " 代码：" + m_iv3ProjectCode);
        showProcess("配置项目号...");
        if (!iv3Socket.setProjectCode(m_iv3ProjectCode)) {
            m_errorCode = -99;
            m_errMsg = "配置IV3项目号失败，当前项目号为：" + projectName();
        } else {
            showProcess("产品型号识别中...");
            for (int foo = 0; foo < 3; foo++) {
                code = iv3Socket.productCode();
                if (code != "OK") {
                    addLog("产品型号识别失败，当前配置型号：" + m_iv3ProjectCode + ", 识别型号" + code, -1);
                    signalOperate(OID_Out4, false);
                    msgBox("请重新调整产品位置，执行下一次检测", 4);
                    signalOperate(OID_Out4, true);
                } else {
                    addLog("产品型号识别完成，当前配置型号：" + m_iv3ProjectCode + ", 识别型号：" + code);
                    bRst = true;
                    break;
                }
            }
        }
    }

    if (!bRst) {
        m_errorCode = -9;
        m_errMsg = "产品型号识别失败";
    }

    if(bRst) {
        emit tableAddRowData(QStringList() << "产品识别" << projectName() << code
                             << "/" << "/" << "/"
                             << "/" << "PASS");
        emit tableUpdateTestStatus(0);
    } else {
        emit tableAddRowData(QStringList() << "产品识别" << projectName() << code
                             << "/" << "/" << "/"
                             << "/" << "FAIL");
        emit tableUpdateTestStatus(1);
    }

    signalOperate(OID_Out4, false);
}

void AutoProcessElecLock::print()
{
    if (m_errorCode != 0) {
        return;
    }

    QElapsedTimer tStart;
    tStart.start();
    showProcess("打印SN");
    QString mTemplatePath;
    if (!getPrintFile(mTemplatePath)) {
        m_errorCode = -4;
        m_errMsg = "参数获取失败，[条码打印模板路径]不存在";
        addLog(m_errMsg, -1);
        return;
    }
    logNormal("当前模板文件为："+ mTemplatePath);

    CMyPrinter printer;
    QString printerName = printer.defalutPrinterName();
    logNormal("当前打印机名称："+ printerName);

    if (!printer.load(printer.defalutPrinterName(), mTemplatePath, 1, 1)) {
        logFail("获取默认打印机名称异常或者模板路径未正常配置");
        m_errorCode = -4;
        m_errMsg = "获取默认打印机名称异常或者模板路径未正常配置";
        return ;
    }
    QString sCodeId = "ID00X";
    if (!printer.setItem(sCodeId, m_sn)) {
        logFail("替换贴纸SN失败");
        m_errorCode = -4;
        m_errMsg = "替换贴纸SN失败";
        logFail(m_errMsg);
        return ;
    }

    logNormal("开始打印标签");
    printer.print();
    logNormal("完成打印标签");

    emit tableAddRowData(QStringList() << "打印SN" << "/" <<  "/"
                         <<  "/" <<  "/" << "/" <<  "/" << "PASS");
    emit tableUpdateTestStatus(0);
}

void AutoProcessElecLock::compare()
{
    QString title = "扫码打印SN到框内";
    for (int foo = 0; foo < 3; foo++) {
        QString sn = msgBox(title);
        if (sn.compare(m_sn) != 0) {
            addLog("打印SN: " + sn + "与整机SN: " + m_sn + "不匹配, 重试", -1);
            title = "打印SN与实际SN不匹配，重新扫码";
        } else {
            addLog("打印SN: " + sn + "与整机SN: " + m_sn + "匹配完成", 0);
            return;
        }
        if (foo == 2) {
            m_errorCode = -3;
            m_errMsg = "打印SN比对失败";
            return;
        }
    }

}


bool AutoProcessElecLock::resetElecLock()
{
    SystemInfo infor;
    ConfigInfoElecLock::getInstance()->GetSysInfo(infor);
    if (!infor.elecLockSwitch) {
        logAddWarningLog("当前电子锁未启用，直接OK");
        return true;
    }

    if (!m_serialElecLock.open(infor.elecLockCom, 9600, true)) {
        logAddFailLog("电子锁端口打开失败");
        return false;
    }

    // 继电器断开，即电子锁断电，电子锁断电会加锁
    unsigned char uc_lock_cmd[] = {0x33, 0x01, 0x11, 0x00, 0x00, 0x00, 0x01, 0x46};
    unsigned char uc_lock_ret[] = {0x33, 0x01, 0x11, 0x00, 0x00, 0x00, 0x01, 0x35};
    // 继电器闭合，即电子锁通电，电子锁断电会解锁
    unsigned char uc_unlock_cmd[] = {0x33, 0x01, 0x12, 0x00, 0x00, 0x00, 0x01, 0x47};
    unsigned char uc_unlock_ret[] = {0x33, 0x01, 0x12, 0x00, 0x00, 0x00, 0x01, 0x36};

    bool bRstOk = false;
    m_serialElecLock.write(uc_unlock_cmd, sizeof(uc_unlock_cmd));
    if (!m_serialElecLock.find(uc_unlock_ret, sizeof(uc_unlock_ret), 5000)) {
        logAddFailLog("电子锁解锁失败");
    } else {
        logAddNormalLog("电子锁解锁完成");
        QThread::msleep(1000);
        m_serialElecLock.write(uc_lock_cmd, sizeof(uc_lock_cmd));
        if (m_serialElecLock.find(uc_lock_ret, sizeof(uc_lock_ret), 5000)) {
            logAddNormalLog("电子锁加锁完成");
            bRstOk = true;
        } else {
            logAddFailLog("电子锁加锁失败");
        }
    }
    m_serialElecLock.close();

    return bRstOk;
}

bool AutoProcessElecLock::getPrintFile(QString& filename)
{
    QString mTemplatePath;
    bool ret = ConfigInfo::getInstance()->getValueString(stationName(), "条码打印模板路径", mTemplatePath);
    if (!ret) {
        m_errorCode = -4;
        m_errMsg = "参数获取失败，[条码打印模板路径]不存在";
        addLog(m_errMsg, -1);
        return false;
    }

    if (!QFile::exists(mTemplatePath)) {
        m_errorCode = -4;
        m_errMsg = "模板文件不存在";
        logFail(m_errMsg);
        return false;
    }
    filename = mTemplatePath;
    return true;
}

void AutoProcessElecLock::setScrewDeviceEnable(const bool enable)
{
    if(m_autoPosInfo.enable != enable) {
        m_serial->setEnable(enable);
        m_autoPosInfo.enable = enable;
        StaticSignals::getInstance()->statusUpdateEnable(enable);
    }
}

void AutoProcessElecLock::slotScrewSeletState(const int state, bool flag)
{
    m_screwSelectSerial->setPosClamped(state - 1, flag);
}

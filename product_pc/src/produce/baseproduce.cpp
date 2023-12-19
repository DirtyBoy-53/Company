#include "baseproduce.h"
#include "XyPropertyPage.h"
#include <QTimer>
#include <Windows.h>
BaseProduce::BaseProduce()
{
    showDefault();
    if (isScreenUnlock()) { // 60s 执行一次模拟键盘点击动作
        qDebug() << "启动防锁屏功能已启动";
        connect(&m_timerScreenLocker, &QTimer::timeout, this, [=](){
            // qDebug() << ">>> 防锁屏功能触犯...";
            // 按一下
            keybd_event(VK_SCROLL, 0, KEYEVENTF_EXTENDEDKEY, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            // 再按一下
            keybd_event(VK_SCROLL, 0, KEYEVENTF_EXTENDEDKEY, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);

        });
        m_timerScreenLocker.start(60000);
    }
    m_consumLifeIsOver = false;
}

BaseProduce::~BaseProduce()
{
    if (m_btnSelector) {
        m_btnSelector->close();
        m_btnSelector->freeHandle();
        delete m_btnSelector;
        m_btnSelector = nullptr;
    }
}
void BaseProduce::slotprintSN()
{
   
    return ;
}

void BaseProduce::slotLifeTimeOutLimit(bool lifeOver)
{
    m_consumLifeIsOver = lifeOver;

    qDebug() << "++++++++++++++++++++recieve lifeOver " << lifeOver;
}


void BaseProduce::slotConnectBtnClicked()
{
    // emit tableClearData();
    // emit logClearData();
    // emit resultShowDefault();

    process(); // 序列的形成放在这里，不在后面StartTest的地方了

    m_useBtnSelector = false;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "按钮选择器", m_useBtnSelector);
    if (m_useBtnSelector) {
        m_btnSelector = new CButtonSelector;
        qDebug() << "使用按钮选择器";
        if (!m_btnSelector->getHandle()) {
            showFail("获取按钮选择器句柄失败");
            return;
        } else {
            QString comport;
            ConfigInfo::getInstance()->getValueString("GLOBAL", "按钮选择器端口", comport);
            qDebug() << "开启端口：" << comport;
            m_btnSelector->close();
            if (!m_btnSelector->open(comport)) {
                qDebug() << "开启按钮选择器失败:" << m_btnSelector->errorString();
                showFail("初始化按钮选择器错误: " + comport + m_btnSelector->errorString());
                return;
            } else {
                qDebug() << "开启按钮选择器OK";
            }
        }
    }

    showProcess("初始化完成");
    updateStatus(1);
}

void BaseProduce::slotStartWorkBtnClicked()
{
    emit tableClearData();
    emit logClearData();
    emit resultShowDefault();
    logClear();
    m_errMsg.clear();
    m_bResultCode = true;
    m_bWorkingStatus = true;
    startProcess();
}

void BaseProduce::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_bWorkingStatus = false;
    m_manualStoped = true;
    m_signal.signal();
    hideMsgBox();
    showProcess("请先点检环境是否正常");
}


void BaseProduce::slotBoxClosed(QString text)
{
    m_boxMsg = text;
    m_signal.signal();
}

void BaseProduce::slotClickedFinish(bool result)
{
    if(result) {
        m_boxMsg = "PASS";
    } else {
        m_boxMsg = "FAIL";
    }
    m_signal.signal();
}

QString BaseProduce::msgBox(QString sMsg, int type)
{
    QString data;
    if (m_useBtnSelector && type != 0) {
        showMsgBox(sMsg, 5);
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < 3600000) {
            QCoreApplication::processEvents();
            BS_Status bss = m_btnSelector->getStatus(1500);
            if (bss == BS_StatusOK) {
                data = "PASS";
                break;
            } else if (bss == BS_StatusNG) {
                data = "FAIL";
                break;
            }
        }
        hideMsgBox();

    } else {
        showMsgBox(sMsg, type);
        if(sMsg.length() <= 0) {
            return "";
        }
        int iCount = 20000000;
        while(iCount > 0) {
            if(m_signal.wait(200)) {
                break;
            }
            iCount --;
        }
        data = m_boxMsg.toUpper();
    }
    return data;
}

void BaseProduce::cClearGraph()
{
    emit chartClearGraph();
}

void BaseProduce::cAddGraph(QString name, QString xName, QString yName)
{
    emit chartAddGraph(name, xName, yName);
}

void BaseProduce::cAddLine(QString lineName, int r, int g, int b, int graphNo)
{
    emit chartAddLine(lineName, r, g, b, graphNo);
}

void BaseProduce::cAddData(float x, float y, int graphNo, int lineNo)
{
    emit chartAddData(x, y, graphNo, lineNo);
}

void BaseProduce::cClearData(int graphNo, int lineNo)
{
    emit chartClearData(graphNo, lineNo);
}

void BaseProduce::cSetXRange(float min, float max, int graphNo)
{
    emit chartSetXRange(min, max, graphNo);
}

void BaseProduce::cSetYRange(float min, float max, int graphNo)
{
    emit chartSetYRange(min, max, graphNo);
}

void BaseProduce::logPass(QString log, int slot)
{
    qInfo("pass %s", log.toStdString().data());
    emit logAddPassLog(log, slot);
}

void BaseProduce::logFail(QString log, int slot)
{
    emit logAddFailLog(log, slot);
    showFail(log);

    qInfo("error %s", log.toStdString().data());
    m_errMsg = log;
    m_bResultCode = false;
}

void BaseProduce::logWarming(QString log, int slot)
{
    qInfo("warning %s", log.toStdString().data());
    emit logAddWarningLog(log, slot);
}

void BaseProduce::logNormal(QString log, int slot)
{
    qInfo("normal %s", log.toStdString().data());
    emit logAddNormalLog(log, slot);
}

void BaseProduce::logClear(int slot)
{
    emit logClearData(slot);
}

void BaseProduce::logTxt(QString path, int slot)
{
    emit logSaveTxt(path, slot);
}

void BaseProduce::showDefault(int slot)
{
    emit resultShowDefault(slot);
}

void BaseProduce::showPass(int slot)
{
    emit resultShowPass(slot);
}

void BaseProduce::showFail(QString errString, int slot)
{
    emit resultShowFail(errString, slot);
}

void BaseProduce::showProcess(QString step, int slot)
{
    emit resultShowProcess(step, slot);
}

void BaseProduce::tHeader(QStringList header, int slot)
{
    emit tableAddHeader(header, slot);
}

void BaseProduce::tRowData(QStringList data, int slot)
{
    emit tableAddRowData(data, slot);
}

void BaseProduce::tUpdateData(QString data, int col, int slot)
{
    emit tableUpdateData(data, col, slot);
}

void BaseProduce::tUpdateTestStatus(int status, int slot)
{
    emit tableUpdateTestStatus(status, slot);
}

void BaseProduce::tClear(int slot)
{
    emit tableClearData(slot);
}

void BaseProduce::tCsv(QString list, int slot)
{
    emit tableSaveCsv(list, slot);
}

void BaseProduce::showPackageTable(MesCheckItem *item, QString value)
{
    QStringList item1List;
    item->sValue = value;
    item->sResult = m_bResultCode ? MESPASS : MESFAILED;
    item1List << item->sItem;
    item1List << item->sValue;
    item1List << item->sExp;
    item1List << item->sResult;
    tRowData(item1List);
    if (item->sResult == MESPASS) {
        emit tableUpdateTestStatus(0);
    } else {
        emit tableUpdateTestStatus(1);
    }
}

void BaseProduce::checkItem(MesCheckItem *items, int idx)
{
    int iStart = 0;
    int iEnd = MESMAXCHECKNUM;
    if(idx >= 0 && idx < MESMAXCHECKNUM) {
        iStart = idx;
        iEnd = idx + 1;
    }
    for(int i = iStart; i < iEnd; i ++) {
        QString decItem = items[i].sItem;
        if(decItem.size() <= 0 || items[i].sMin.size() <= 0) {
            continue;
        }
        double value = items[i].sValue.toDouble();
        if(items[i].sValue.size() <= 0
                || (value + 1e-7 < items[i].sMin.toDouble() ||
                value - 1e-7 > items[i].sMax.toDouble() )){
            QString log = QString("Item: %1 \tValue: %3 [%4, %5] FAILED").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(items[i].sMin)
                    .arg(items[i].sMax);
            logFail(log);
            items[i].sResult = "NG";
        }else {
            QString log = QString("Item: %1 \tValue: %3 [%4, %5] PASS").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(items[i].sMin)
                    .arg(items[i].sMax);
            logNormal(log);
            items[i].sResult = MESPASS;
        }
        QStringList item;
        item << items[i].sItem;
        item << items[i].sValue;
        item << (items[i].sMin + "~"+items[i].sMax);
        item << items[i].sResult;
        tRowData(item);
        if (items[i].sResult == MESPASS) {
            emit tableUpdateTestStatus(0);
        } else {
            emit tableUpdateTestStatus(1);
        }
    }

}

bool BaseProduce::isOnlineMode()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    if (value.indexOf("生产模式") >= 0) {
        return true;
    }

    return false;
}

bool BaseProduce::isOfflineMode()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    if (value.indexOf("离线模式") >= 0) {
        return true;
    }

    return false;
}

bool BaseProduce::isGoldenMode()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    if (value.indexOf("点检模式") >= 0) {
        return true;
    }

    return false;
}

bool BaseProduce::isDebugMode()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    if (value.indexOf("调试模式") >= 0) {
        return true;
    }

    return false;
}

bool BaseProduce::isPressureMode()
{
    bool value;
    bool bret = ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "压测模式", value);
    if (!bret) {
        return false;
    }

    return value;
}

bool BaseProduce::isScreenUnlock()
{
    bool value;
    bool bret = ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "启动防锁屏", value);
    if (!bret) {
        return false;
    }

    return value;
}

bool BaseProduce::isWorking()
{
    return m_bWorkingStatus;
}

TestMode BaseProduce::testMode()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    if (value.indexOf("点检模式") >= 0) {
        return TestMode_Golden;
    } else if (value.indexOf("生产模式") >= 0) {
        return TestMode_Production;
    } else if (value.indexOf("离线模式") >= 0){
        return TestMode_Offline;
    } else if (value.indexOf("调试模式") >= 0){
        return TestMode_Debug;
    }

    return TestMode_Offline;
}

QString BaseProduce::testModeStr()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", value);
    return value;
}

QString BaseProduce::projectName()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "项目选择", value);
    return value;
}

SnGetMethod BaseProduce::snGetMethod()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "获取SN", value);
    if (value.compare("MES获取") == 0) {
        return SnGetMothod_Mes;
    } else if (value.compare("产品读取") == 0) {
        return SnGetMothod_Production;
    } else if (value.compare("自动扫码") == 0) {
        return SnGetMothod_Auto;
    }

    return SnGetMothod_Manual;
}

int BaseProduce::snGetLength()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "SN长度", value);
    return value.toInt();
}

QString BaseProduce::snGetPrefix()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "SN前缀", value);
    return value;
}

QString BaseProduce::logPath()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "LogPath", value);
    return value;
}

QString BaseProduce::dutIP()
{
    QString value;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "盒子IP", value);
    return value;
}

void BaseProduce::setCurrentBarcode(QString sn)
{
    m_sn = sn;

    CacheInfo infor = ConfigInfo::getInstance()->cacheInfo();
    infor.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(infor);
}

void BaseProduce::getSn()
{
    showProcess("获取 SN");
    QString sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        if(0 == ConfigInfo::getInstance()->getValueString("isGetSn").toInt()) {
            m_sn = msgBox("扫码SN");
        }else {
            if(0 != MesCom::instance()->getOrderSns(info.orderId, m_sn)) {
                logFail("获取 failed" + sOut);
                m_errorCode = -19;
                stopProcess();
                return;
            }
        }
    }
    else {
        m_sn = msgBox("扫码SN");
    }

    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);
}

void BaseProduce::jugdeCustomLifeTime(bool ifStopProcess)
{
    bool bUseLifeTime = false;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "耗材寿命管控", bUseLifeTime);
    if (bUseLifeTime) {
        updateLifeTime();
        qDebug() << "++++++++++++++++++++use life time";
        if (m_consumLifeIsOver) {
            msgBox("请更换耗材，使用次数已超限", 1);
            m_errorCode = -999;
            if (ifStopProcess) {
                stopProcess();
            }
        }
    }
}

bool BaseProduce::customLifeIsOver()
{
    return m_consumLifeIsOver;
}

void BaseProduce::enterMes()
{
    //只有生产模式存在
    if(!isOnlineMode()) {
        return;
    }

    m_skipMesUpload = false;
    if (m_errorCode != 0) {
        m_skipMesUpload = true;
        return;
    }

    MesCom::instance()->setUser(MesClient::instance()->sUser());
    QString sMes;
    if(!MesCom::instance()->enterProduce(m_sn, sMes)) {
        logFail("进站失败" + sMes);
        m_errMsg = "进站失败" + sMes;
        m_errorCode = -16;
        m_skipMesUpload = true;
        // stopProcess();
        return;
    }
}

void BaseProduce::outMes()
{
    //只有生产模式存在
    if(isOnlineMode() && !m_skipMesUpload) {
        QString sMes;
        MesCom::instance()->setItemResult(m_checkItem);
        if(!MesCom::instance()->outProduce(m_bResultCode?0:1, sMes)) {
            logFail("出站失败 " + sMes);
            m_errorCode = -15;
            m_errMsg = "出站失败 " + sMes;
        }else {
            showPass();
            logNormal("出站成功");
        }
    }else if(isGoldenMode()) {//点检模式上传
        if(m_checkItem) {

        }
    }
}

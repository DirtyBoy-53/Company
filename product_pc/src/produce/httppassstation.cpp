#include "httppassstation.h"


HttpPassStation::HttpPassStation()
{
    initStateMachine();
    m_httpServer.registerGetConnectState((getConnectState)getConnectCodeCallBack, this);
    m_httpServer.registerSetPcbCode((getPcbaCodeFunc)setPcbCodeCallBack, this);
}

void HttpPassStation::slotConnectBtnClicked()
{
    qDebug() << " start http server ";
    m_httpServer.Start(8088);
    m_isRunning = true;
    emit tableAddHeader(QStringList() << "PCB1" << "PCB2" << "结果");

    BaseProduce::slotConnectBtnClicked();
}

void HttpPassStation::slotStopWorkBtnClicked()
{
    m_isRunning = false;
    m_httpServer.Stop();
}

void HttpPassStation::initStateMachine()
{
    emit tableAddHeader(QStringList() << "PCB1" << "PCB2" << "结果");
}

void HttpPassStation::slotsStoped()
{
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath");
    logPath += "/" + m_sn + ".csv";
    tableSaveCsv(logPath);

    startProcess();
}

int HttpPassStation::setPcbCodeCallBack(QList<PcbCodeStruct> &pcba, void *param)
{
    HttpPassStation *window = reinterpret_cast<HttpPassStation *>(param);
    if(window == nullptr) {
        return -99;
    }
    return window->setPcbCode(pcba);
}

void HttpPassStation::getSn()
{
    showProcess("获取 SN");
    QString sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        if(0 != MesCom::instance()->getOrderSns(info.orderId, m_sn)) {
            logFail("获取 failed" + sOut);
            m_errorCode = -19;
            stopProcess();
            return;
        }
    }
    else {
        m_sn = msgBox("扫码SN");
    }

    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);
}

int HttpPassStation::setPcbCode(QList<PcbCodeStruct> &pcba)
{
    qDebug() << "set pcbcode" << pcba.size();
    if(!m_isRunning) {
        m_errorCode = -99;
        return m_errorCode;
    }
    m_list = pcba;

    for(int i=0; i<m_list.size(); i++) {
        m_errorCode = 0;
        addLog("绑定pcb : " + m_list[i].code1 + " " + m_list[i].code2);

        getSn();
        enterMes();

        if(m_errorCode == 0) {
            MesCheckItem item1;
            item1.sItem = "sensor_pcb_1";
            item1.sValue = m_list[i].code1;
            item1.sResult = "PASS";
            MesCom::instance()->addItemResultEx(item1);

            MesCheckItem item2;
            item2.sItem = "sensor_pcb_2";
            item2.sValue = m_list[i].code2;
            item2.sResult = "PASS";
            MesCom::instance()->addItemResultEx(item2);

            m_bResultCode = true;
            outMes();
        }

        updateTable(m_list[i].code1, m_list[i].code2);
    }

    if(m_errorCode == 0) {
        addLog("绑定完成", 1);
    } else {
        addLog("绑定失败", -1);
    }

    return m_errorCode;
}

int HttpPassStation::getConnectCodeCallBack(void *param)
{
    HttpPassStation *window = reinterpret_cast<HttpPassStation *>(param);
    if(window == nullptr) {
        return -99;
    }
    return window->getConnectCode();
}

int HttpPassStation::getConnectCode()
{
    if(m_isRunning) {
        m_errorCode = 0;
    } else {
        m_errorCode = -1;
    }
    tableClearData();
    logClear();
    return m_errorCode;
}

void HttpPassStation::addLog(QString log, const int  result)
{
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
        showPass();
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

void HttpPassStation::updateTable(QString pcb1, QString pcb2)
{
    QString resultStr = m_errorCode == 0 ? "PASS" : "FAIL";

    emit tableAddRowData(QStringList() << pcb1 << pcb2 << resultStr);
    emit tableUpdateTestStatus(0);
}

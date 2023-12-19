#include "comproduce.h"

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item  value minvalue maxvalue exp result
    {"获取SN", "", "","","-"},

    {""},
};

ComProduce::ComProduce()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &ComProduce::slotsStoped);
    add(0, "start", std::bind(&ComProduce::checkNext, this));
    add(10, "getSn", std::bind(&ComProduce::getSn, this));
    add(20, "enterMes", std::bind(&ComProduce::enterMes, this));
    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1000") {
        add(21, "get sn code", std::bind(&ComProduce::getSnCode, this));
    }
    add(30, "outMes", std::bind(&ComProduce::outMes, this));
    add(40, "end", std::bind(&ComProduce::stepend, this));
}

void ComProduce::step0()
{

}

void ComProduce::stepend()
{
    stopProcess();
}

void ComProduce::getSn()
{
    //只有生产模式存在
    if(0 != ConfigInfo::getInstance()->sysInfo().mode) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        m_sn = "XD01A123456789112";
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
        return;
    }

    showProcess("获取 SN");
    QString sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    if(info.sCodeId == "DE1000" || info.sCodeId == "DE1100" || info.sCodeId == "CL1000") {
        if(0 != MesCom::instance()->getOrderSns(info.orderId, m_sn)) {
            logFail("获取 failed" + sOut);
            stopProcess();
            return;
        }
    }else {
        m_sn = msgBox("请扫码sn");
    }

    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);
}

void ComProduce::slotStartWorkBtnClicked()
{
    tClear();
    m_errMsg.clear();
    logClear();
    m_bResultCode = true;
    showProcess("开始");
    startProcess();
}

void ComProduce::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_signal.signal();
}

void ComProduce::slotConnectBtnClicked()
{
    emit tableClearData();
    emit logClearData();

    emit resultShowDefault();
    QStringList headTitles;
    headTitles << "检测项";
    headTitles << "实际值";
    headTitles << "期望值";
    headTitles << "测试结果";
    tHeader(headTitles);

    BaseProduce::slotConnectBtnClicked();
}

void ComProduce::slotsStoped()
{
    qInfo("stop...00");
    if(m_bResultCode) {
        showPass();
    }else {
        showFail("NG");
    }
    m_lstRes = m_bResultCode;

    startProcess();
}

void ComProduce::checkNext()
{
    if(m_errorCode != 0) {
        msgBox("NG 请将不良品放入不良品盒", 3);
    } else {
        if(!m_isFirst) {
            msgBox("PASS 过站成功，确认后开始下一个", 3);
        }
        m_isFirst = false;

    }
    tClear();
    logClear();
}

void ComProduce::getSnCode()
{
    logNormal("扫镜头码");
    QString code1 = msgBox("扫镜头码");
    logNormal("扫镜头支架码");
    QString code2 = msgBox("扫镜镜头支架码");

    MesCheckItem item1;
    item1.sItem = "sensor_pcb_1";
    item1.sValue = code1;
    item1.sResult = "PASS";
    MesCom::instance()->addItemResultEx(item1);

    MesCheckItem item2;
    item2.sItem = "sensor_pcb_2";
    item2.sValue = code2;
    item2.sResult = "PASS";
    MesCom::instance()->addItemResultEx(item2);

    m_bResultCode = true;

}

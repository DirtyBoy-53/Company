#include "BurningDe1000.h"
#include <functional>
#include <QMessageBox>
#include <inputmessagebox.h>
#include <QFile>
#include <CMyPrinter.h>
#include <QThread>
#include <util.h>
#include <QDateTime>
#include <QCoreApplication>
#include <QProcess>
#include <QElapsedTimer>
#include <QTextCodec>

#define CURCODE ConfigInfo::getInstance()->getValueString("当前组件编码")
#define ADDCONFIG(name,value,type,pro,enumNames,bSave) \
{ \
    XmlConfigInfo info={CURCODE,name,value,type,pro,enumNames}; \
    ConfigInfo::getInstance()->addConfig(info, bSave); \
}

#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)

#define CHECKSTATUS \
{\
    if(m_errorCode != 0) return; \
    if("调试模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {\
        if(!m_workStatus)\
        {\
            return;\
        }\
    }\
}

//static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
//    //item             stardvalue minvalue maxvalue result
//    {"用户", "", "","",""},
//    {"SN编码", "", "","",""},
//    {"NK9070A.01_PCBA", "", "","",""},//电源板
//    {"NC1002A.01_PCBA", "", "","",""},//ASIC板
//    {"NK2239A.01_PCBA", "", "","",""},//信号板
//    {"版本文件核对", "", "","",""},
//    {"版本文件烧写", "", "","",""},
//    {"固件校验", "", "","",""},
//    {"asic_version", "", "","",""},
//    {"NK9070A_tp2", "", "4.8","5.2","ADC1_9"},
//    {"NC1002A_tp1", "", "1.1","1.3","ADC1_5"},
//    {"NC1002A_tp2", "", "1.7","1.9","ADC3_1"},
//    {"NC1002A_tp3", "", "1.4","1.6","ADC1_2"},
//    {"NC1002A_tp5", "", "3.2","3.4","ADC1_6"},
//    {"NK2239A_tp2", "", "3.5","3.7","ADC3_13"},
//    {"NK2239A_tp3", "", "6.6","6.9","ADC3_15"},
//    {"NK2239A_0116", "", "7.1","7.7","ADC3_16"},
//    {"打印SN", "", "","",""},
//    {"检测SN", "", "","",""},
//    {""},
//};

// XD01A
static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item             stardvalue minvalue maxvalue result
    {"用户", "", "","",""},
    {"SN编码", "", "","",""},
    {"PowerPCBA_SN", "", "","",""},//电源板
    {"AsicPCBA_SN", "", "","",""},//ASIC板
    {"SignalPCBA_SN", "", "","",""},//信号板
    {"版本文件核对", "", "","",""},
    {"版本文件烧写", "", "","",""},
    {"固件校验", "", "","",""},
    {"asic_version", "", "","",""},
    {"Power_1_tp2", "", "4.8","5.2","ADC1_9"},
    {"Asic_1_tp1", "", "1.1","1.3","ADC1_5"},
    {"Asic_1_tp2", "", "1.7","1.9","ADC3_1"},
    {"Asic_1_tp3", "", "1.4","1.6","ADC1_2"},
    {"Asic_1_tp5", "", "3.2","3.4","ADC1_6"},
    {"Signal_1_tp2", "", "3.5","3.7","ADC3_13"},
    {"Signal_1_tp3", "", "6.6","6.9","ADC3_15"},
    {"Signal_1_tp16", "", "7.1","7.7","ADC3_16"},
    {"打印SN", "", "","",""},
    {"检测SN", "", "","",""},
    {""},
};


// XD02A/XD03A/XD01B
static MesCheckItem g_mesCheckPackage1[MESMAXCHECKNUM] = {
    //item             stardvalue minvalue maxvalue result
    {"用户", "", "","",""},
    {"SN编码", "", "","",""},
    {"PowerPCBA_SN", "", "","",""},//电源板
    {"AsicPCBA_SN", "", "","",""},//ASIC板
    {"版本文件核对", "", "","",""},
    {"版本文件烧写", "", "","",""},
    {"固件校验", "", "","",""},
    {"asic_version", "", "","",""},
    {"Power_tp4", "", "4.8","5.2","ADC1_9"},
    {"Power_tp6", "", "1.1","1.3","ADC1_5"},
    {"Power_tp5", "", "1.7","1.9","ADC3_1"},
    {"Power_tp10", "", "7.3","7.7","ADC3_15"},
    {"Asic_tp30", "", "3.2","3.4","ADC1_2"},
    {"Asic_tp2", "", "3.5","3.7","ADC1_6"},
    {"Asic_tp3", "", "6.6","6.9","ADC3_16"},
    {"Asic_tp36", "", "1.1","1.3","ADC3_4"},
    {"Asic_tp34", "", "4.8","5.2","ADC3_13"},
    {"打印SN", "", "","",""},
    {"检测SN", "", "","",""},
    {""},
};

const char P_5V_OUT[] = {"P_5V_OUT"};
const char ADC1_2[] = {"ADC1_2"};
const char ADC1_5[] = {"ADC1_5"};
const char ADC1_6[] = {"ADC1_6"};
const char ADC1_9[] = {"ADC1_9"};
const char ADC1_10[] = {"ADC1_10"};
const char ADC1_16[] = {"ADC1_16"};
const char ADC2_2[] = {"ADC2_2"};
const char ADC2_6[] = {"ADC2_6"};
const char ADC3_0[] = {"ADC3_0"};
const char ADC3_1[] = {"ADC3_1"};
const char ADC3_4[] = {"ADC3_4"};
const char ADC3_5[] = {"ADC3_5"};
const char ADC3_7[] = {"ADC3_7"};
const char ADC3_9[] = {"ADC3_9"};
const char ADC3_13[] = {"ADC3_13"};
const char ADC3_15[] = {"ADC3_15"};
const char ADC3_16[] = {"ADC3_16"};

static double getVoltage(const char* nameIO, McuWorkInfo infor)
{
    if (memcmp(nameIO, P_5V_OUT, strlen(P_5V_OUT)) == 0) {
        return infor.volOut;
    }
    if (memcmp(nameIO, ADC1_2, strlen(ADC1_2)) == 0) {
        return infor.volAdc1_inp2;
    }
    if (memcmp(nameIO, ADC1_5, strlen(ADC1_5)) == 0) {
        return infor.volAdc1_inp5;
    }
    if (memcmp(nameIO, ADC1_6, strlen(ADC1_6)) == 0) {
        return infor.volAdc1_inp6;
    }
    if (memcmp(nameIO, ADC1_9, strlen(ADC1_9)) == 0) {
        return infor.volAdc1_inp9;
    }
    if (memcmp(nameIO, ADC1_10, strlen(ADC1_10)) == 0) {
        return infor.volAdc1_inp10;
    }
    if (memcmp(nameIO, ADC1_16, strlen(nameIO)) == 0) {
        return infor.volAdc1_inp16;
    }
    if (memcmp(nameIO, ADC2_2, strlen(nameIO)) == 0) {
        return infor.volAdc2_inp2;
    }
    if (memcmp(nameIO, ADC2_6, strlen(nameIO)) == 0) {
        return infor.volAdc2_inp6;
    }
    if (memcmp(nameIO, ADC3_0, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp0;
    }
    if (memcmp(nameIO, ADC3_1, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp1;
    }
    if (memcmp(nameIO, ADC3_4, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp4;
    }
    if (memcmp(nameIO, ADC3_5, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp5;
    }
    if (memcmp(nameIO, ADC3_7, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp7;
    }
    if (memcmp(nameIO, ADC3_9, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp9;
    }
    if (memcmp(nameIO, ADC3_13, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp13;
    }
    if (memcmp(nameIO, ADC3_15, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp15;
    }
    if (memcmp(nameIO, ADC3_16, strlen(nameIO)) == 0) {
        return infor.volAdc3_inp16;
    }

    return -9999;
}

BurningDe1000::BurningDe1000()
{
    ADDCONFIG("McuCom", "COM4", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("ASICCom", "COM4", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否等待夹具", "1", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否烧录", "1","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否网络获取SN", "1","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("PythonPath", "D:/tool/python31","FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("PowerEXP", "","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("ASICEXP", "","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("SingleEXP", "","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("模板路径", "E:", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("本地版本路径", "E:", "FilePath",LOGIN_ADMIN,"", true);
    ADDCONFIG("Python烧写", "0", "String",LOGIN_ADMIN,"", true);
    ADDCONFIG("硬件版本号", "HW_V[001]", "String",LOGIN_ADMIN,"", true);
    ADDCONFIG("旧MCU板", "True", "Enum",LOGIN_ADMIN,"True,False", true);
    if(projectName().contains("XD01A")) {
        m_pItem = g_mesCheckPackage;
    }else {
        m_pItem = g_mesCheckPackage1;
    }
    connect(pMachine(), &QStateMachine::stopped,
            this, &BurningDe1000::slotsStoped);
    add(0, "start", std::bind(&BurningDe1000::step0, this));
    add(1, "getSn", std::bind(&BurningDe1000::getSn, this));
    add(10, "judgeCustomLifeTime", std::bind(&BurningDe1000::jugdeCustomLifeTime, this, true));
    add(20, "enterMes", std::bind(&BurningDe1000::enterMes, this));
    add(50, "step4", std::bind(&BurningDe1000::getSnPCBPower, this));
    add(60, "step5", std::bind(&BurningDe1000::getSnPCBAsic, this));
    if(projectName().contains("XD01A")) {
        add(70, "step6", std::bind(&BurningDe1000::getSnPCBSignal, this));
    }
    add(75, "step6", std::bind(&BurningDe1000::fileCheck, this));
    add(80, "step6", std::bind(&BurningDe1000::checkJiaju, this));
    add(90, "step6", std::bind(&BurningDe1000::burning, this));
    add(91, "step6", std::bind(&BurningDe1000::getWorkInfo, this));
    add(92, "step6", std::bind(&BurningDe1000::asicFileCheck, this));
    add(110, "step7", std::bind(&BurningDe1000::comAsicVersion, this));
    add(100, "step3", std::bind(&BurningDe1000::comVol, this));
    add(130, "moveto111", std::bind(&BurningDe1000::printSN, this));
    add(140, "step2", std::bind(&BurningDe1000::checkSN, this));
    add(150, "step2", std::bind(&BurningDe1000::outMes, this));
    add(200, "step2", std::bind(&BurningDe1000::stepEnd, this));

    QString filePath = QCoreApplication::applicationDirPath() + "/cvs";
    QString timeStr = QDateTime::currentDateTime().toString("yyyy_MM_dd");
    QString fileDir = filePath + "/" + timeStr;
    Util::createFileDir(fileDir);

    QStringList logTitles;
    logTitles << "SN";
    logTitles << "开始时间";
    logTitles << "结束时间";
    logTitles << "测试时间";
    logTitles << "测试结果";


    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(m_pItem[i].sItem.size() <= 0) {
            break;
        }
        logTitles << m_pItem[i].sItem;
    }

    QString filePathOut = GETCONFIG("LogPath") + "/data.csv";
    m_allCvsLog.setCsvLogPath(filePathOut);
    QFile file(filePathOut);
    if(!file.exists()) {
        m_allCvsLog.addCsvTitle(logTitles);
    }
}

void BurningDe1000::printSN()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }

    MesCheckItem *pItem = findItem("打印SN");
    if(!pItem) {
        return;
    }
    QElapsedTimer tStart;
    tStart.start();
    showProcess("打印SN");
    pItem->sValue = m_sn;
    pItem->sResult = MESFAILED;
    pItem->sExp = m_sn;
    QString mTemplatePath = GETCONFIG("模板路径");
    QFile file(mTemplatePath);
    if (!file.exists()) {
        m_errorCode = -4;
        m_errMsg = "打开模板文件失败";
        logFail("打开模板文件失败");
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
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
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
        return ;
    }
    QString sCodeId = "ID00X";
    if (!printer.setItem(sCodeId, m_sn)) {
        logFail("替换贴纸SN失败");
        m_errorCode = -4;
        logFail("替换贴纸SN失败");
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
        return ;
    }
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    pItem->sValue = m_sn;
    pItem->sResult = MESPASS;
    logNormal("开始打印标签");
    printer.print();
    logNormal("完成打印标签");
    showItem(*pItem);

}

void BurningDe1000::checkSN()
{
    CHECKSTATUS;
    if(isGoldenMode()) {
        return;
    }
    QElapsedTimer tStart;
    tStart.start();

    MesCheckItem *pItem = findItem("检测SN");
    if(!pItem) {
        return;
    }
    pItem->sValue = MESFAILED;
    pItem->sExp = m_sn;
    pItem->sResult = MESFAILED;
    showProcess("扫码SN 检测核对");
    logNormal("开始扫码SN");
    QString sn = msgBox("请扫码SN");
    pItem->sValue = sn;
    if(!m_workStatus) {
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
        return;
    }
    logNormal("扫码SN " + sn);
    if(sn != ConfigInfo::getInstance()->cacheInfo().sSn) {
        m_errorCode = -5;
        m_errMsg = "扫码比对失败";
        logFail("扫码SN失败 ");
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
        return;
    }
    pItem->sResult = MESPASS;
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningDe1000::getWorkInfo()
{
    CHECKSTATUS
    QString sSerialNameMcu = GETCONFIG("McuCom");
    logNormal("Start open mcu serial...");
    mFixtureMcu->closeSerial();
    QThread::sleep(1);
    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
        logFail("开启MCU串口失败");
        m_errorCode = -2;
        return;
    }
    if (!mFixtureMcu->controlSensorPower(0)) {
        mFixtureMcu->closeSerial();
        m_errMsg = "给ASIC下电电失败";
        logFail(m_errMsg);
        m_errorCode = -2;
        return;
    }
    QThread::sleep(1);
    logNormal("Start power up...");
    if (!mFixtureMcu->controlSensorPower(12, true, m_bOldMcu)) {
        m_errMsg = "给ASIC上电失败";
        mFixtureMcu->closeSerial();
        logFail(m_errMsg);
        m_errorCode = -2;
        return;
    }

    QString sSerialNameAsic = ConfigInfo::getInstance()->getValueString("ASICCom");
    mAsicDbgSerial->closeSerial();
    QThread::sleep(10);// 等待产品启动
    logNormal("open Asic Serial...");
    if (!mAsicDbgSerial->openSerial(sSerialNameAsic)) {
        logFail("开启ASIC串口失败");
        m_errorCode = -2;
        return;
    }

    logNormal("Waitting asic power up in 8s...");
    if (!mAsicDbgSerial->waitForStartup(8000)) {
        mAsicDbgSerial->closeSerial();
        logFail("ASIC启动失败");
        m_errorCode = -2;
        return;
    }


    if(!isGoldenMode()) {
        QString sCrc;
        m_crcAll.clear();
        if (!mAsicDbgSerial->getCrc(sCrc)) {
            logFail("Get sCrc Infor Failed");
            m_errorCode = -2;
            mFixtureMcu->closeSerial();
            return;
        }else {
            m_crcAll = sCrc;
        }

        logNormal("Read asic version..." + sSerialNameAsic);
        if (!mAsicDbgSerial->readVersion(mCrtVersion)) {
            mAsicDbgSerial->closeSerial();
            logFail("ASIC版本读取失败" + sSerialNameAsic);
            m_errorCode = -2;
            return;
        }
        if(projectName().contains("XD02A")) {
            mCrtVersion.replace('_','.');
        }

        logNormal("read version" + mCrtVersion);

        QString hwVersion = GETCONFIG("硬件版本号");
        if (!hwVersion.isEmpty()) {
            logNormal("Read asic hw version..." + sSerialNameAsic);
            if (!mAsicDbgSerial->readHwVersion(m_readHwVersion)) {
                mAsicDbgSerial->closeSerial();
                logFail("ASIC版本读取失败"+sSerialNameAsic);
                m_errorCode = -2;
                return;
            }
            logNormal("read hw version " + m_readHwVersion);
        }

        logNormal("Write asic SN " + m_sn);
        if (!mAsicDbgSerial->writeSN(m_sn)) {
            mAsicDbgSerial->closeSerial();
            logFail("写入sn失败");
            m_errorCode = -2;
            return;
        }
    }

    McuWorkInfo mcuInfor;
    if (!mFixtureMcu->getMcuWorkInfo(mcuInfor)) {
        logFail("Get Mcu Voltage Infor Failed");
        m_errorCode = -2;
        mFixtureMcu->closeSerial();
        return;
    }
    m_mcuInfo = mcuInfor;

    // 0. 控制ASIC下电
    logNormal("Start control aisc pcb power off...");
    if (!mFixtureMcu->controlSensorPower(0)) {
        logFail("ASIC下电失败");
        m_errorCode = -2;
        mFixtureMcu->closeSerial();
        return;
    }
    // -1. 关闭ASIC串口
    showProcess("获取信息完成");
}

void BurningDe1000::getSnPCBPower()
{
    CHECKSTATUS
    QElapsedTimer tStart;
    tStart.start();
    showProcess("开始扫码电源板");
    logNormal("开始扫码电源板");
    QString sn = msgBox("开始扫码电源板 SN");
    CHECKSTATUS
    tableClearData();
    mSnPCBPower = sn;
    m_pItem[2].sResult = MESFAILED;
    m_pItem[2].sValue = mSnPCBPower;
    m_pItem[2].sExp = GETCONFIG("PowerEXP").toUpper();
    if(m_pItem[2].sExp.size() > 0 && !sn.contains(m_pItem[2].sExp)) {
        logFail("开始扫码电源板 failed");
        m_pItem[2].tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(m_pItem[2]);
        m_errorCode = -1;
        m_errMsg = "扫码电源板FAIL";
        return;
    }
    logNormal("扫码电源板 " + sn);

    m_pItem[2].sResult = MESPASS;
    m_pItem[2].tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(m_pItem[2]);
}

void BurningDe1000::getSnPCBAsic()
{
    CHECKSTATUS
    showProcess("开始扫码Asic板");
    QElapsedTimer tStart;
    tStart.start();
    logNormal("开始扫码Asic板");
    QString sn = msgBox("开始扫码Asic板");
    CHECKSTATUS
    m_pItem[3].sResult = MESPASS;
    m_pItem[3].sExp = GETCONFIG("ASICEXP").toUpper();
    if(m_pItem[3].sExp.size() > 0 && !sn.contains(m_pItem[3].sExp.toUpper())) {
        logFail("扫码Asic板 failed");
        m_pItem[3].sResult = MESFAILED;
        m_pItem[3].tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(m_pItem[3]);
        m_errorCode = -1;
        m_errMsg = "扫码ASIC板FAIL";
        return;
    }
    logNormal("扫码Asic板 " + sn);
    mSnPCBAsic = sn;
    m_pItem[3].sExp = ConfigInfo::getInstance()->getValueString("ASICEXP").toUpper();
    m_pItem[3].sValue = mSnPCBAsic;
    m_pItem[3].tCheckTm = QString::number(tStart.elapsed()/1000);
    m_pItem[3].sResult = MESPASS;
    showItem(m_pItem[3]);
}

void BurningDe1000::getSnPCBSignal()
{
    CHECKSTATUS
    QElapsedTimer tStart;
    tStart.start();
    showProcess("开始扫码信号板");
    logNormal("开始扫码信号板");
    QString sn = msgBox("开始扫码信号板");
    CHECKSTATUS
    m_pItem[4].sResult = MESPASS;
    if(m_pItem[4].sExp.size() > 0 && !sn.contains(GETCONFIG("SingleEXP").toUpper())) {
        logFail("开始扫码信号板 failed");
        m_errorCode = -9;
        m_errMsg = "扫码信号板FAIL";
        m_pItem[4].sResult = MESFAILED;
        m_pItem[4].tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(m_pItem[4]);
        return;
    }
    logNormal("开始扫码信号板 " + sn);
    mSnPCBSignal = sn;
    QString exp = ConfigInfo::getInstance()->getValueString("SingleEXP").toUpper();
    m_pItem[4].sValue = mSnPCBSignal;
    m_pItem[4].sResult = MESPASS;
    m_pItem[4].sExp = exp;
    m_pItem[4].tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(m_pItem[4]);
}

void BurningDe1000::checkJiaju()
{
    CHECKSTATUS
    if(0 == GETCONFIG("是否等待夹具").toInt()) {
        return;
    }
    MesCheckItem tem;
    tem.sItem = "下压夹具";
    tem.sResult = MESFAILED;
    QElapsedTimer tStart;
    tStart.start();

    QString sSerialNameMcu = GETCONFIG("McuCom");
    logNormal("Start open mcu serial..." + sSerialNameMcu);
    mFixtureMcu->closeSerial();
    QThread::sleep(1);
    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
        logFail("开启MCU串口失败");
        tem.sResult = MESFAILED;
        tem.tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(tem);
        return;
    }
    showProcess("请开始下压夹具");

    logNormal("Start detect the fixture pressure");
    logNormal("自动侦测夹具下压，请开始下压夹具...");
    int iDetectOk = 1;

    while (!mFixtureMcu->pressed() ) {
        if(!m_workStatus) {
            iDetectOk = 0;
            break;
        }
    }
    logNormal(QString("当前工装MCU版本位: %1").arg(mFixtureMcu->version(), 2, QLatin1Char('0')));
    mFixtureMcu->closeSerial();
    if(!iDetectOk) {
        logFail("夹具检测失败");
        tem.sResult = MESFAILED;
        tem.tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(tem);
        return;
    }
    tem.sValue = MESPASS;
    tem.sResult = MESPASS;
    tem.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(tem);
}

bool BurningDe1000::burnWithPython()
{
    QString workDir = QCoreApplication::applicationDirPath() + "/asic_burn";
    QString PythonPath = ConfigInfo::getInstance()->getValueString("PythonPath");

    QProcess process;
    process.start("cmd");
    QString scmd = QString::asprintf("cd /d %s\n", workDir.toStdString().data());
    qInfo()<<scmd;
    process.write(scmd.toStdString().data());
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    scmd = QString::asprintf("%s/python.exe asicburn.py \"%s\" %s\n",
                             PythonPath.toStdString().data(),
                             m_versionPath.toStdString().data(),
                             info.sCodeId.toStdString().data());
    qInfo()<<scmd;
    process.write(scmd.toStdString().data());
    process.write("exit\n");
    int iCount = 3000000;
    QThread::msleep(2000);
    while(iCount > 0) {
        iCount -- ;
        if(process.waitForReadyRead(10)) {
            QString sOut = QString::fromLocal8Bit(process.readAll());
            qInfo("recv %s", sOut.toStdString().data());
            logNormal(sOut);
            if(sOut.contains("show")) {
                QString msg = sOut.split("show").last();

                showProcess("烧写"+msg.split('\n').at(0));
            }

        }
        if(process.waitForFinished(10)) {
            qInfo("finish....");
            break;
        }
    }
    process.waitForFinished();
    int iRes = process.exitCode();
    if(!iRes) {
        logNormal("烧写成功");
        return true;
    }else {
        QTextCodec* code = QTextCodec::codecForName("gbk");
        QString err = code->toUnicode(process.readAllStandardError());
        QString out = code->toUnicode(process.readAllStandardOutput());
        logFail("烧写失败 " + QString::number(iRes) + err);
        logFail("烧写失败 " + QString::number(iRes) + out);
    }

    return false;
}

bool BurningDe1000::burnWithLocal()
{
    QString sSerialNameMcu = GETCONFIG("McuCom");
    QString sSerialNameAsic = ConfigInfo::getInstance()->getValueString("ASICCom");

    // MCU操作
    logNormal("打开 MCU 治具板串口 " + sSerialNameMcu);
    mFixtureMcu->closeSerial();
    QThread::sleep(1);
    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
        logFail("开启MCU串口失败");
        m_errorCode = -2;
        return false;
    }
    logNormal("断电 ");
    if (!mFixtureMcu->controlSensorPower(0)) {
        m_errMsg = "给ASIC下电电失败";
        logFail(m_errMsg);
        m_errorCode = -2;
        return false;
    }
    QThread::msleep(100);

    logNormal("打开 ASIC 烧录串口 " + sSerialNameAsic);
    mAsicDbgSerial->closeSerial();
    if (!mAsicDbgSerial->openSerial(sSerialNameAsic)) {
        logFail("开启ASIC串口失败");
        m_errorCode = -2;
        return false;
    }

    logNormal("上电 ");
    // 最后一个参数需要根据MCU的最新版本做适配
    // 最新版MCU为false，老版本为True
    // MCU加入了一个新的功能：阶梯式上电，以解决EOL宽电压测试的问题
    // 目前来看阶梯式上上电并不适用于烧录，烧录需要瞬间上电
    if (!mFixtureMcu->controlSensorPower(12, false, m_bOldMcu)) {
        m_errMsg = "给ASIC上电失败";
        logFail(m_errMsg);
        m_errorCode = -2;
        return false;
    }

    logNormal("开始进入调试模式[烧录前操作]...");
    if (mAsicDbgSerial->enterDebug(5000)) {
        logNormal("进入Debug模式OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "进入Debug模式失败";
        logFail(m_errMsg);
        return false;
    }

    logNormal("烧写load bin文件[烧录前操作]...");
    if (mAsicDbgSerial->burnFile(m_versionPath + "/dh5100_load.bin", 0xFFFE0000)) {
        logNormal("烧录load bin文件OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录load bin文件失败";
        logFail(m_errMsg);
        return false;
    }

    logNormal("烧写load bin文件完成指令[烧录前操作]...");
    if (mAsicDbgSerial->burnFileEx(0xFFFE0000)) {
        logNormal("烧录load Ex指令OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录load 指令失败";
        logFail(m_errMsg);
        return false;
    }

    logNormal("开始进入mb #模式[烧录前操作]...");
    if (mAsicDbgSerial->enterMbMode(3000)) {
        logNormal("MB 模式进入OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "进入mb #模式失败";
        logFail(m_errMsg);
        return false;
    }

    logNormal("开始灌入ptz指令[烧录前操作]...");
    if (mAsicDbgSerial->enterPtzMode(3000)) {
        logNormal("Ptz 模式进入OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "灌入ptz指令失败";
        logFail(m_errMsg);
        return false;
    }

    logNormal("再次进入调试模式[烧录前操作]...");
    if (mAsicDbgSerial->enterDebug(5000)) {
        logNormal("进入Debug模式OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "进入Debug模式失败";
        logFail(m_errMsg);
        return false;
    }

    if (mAsicDbgSerial->burnFile(m_versionPath + "/mboot_cpu162_ddr162_axi81.fimg", 0x30000000)) {
        logNormal("烧录mboot_cpu162_ddr162_axi81.fimg文件OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录mboot_cpu162_ddr162_axi81.fimg文件失败";
        logFail(m_errMsg);
        return false;
    }

    if (mAsicDbgSerial->burnFile(m_versionPath + "/dh5100_test.bin.uimg", 0x30010000)) {
        logNormal("烧录dh5100_test.bin.uimg文件OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "dh5100_test.bin.uimg文件失败";
        logFail(m_errMsg);
        return false;
    }
    if (!projectName().contains("XD01A")) {
        if (mAsicDbgSerial->burnFile(m_versionPath + "/startimg.bin", 0x30580000)) {
            logNormal("烧录startimg.bin文件OK");
        } else {
            m_errorCode = -2;
            m_errMsg = "startimg.bin文件失败";
            logFail(m_errMsg);
            return false;
        }
    }
    if (mAsicDbgSerial->burnFile(m_versionPath + "/PseudoTable_YUV.bin", 0x30600000)) {
        logNormal("烧录PseudoTable_YUV.bin文件OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录PseudoTable_YUV.bin文件失败";
        logFail(m_errMsg);
        return false;
    }
    if (mAsicDbgSerial->burnFile(m_versionPath + "/flashconfig.bin", 0x30612000)) {
        logNormal("烧录flashconfig.bin文件OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录flashconfig.bin文件失败";
        logFail(m_errMsg);
        return false;
    }
    if (mAsicDbgSerial->burnFileEx(0x30612000)) {
        logNormal("烧录flashconfig Ex指令OK");
    } else {
        m_errorCode = -2;
        m_errMsg = "烧录flashconfig Ex指令失败";
        logFail(m_errMsg);
        return false;
    }


    mFixtureMcu->controlSensorPower(0);
    mAsicDbgSerial->closeSerial();
    mFixtureMcu->closeSerial();
    return true;
}

void BurningDe1000::burning()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }
    if(!GETCONFIG("是否烧录").toInt()) {
        return;
    }

    QElapsedTimer tStart;
    tStart.start();

    MesCheckItem *pItem = findItem("版本文件烧写");
    if(!pItem) {
        return;
    }
    showProcess("烧录中...");


    int burnWay = GETCONFIG("Python烧写").toInt();

    mAsicDbgSerial->closeSerial();
    mFixtureMcu->closeSerial();
    bool bRst = false;
    if (burnWay == 0) {
        bRst = burnWithLocal();
    } else {
        bRst = burnWithPython();
    }

    if (!bRst) {
        m_errorCode = -2;
        m_errMsg = "烧写失败";
        pItem->sResult = MESFAILED;
        pItem->sValue = MESFAILED;
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
    } else {
        pItem->sResult = MESPASS;
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
    }

}

void BurningDe1000::comAsicVersion()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }
    MesCheckItem *pItem = findItem("asic_version");
    if(!pItem) {
        return;
    }
    QString hwVersion = GETCONFIG("硬件版本号");
    QElapsedTimer tStart;
    tStart.start();
    pItem->sResult = MESFAILED;
    showProcess("比对版本号");

    mCrtVersion.replace("_", ".");

    logNormal(mCrtVersion + " " + mSoftVersion );
    pItem->sValue = mCrtVersion;
    pItem->sResult = MESPASS;
    pItem->sExp = mSoftVersion;
    if(mCrtVersion != mSoftVersion) {
        m_errorCode = -2;
        m_errMsg = "版本号比对失败";
        logFail("版本号比对失败");
        pItem->sResult = MESFAILED;
    }

    if (hwVersion != m_readHwVersion) {
        logFail("硬件版本号检测失败");
        pItem->sResult = MESFAILED;
    }

    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningDe1000::saveResult()
{

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_workingTm;
    // save log
    double cost = (double)elapsed / 1000.0;
    logNormal("Test Cost: " + QString::number(cost, 'f', 2));

    QDateTime crtDateTime = QDateTime::currentDateTime();
    mTestEndTimeStr = crtDateTime.toString("yyyy/MM/dd hh:mm:ss:zzz");
    QStringList logData;

    logData << m_sn
                << mTestStartTimeStr
                << mTestEndTimeStr
                << QString::number(cost, 'f', 2)
                << (m_errorCode == 0 ? "PASS" : "FAIL");
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(m_pItem[i].sItem.size() <= 0) {
            break;
        }
        logData << m_pItem[i].sValue;
    }

    m_allCvsLog.addCsvLog(logData);
    QString logPath = GETCONFIG("LogPath");
    logPath += "/" + QDateTime::currentDateTime().toString("yyyy_MM_dd");
    logPath += "/" + m_sn + ".csv";
    tableSaveCsv(logPath);
}

void BurningDe1000::comVol()
{
    CHECKSTATUS
    QElapsedTimer tStart;
    tStart.start();
    showProcess("比对电压");
    // getWorkInfo();
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        QString decItem = m_pItem[i].sItem;
        if(decItem.size() <= 0 || m_pItem[i].sMin.size() <= 0) {
            continue;
        }
        double value = getVoltage(m_pItem[i].sExp.toLocal8Bit().data(), m_mcuInfo);
        int iOk = 0;
        m_pItem[i].sValue = QString::asprintf("%.2f", value);
        if(value + 1e-5 < m_pItem[i].sMin.toDouble() ||
                value - 1e-5 > m_pItem[i].sMax.toDouble() ){
            QString log = QString("Item: %1 \tVoltage: %3 [%4, %5] FAILED").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(m_pItem[i].sMin)
                    .arg(m_pItem[i].sMax);
            logFail(log);
            m_pItem[i].sResult = MESFAILED;
            m_errorCode = -3;
            m_errMsg = "电压测试失败";
            iOk = 1;
        }else {
            QString log = QString("Item: %1 \tVoltage: %3 [%4, %5] PASS").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(m_pItem[i].sMin)
                    .arg(m_pItem[i].sMax);
            logNormal(log);
            m_pItem[i].sResult = MESPASS;
        }
        m_pItem[i].tCheckTm = QString::number(tStart.elapsed()/1000);;
        showItem(m_pItem[i]);
    }
}

void BurningDe1000::stepEnd()
{

    saveResult();
    if(m_errorCode == 0) {
        showPass();
    }else {
        showFail("NG " + m_errMsg);
    }
    msgBox("请更换产品，执行下一轮测试", 4);
    stopProcess();
}

void BurningDe1000::start()
{

    showProcess("开始检测");
    startProcess();
}

void BurningDe1000::slotsStoped()
{
    if(m_workStatus) {
        startProcess();
    }
}

void BurningDe1000::showItem(MesCheckItem item)
{
    QStringList itemString;
    itemString << item.sItem;
    itemString << item.sValue;
    if(item.sMin.size() > 0 && item.sMax.size() > 0) {
        itemString << (item.sMin + "~"+item.sMax);
    }else {
        if(item.sExp.size()<= 0) {
            itemString<<"/";
        }else {
            itemString << item.sExp;
        }
    }
    itemString << item.tCheckTm;
    itemString << item.sResult;

    tRowData(itemString);
    if(item.sResult == MESFAILED) {
        tableUpdateTestStatus(1);
    }
    if(item.sResult == MESPASS) {
        tableUpdateTestStatus(0);
    }
}

void BurningDe1000::slotStartWorkBtnClicked()
{
    tClear();
    m_errMsg.clear();
    mVoltageList.clear();
    logClear();
    QDateTime dateTime = QDateTime::currentDateTime();
    mTestStartTimeStr = dateTime.toString("yyyy/MM/dd hh:mm:ss:zzz");
    m_workStatus = true;
    m_workingTm = QDateTime::currentMSecsSinceEpoch();
    start();
}

void BurningDe1000::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_workStatus = false;
    m_signal.signal();
    hideMsgBox();
    showProcess("请先点检环境是否正常");
}

void BurningDe1000::slotConnectBtnClicked()
{

    emit tableClearData();
    emit logClearData();

    emit resultShowDefault();
    QStringList headTitles;
    headTitles << "检测项";
    headTitles << "实际值";
    headTitles << "期望值";
    headTitles << "测试时间";
    headTitles << "测试结果";

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "旧MCU板", m_bOldMcu);
    logNormal((m_bOldMcu ? "使用旧MCU板子" : "使用新MCU板子"));

    tHeader(headTitles);
    if(!mFixtureMcu) {
        mFixtureMcu = new McuSerialManager();
    }
    if(!mAsicDbgSerial) {
        mAsicDbgSerial = new ASicSerialManager;
    }
    connect(mAsicDbgSerial, &ASicSerialManager::sigLogOut, this, &BurningDe1000::showProcess);
    mFixtureMcu->closeSerial();
    if(!mFixtureMcu->openSerial(GETCONFIG("McuCom"))) {
        emit showMsgBox("MCU 端口打开失败，请确认配置", 3);
        if(!isDebugMode()) {
            return;
        }
    }
    mFixtureMcu->closeSerial();

    mAsicDbgSerial->closeSerial();
    if(!mAsicDbgSerial->openSerial(GETCONFIG("ASICCom"))) {
        emit showMsgBox("ASIC 端口打开失败，请确认配置", 3);
        if("调试模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {
            return;
        }
    }
    mAsicDbgSerial->closeSerial();

    BaseProduce::slotConnectBtnClicked();
}

void BurningDe1000::getSn()
{
    CHECKSTATUS
    //只有生产模式存在
    if(!isOnlineMode()) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        m_sn = msgBox("请扫码下一个SN");
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
        logNormal("获取整机SN: " + m_sn);
        return;
    }
    showProcess("获取 SN");
    QString sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    if(ConfigInfo::getInstance()->getValueString("是否网络获取SN").toInt() &&
            (info.sCodeId == "DE1000" || info.sCodeId == "DE1100")) {
        if(0 != MesCom::instance()->getOrderSns(info.orderId, sOut)) {
            logFail("获取SN失败" + sOut);
            m_errorCode = -101;
            return;
        }else {
            m_sn = sOut;
        }
    }else {
        m_sn = msgBox("请扫码下一个sn");
    }

    logNormal("获取整机SN: " + m_sn);
    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);

}

void BurningDe1000::outMes()
{
    CHECKSTATUS
    //只有生产模式存在
    if(isOnlineMode() && !m_skipMesUpload) {
        QString sMes;
        MesCom::instance()->setItemResult(m_pItem);
        if(!MesCom::instance()->outProduce((m_errorCode == 0) ? 0 : 1, sMes)) {
            logFail("出站失败 " + sMes);
            m_errorCode = -15;
            m_errMsg = "出栈失败：" + sMes;
        }else {
            logNormal("出站成功");
        }
    }else if("点检模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {//点检模式上传
        if(m_checkItem) {

        }
    }
}

void BurningDe1000::enterMes()
{
    CHECKSTATUS
    //只有生产模式存在
    if(!isOnlineMode()) {
        return;
    }
    MesCheckItem tem;
    tem.sItem = "编码检测";
    tem.sResult = MESFAILED;
    tem.sValue=m_sn;
    QElapsedTimer tStart;
    tStart.start();
    MesCom::instance()->setUser(MesClient::instance()->sUser());
    QString sMes;
    m_skipMesUpload = false;
    if(!MesCom::instance()->enterProduce(m_sn, sMes)) {
        logFail("进站失败" + sMes);
        m_errorCode = -16;
        m_skipMesUpload = true;
    }else {
        tem.sResult=MESPASS;
    }
    showItem(tem);
}



void BurningDe1000::step0()
{
    m_workingTm = QDateTime::currentMSecsSinceEpoch();
    m_errorCode = 0;
    m_errMsg.clear();
    mSnPCBPower.clear();
    mSnPCBAsic.clear();
    mSnPCBSignal.clear();
    mCrtVersion.clear();
    m_readHwVersion.clear();
    mCrtVersion.clear();

    m_crcAll.clear();
    m_crcConfig = 0;
    m_crcApp = 0;
    m_crcBoot = 0;
    m_crcSting = 0;
    logClear();

//    int mode = mAsicDbgSerial->getProductMode(15000);
//    qDebug() << "current mode is " << mode;
//    if (mode == 1) {// 需要擦除后再烧录
//        logNormal("已烧录过程序，进行擦除操作");
//        if (mAsicDbgSerial->enterMbMode(5000)) {
//            logNormal("进入MB模式OK");
//        }
//    } else if (mode == 2) {// 直接烧录

//    } else {
//        m_errorCode = -2;
//        logFail("查询烧录模式失败");
//        return;
//    }
}

void BurningDe1000::fileCheck()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }
    MesCheckItem *pItem = findItem("版本文件核对");
    if(!pItem) {
        m_errorCode = -2;
        m_errMsg = "无版本文件比对计划";
        return;
    }
    QString workDir = QCoreApplication::applicationDirPath() + "/asic_burn";

    bool bRst = false;
    if(!isOnlineMode()) {
        m_versionPath = GETCONFIG("本地版本路径");
        mSoftVersion = m_versionPath.split("/").last();
        bRst = true;
    }else {
        logNormal("在线模式从MES下载版本文件");
        mSoftVersion.clear();
        bRst = MesCom::instance()->getUpgradeFileVersion(mSoftVersion, "asic");
        m_versionPath = workDir + "/" + mSoftVersion;
        QString versionFilePath = m_versionPath + "/update.zip";
        bRst &= MesCom::instance()->getVersionFile(versionFilePath, "asic");
    }
    qInfo()<<"version path "<<m_versionPath;
    qInfo()<<"version "<<mSoftVersion;
    if (!bRst) {
        m_errorCode = -1;
        m_errMsg = "获取版本文件失败";
        logFail("获取版本文件失败");
        return;
    }


    if(isOnlineMode()){
        QString toolPath = QCoreApplication::applicationDirPath() + "/tools/zip/unzip.exe";
        QProcess processBat;
        processBat.start("cmd");
        QString scmd = QString::asprintf("cd /d \"%s/%s\"\n", workDir.toStdString().data(),
                                                mSoftVersion.toStdString().data());
        processBat.write(scmd.toStdString().data());
        processBat.write(QString("\"" + toolPath + "\"" + " -o update.zip\n").toStdString().data());
        processBat.write("exit\n");
        processBat.waitForStarted();
        processBat.waitForFinished();
        qInfo("%s",QString::fromLocal8Bit(processBat.readAll()).toStdString().data());
    }
    QElapsedTimer tStart;
    tStart.start();
    pItem->sResult = MESFAILED;
    showProcess("版本文件核对");
    QFile file(m_versionPath+"/filelist.txt");
    file.open(QFile::ReadOnly);
    QByteArray sData;
    if(file.isOpen()) {
        sData = file.readAll();
        file.close();
    } else {
        m_errorCode = -1;
        m_errMsg = "CRC文件开启失败";
        logFail("CRC文件开启失败");
        return;
    }
    bool isCheckOk = false;
    if(sData.size() > 0) {
        QList<QByteArray> lItem = sData.split('\r');
        for(int i = 0; i < lItem.size(); i ++) {
            if(lItem.at(i).size() < 34) {
                continue;
            }
            QString sSrcMd5 = lItem.at(i).split('*').at(0);
            QString fileName = lItem.at(i).split('*').at(1);
            qInfo()<< "文件读取：" << sSrcMd5 << " -- " << fileName;
            qInfo()<< "开始计算：" << m_versionPath + "/" + fileName << " MD5/CRC";
            QString sDstMd5 = Util::md5sum(m_versionPath + "/" + fileName);
            quint32 sCrc = Util::crc32ChkCalc(m_versionPath + "/" + fileName);
            if(fileName.contains("dh5100_test.bin")) {
                m_crcApp = sCrc;
                if (m_crcApp == 0x00) {
                    m_errorCode = -1;
                    m_errMsg = "获取本地版本文件CRC失败";
                    logFail("获取本地版本文件CRC失败");
                    return;
                }
                logNormal("app 文件CRC：" + QString::number(sCrc, 16));
            }else if(fileName.contains("mboot_cpu")) {
                m_crcBoot = sCrc;
                if (m_crcBoot == 0x00) {
                    m_errorCode = -1;
                    m_errMsg = "获取本地版本文件CRC失败";
                    logFail("获取本地版本文件CRC失败");
                    return;
                }
                logNormal("boot 文件CRC：" + QString::number(sCrc, 16));
            }else if(fileName.contains("startimg")) {
                m_crcSting = sCrc;
                if (m_crcSting == 0x00) {
                    m_errorCode = -1;
                    m_errMsg = "获取本地版本文件CRC失败";
                    logFail("获取本地版本文件CRC失败");
                    return;
                }
                logNormal("startimg 文件CRC：" + QString::number(sCrc, 16));
            }else if(fileName.contains("Pseudo")) {
                m_crcWc = sCrc;
            }else if(fileName.contains("config")) {
                m_crcConfig = sCrc;
            }
            qInfo("crc %08x", sCrc);
            qInfo()<<sSrcMd5<<" "<<sDstMd5;
            if(!sSrcMd5.toUpper().contains(sDstMd5)) {
                m_errorCode = -1;
                m_errMsg = "本地版本文件CRC比对失败";
                logFail("本地版本文件CRC比对失败");
                return;
            }
        }
        isCheckOk = true;
    } else {
        isCheckOk = false;
        m_errorCode = -1;
        m_errMsg = "CRC文件为空";
        logFail("CRC文件为空");
    }

    pItem->sResult = isCheckOk ? MESPASS : MESFAILED;
    pItem->sValue = isCheckOk ? MESPASS : MESFAILED;
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningDe1000::asicFileCheck()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }

    showProcess("烧写文件比对");
    MesCheckItem item;
    item.sItem = "固件效验";
    item.sExp = MESPASS;
    item.sValue = MESFAILED;
    item.sResult = MESFAILED;
    QElapsedTimer tStart;
    // getWorkInfo();
    tStart.start();

    showProcess("固件效验");

    QString sApp = QString::asprintf("app:0x%x", m_crcApp);
    QString sBoot = QString::asprintf("boot:0x%x", m_crcBoot);
    QString sWc = QString::asprintf("wc:0x%x", m_crcWc);
    QString sSting = QString::asprintf("stimg:0x%x", m_crcSting);
    QString sConfig = QString::asprintf("config:0x%x", m_crcConfig);
    bool bRst = m_crcAll.contains(sApp);
    bRst &= m_crcAll.contains(sBoot);
    bRst &= m_crcAll.contains(sWc);
    if (!projectName().contains("XD01A")) {
        bRst &= m_crcAll.contains(sSting);
    }
    // bRst &= m_crcAll.contains(sConfig); // boot启动会修改config的内容，crc会不一样，忽略比对...
    if( bRst ){
        item.sValue = MESPASS;
        item.sResult = MESPASS;
    }else {
        m_errorCode = -9;
        m_errMsg = "固件校验失败";
        logFail("固件效验失败");
    }

    logNormal("产品固件：" + m_crcAll);
    logNormal("本地固件：" + sApp + sBoot + sWc + sSting + sConfig);

    showItem(item);
}

MesCheckItem *BurningDe1000::findItem(QString sItem)
{
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(m_pItem[i].sItem == sItem) {
            return &m_pItem[i];
        }
    }
    return nullptr;
}

#include "burningmcucb1000.h"
#include <functional>
#include <QMessageBox>
#include <QFile>
#include <CMyPrinter.h>
#include <QThread>
#include <util.h>
#include <QDateTime>
#include <QCoreApplication>
#include <QProcess>
#include <QElapsedTimer>

#define CURCODE ConfigInfo::getInstance()->getValueString("当前组件编码")
#define ADDCONFIG(name,value,type,pro,enumNames,bSave) \
{ \
    XmlConfigInfo info={CURCODE,name,value,type,pro,enumNames}; \
    ConfigInfo::getInstance()->addConfig(info, bSave); \
}

#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)

#define CHECKSTATUS \
{\
    if(m_errorCode!=0)\
        return;\
    if("调试模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {\
        if(!m_workStatus)\
        {\
            return;\
        }\
    }\
}

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item             stardvalue minvalue maxvalue result
    {"mcu_version", "V1.1.1", "","",""},
    {"mcu_tp6", "", "4.8","5.2","ADC1_9"},
    {"mcu_tp7", "", "3.2","3.4","ADC1_5"},
    {"mcu_tp8", "", "13","17","ADC3_15"},
    {"mcu_tp9", "", "13","17","ADC3_16"},
    {"mcu_tp10", "", "0.7","0.9","ADC3_1"},
    {"mcu_tp11", "", "0.7","0.9","ADC1_2"},
    {"mcu_tp12", "", "0.7","0.9","ADC1_6"},
    {"mcu_tp14", "", "1.7","1.9","ADC3_4"},
    {"mcu_tp16", "", "1.0","1.2","ADC3_13"},
    {"mcu_tp17", "", "1.7","1.9","ADC1_10"},
    {"mcu_tp18", "", "1.7","1.9","ADC3_5"},
    {"mcu_tp19", "", "1.0","1.2","ADC1_16"},
    {"mcu_tp25", "", "3.2","3.4","ADC3_9"},
    {"mcu_tp27", "", "0.7","0.9","ADC3_0"},
    {"mcu_tp66", "", "1.1","1.3","ADC3_7"},
    {"NK1272A_PCBA", "NK1272A*", "","",""},
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

// 注意：load.txt中的文件不能有空格，否则会测试失败
static void setLoad(QString rootPath, QString versionPath)
{
    QFile outFile(rootPath + "/load.txt");
    outFile.open(QFile::WriteOnly | QFile::Truncate);
    QString sBoot,sApp;
    sBoot.sprintf("%s/BOOT.bin", versionPath.toStdString().data());
    sApp.sprintf("%s/CAN_sample.bin", versionPath.toStdString().data());
    if(outFile.isOpen()) {
        outFile.write("r\n");
        outFile.write("h\n");
        outFile.write("erase 0x8000000,0x801ffff\n");
        QString sCmd;
        sCmd = "loadbin " + sBoot + " 0x8000000\n";
        outFile.write(sCmd.toStdString().data());
        sCmd = "loadbin " + sApp + " 0x8008000\n";
        outFile.write(sCmd.toStdString().data());
        sCmd = "verifybin " + sBoot + ",0x8000000\n";
        outFile.write(sCmd.toStdString().data());
        sCmd = "verifybin " + sApp + ",0x8008000\n";
        outFile.write(sCmd.toStdString().data());
        outFile.write("r\n");
        outFile.write("q\n");
        outFile.close();
    }
}

BurningMcuCB1000::BurningMcuCB1000()
{
    ADDCONFIG("McuCom", "COM4", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("JLinkPath", "E:", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("PCBAEXP", "", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("PythonPath", "", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否等待夹具", "1", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否烧录", "1","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否网络获取SN", "1","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("检测授权", "1","String",LOGIN_ADMIN,"", false);
    ADDCONFIG("授权程序路径", "1","FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("授权文件", "1","FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("本地版本路径", "E:", "FilePath",LOGIN_ADMIN,"", true);
    connect(pMachine(), &QStateMachine::stopped,
            this, &BurningMcuCB1000::slotsStoped);


    add(0, "start", std::bind(&BurningMcuCB1000::step0, this));
    add(5, "getSn", std::bind(&BurningMcuCB1000::getSn, this));
    add(10, "judgeCustomLifeTime", std::bind(&BurningMcuCB1000::jugdeCustomLifeTime, this, true));
    add(20, "enterMes", std::bind(&BurningMcuCB1000::enterMes, this));
    add(50, "step4", std::bind(&BurningMcuCB1000::getSnPCBMcu, this));
    add(75, "step6", std::bind(&BurningMcuCB1000::fileCheck, this));
    add(80, "step6", std::bind(&BurningMcuCB1000::checkJiaju, this));
    add(90, "step6", std::bind(&BurningMcuCB1000::burning, this));
    add(92, "step6", std::bind(&BurningMcuCB1000::waitver, this));
    add(93, "step6", std::bind(&BurningMcuCB1000::restart, this));
    add(94, "step6", std::bind(&BurningMcuCB1000::checkCan, this));
    add(101, "step7", std::bind(&BurningMcuCB1000::comMcuVersion, this));
    add(102, "step3", std::bind(&BurningMcuCB1000::writeSn, this));
    add(110, "step3", std::bind(&BurningMcuCB1000::comVol, this));

    add(150, "step2", std::bind(&BurningMcuCB1000::outMes, this));
    add(200, "step2", std::bind(&BurningMcuCB1000::stepEnd, this));

    mFixtureMcuCheck = new McuSerialManager();

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
        if(g_mesCheckPackage[i].sItem.size() <= 0) {
            break;
        }
        logTitles << g_mesCheckPackage[i].sItem;
    }

    QString filePathOut = GETCONFIG("LogPath") + "/data.csv";
    m_allCvsLog.setCsvLogPath(filePathOut);
    QFile file(filePathOut);
    if(!file.exists()) {
        m_allCvsLog.addCsvTitle(logTitles);
    }

    mFixtureMcu = new McuSerialManager();
}


void BurningMcuCB1000::getWorkInfo()
{
    showProcess("获取信息");
//    QString sSerialNameMcu = GETCONFIG("McuCom");

//    logNormal("Start open mcu serial...");
//    mFixtureMcu->closeSerial();
//    QThread::sleep(1);
//    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
//        m_errMsg = "开启MCU串口失败";
//        logFail(m_errMsg);
//        m_errorCode = -1;
//        return;
//    }

#if 1

//    if (!mFixtureMcu->setSysMode(MODE_USB_ADC)) {
//        logFail(m_errMsg);
//        m_errorCode = -1;
//        return;
//    }

    if (!isGoldenMode()) {
//        if (!mFixtureMcu->setSysMode(MODE_CAN_232)) {
//            m_errMsg = "CAN读取模式切换失败";
//            logFail(m_errMsg);
//            m_errorCode = -1;
//            return;
//        }


//        logNormal("切换至ADC模式");
//        if (!mFixtureMcu->setSysMode(MODE_SN_DATE)) {
//            m_errMsg = "ADC读取模式切换失败";
//            logFail(m_errMsg);
//            m_errorCode = -1;
//            return;
//        }
    }

#endif
    // -1. 关闭ASIC串口
    // mFixtureMcu->closeSerial();
}

void BurningMcuCB1000::getSnPCBMcu()
{
    MesCheckItem *pItem = findItem("NK1272A_PCBA");
    if(!pItem) {
        return;
    }
    CHECKSTATUS
    QElapsedTimer tStart;
    tStart.start();
    showProcess("开始扫码PCB");
    logNormal("开始扫码PCB");
    QString sn = msgBox("请扫码PCB");
    tableClearData();

    logNormal("扫码PCB " + sn);
    pItem->sExp = ConfigInfo::getInstance()->getValueString("PCBAEXP").toUpper();
    if(pItem->sExp.size() > 0 && !sn.contains(pItem->sExp.toUpper())) {
        logFail("扫码PCB failed");
        m_errorCode = -1;
        m_errMsg = "PCB扫码失败";
        pItem->sResult = MESFAILED;
        pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(*pItem);
        return;
    }
    mSnPCBPower = sn;
    pItem->sValue = mSnPCBPower;
    pItem->sResult = MESPASS;
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningMcuCB1000::comMcuVersion()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }
    MesCheckItem *pItem = findItem("mcu_version");
    if(!pItem) {
        return;
    }
    QElapsedTimer tStart;
    tStart.start();
    pItem->sResult = MESFAILED;
    showProcess("比对版本号");

    logNormal("Read version...");
    ArmWorkInfo infor;
    if (!mFixtureMcu->getArmWorkInfo(infor)) {
        m_errMsg = "获取ARM信息失败";
        logFail(m_errMsg);
        m_errorCode = -1;
        return;
    }
    m_armInfo = infor;
    logNormal("Read version: " + m_armInfo.mcuVersion);


    mCrtVersion = m_armInfo.mcuVersion;
    logNormal(mCrtVersion + " " +mSoftVersion );
    pItem->sValue = mCrtVersion;
    pItem->sResult = MESPASS;
    pItem->sExp = mSoftVersion;
    if(mCrtVersion != mSoftVersion) {
        m_errorCode = -1;
        m_errMsg = "版本号检测失败";
        logFail(m_errMsg);
        pItem->sResult = MESFAILED;
    }
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningMcuCB1000::fileCheck()
{
    CHECKSTATUS
    if (isGoldenMode()) {
        return;
    }
    MesCheckItem item;
    item.sItem = "版本文件核对";
    QString workDir = QCoreApplication::applicationDirPath() + "/mcu_burn";

    bool bRst = false;
    if(!isOnlineMode()) {
        m_versionPath = GETCONFIG("本地版本路径");
        mSoftVersion = m_versionPath.split("/").last();
        bRst = true;
    }else {
        mSoftVersion.clear();
        bRst = MesCom::instance()->getUpgradeFileVersion(mSoftVersion, "mcu");
        m_versionPath = workDir + "/" + mSoftVersion;
        QString versionFilePath = m_versionPath + "/update.zip";
        bRst &= MesCom::instance()->getVersionFile(versionFilePath, "mcu");
    }
    qInfo()<<"version path "<<m_versionPath;
    qInfo()<<"version "<<mSoftVersion;
    mSoftVersion = mSoftVersion.split("_").at(0);
    if (!bRst) {
        m_errorCode = -1;
        m_errMsg = "获取产品软件和版本失败";
        logFail("获取产品软件和版本失败");
        return;
    }

    {
        QString toolPath = QCoreApplication::applicationDirPath() + "/tools/zip/unzip.exe";
        QProcess processBat;
        processBat.start("cmd");
        QString scmd = QString::asprintf("cd /d \"%s\"\n", m_versionPath.toStdString().c_str());
        processBat.write(scmd.toStdString().data());
        processBat.write(QString(toolPath + " -o update.zip\n").toStdString().data());
        processBat.write("exit\n");
        processBat.waitForStarted();
        processBat.waitForFinished();
        qInfo("%s",QString::fromLocal8Bit(processBat.readAll()).toStdString().data());
    }
    {
        QFile fileV(m_versionPath+"/version.txt");
        fileV.open(QFile::ReadOnly);
        QString lineData;
        QStringList listLine;
        if(fileV.isOpen()) {
            while(!fileV.atEnd()) {
                lineData = QString(fileV.readLine());
                listLine = lineData.split(":");
                if(listLine.size() >= 2) {
                    if(listLine.at(0) == "MCU") {
                        mSoftVersion = listLine.at(1);
                        mSoftVersion = mSoftVersion.split("_").at(0);
                    }
                }
            }

            fileV.close();
        }
    }

    QElapsedTimer tStart;
    tStart.start();
    item.sResult = MESFAILED;
    showProcess("版本文件核对");
    QFile file(m_versionPath+"/filelist.txt");
    file.open(QFile::ReadOnly);
    QByteArray sData;
    if(file.isOpen()) {
        sData = file.readAll();
        file.close();
    }
    int iCheckOk = 1;
    if(sData.size() > 0) {
        QList<QByteArray> lItem = sData.split('\n');
        for(int i = 0; i < lItem.size(); i ++) {
            if(lItem.at(i).size() < 34) {
                continue;
            }
            qInfo()<<"---------------";
            QString sSrcMd5 = lItem.at(i).split('*').at(0);
            QString fileName = lItem.at(i).split('*').at(1);
            QString sDstMd5 = Util::md5sum(m_versionPath + "/" + fileName);

            qInfo()<<sSrcMd5<<" "<<sDstMd5;
            if(!sSrcMd5.toUpper().contains(sDstMd5)) {
                iCheckOk = 0;
                break;
            }
        }
    }

    item.sResult = iCheckOk ? MESPASS : MESFAILED;
    item.sValue = iCheckOk ? MESPASS : MESFAILED;
    item.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(item);
}

void BurningMcuCB1000::saveResult()
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
                << ((m_errorCode == 0) ? "PASS" : "FAIL");
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(g_mesCheckPackage[i].sItem.size() <= 0) {
            break;
        }
        logData << g_mesCheckPackage[i].sValue;
    }

    m_allCvsLog.addCsvLog(logData);
    QString logPath = GETCONFIG("LogPath");
    logPath += "/" + QDateTime::currentDateTime().toString("yyyy_MM_dd");
    logPath += "/" + m_sn + ".csv";
    tableSaveCsv(logPath);
}

void BurningMcuCB1000::burning()
{
    CHECKSTATUS
    if (isGoldenMode()) {
        return;
    }
    if(!GETCONFIG("是否烧录").toInt()) {
        return;
    }

    MesCheckItem item;
    item.sItem = "版本文件烧写";
    QElapsedTimer tStart;
    tStart.start();

    QString loadPath = QCoreApplication::applicationDirPath() + "/mcu_burn";
    setLoad(loadPath,m_versionPath);

    showProcess("烧写中...");
    bool bVerifyOk = false;
    QString sOut;
    {
        logNormal("开始烧写load.txt");
        QProcess process;
        process.start("cmd");
        process.waitForStarted();

        QString sCmd = ConfigInfo::getInstance()->getValueString("JLinkPath")
                + "/JLink.exe -device AC78013MDQA -if swd -speed 4000 \""
                + loadPath + "/load.txt\"\n";
        process.write(sCmd.toStdString().data());

        while (true) {
            QCoreApplication::processEvents();

            sOut = QString::fromLocal8Bit(process.readAllStandardOutput());
            if (!sOut.isEmpty())
                logNormal("过程LOG:" + sOut);
            if(sOut.contains("processing completed.")) {
                logNormal(sOut);
                logPass("Has found key words: processing completed.");
                bVerifyOk = true;
                QThread::sleep(3);
                break;
            }
            QThread::sleep(1);
        }
        process.write("exit\n");
        process.waitForFinished(6000);
        sOut = QString::fromLocal8Bit(process.readAllStandardOutput());
        logNormal(sOut);
    }

    qInfo("out %s", sOut.toStdString().data());

    if (bVerifyOk) {
        logNormal("烧写成功");
        item.sResult = MESPASS;
    }else {
        item.sResult = MESFAILED;
        logFail("烧写失败");
        m_errorCode = -1;
        m_errMsg = "烧写失败";
    }
    item.sValue="/";
    item.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(item);
}

void BurningMcuCB1000::comVol()
{
    CHECKSTATUS
    QElapsedTimer tStart;
    tStart.start();

    showProcess("比对电压");

    McuWorkInfo mcuInfor;
    if (!mFixtureMcu->getMcuWorkInfo(mcuInfor)) {
        m_errMsg = "Get Mcu Voltage Infor Failed";
        logFail(m_errMsg);
        m_errorCode = -1;
        return;
    }else {
        m_mcuInfo = mcuInfor;
    }


    MesCheckItem * pItem = g_mesCheckPackage;
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        QString decItem = pItem[i].sItem;
        if(decItem.size() <= 0 || pItem[i].sMin.size() <= 0) {
            continue;
        }
        double value = getVoltage(pItem[i].sExp.toLocal8Bit().data(), m_mcuInfo);
        int iOk = 0;
        pItem[i].sValue = QString::asprintf("%.2f", value);
        if(value + 1e-5 < pItem[i].sMin.toDouble() ||
                value - 1e-5 > pItem[i].sMax.toDouble() ){
            QString log = QString("Item: %1 \tVoltage: %3 [%4, %5] FAILED").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(pItem[i].sMin)
                    .arg(pItem[i].sMax);
            logFail(log);
            pItem[i].sResult = MESFAILED;
            m_errorCode = -3;
            m_errMsg = "电压比对失败";
            iOk = 1;
        }else {
            QString log = QString("Item: %1 \tVoltage: %3 [%4, %5] PASS").arg(decItem)
                    .arg(QString::number(value, 'f', 2))
                    .arg(pItem[i].sMin)
                    .arg(pItem[i].sMax);
            logNormal(log);
            pItem[i].sResult = MESPASS;
        }
        pItem[i].tCheckTm = QString::number(tStart.elapsed()/1000);;
        showItem(pItem[i]);
    }

}

void BurningMcuCB1000::stepEnd()
{
    if (!mFixtureMcu->setSysMode(MODE_USB_ADC)) {
    }
    if (!mFixtureMcu->controlArmPower(0)) {
    }
    mFixtureMcu->closeSerial();

    saveResult();
    if(m_errorCode == 0) {
        showPass();
    }else {
        showFail("NG " + m_errMsg);
    }
    msgBox("请更换产品，执行下一轮测试", 4);
    stopProcess();
}

void BurningMcuCB1000::step0()
{
    m_workingTm = QDateTime::currentMSecsSinceEpoch();
    m_errorCode = 0;
	m_errMsg.clear();
    mSnPCBPower.clear();
    mCrtVersion.clear();

}

void BurningMcuCB1000::start()
{
    qInfo("start ");
    m_workingTm = QDateTime::currentMSecsSinceEpoch();
    mSnPCBPower.clear();
    mCrtVersion.clear();
    showDefault();
    showProcess("开始检测");
    while(pMachine()->isRunning()){
        pMachine()->stop();
        qInfo("wait to stop");
    }
    startProcess();
}

void BurningMcuCB1000::waitver()
{
    CHECKSTATUS
    if (isGoldenMode()) {
        return;
    }

    if(!GETCONFIG("检测授权").toInt()) {
        return;
    }

    QThread::msleep(2000);

    int nRet = checkAutority();
    if (nRet == 1) {
        logNormal("已经授权过, 不再执行授权动作");
        return;
    } else if (nRet == -1) {
        logFail(m_errMsg);
        m_errorCode = -3;
        return;
    } else { // -2
        logNormal("产品还未授权过, 将执行授权动作");
    }

    if (!mFixtureMcu->setSysMode(MODE_USB_ADC)) {
        m_errMsg = "切换USB_ADC模式失败";
        m_errorCode = -3;
        logNormal(m_errMsg);
        return;
    }

    QElapsedTimer tStart;
    tStart.start();
    showProcess("开始授权");
    QString workDir = QCoreApplication::applicationDirPath() + "/mcu_burn";
    QProcess process;
    process.start("cmd");
    process.waitForStarted();
    QString PythonPath = GETCONFIG("PythonPath");
    QString scmd = QString::asprintf("cd /d %s\n", workDir.toStdString().data());
    process.write(scmd.toStdString().data());
    scmd = QString::asprintf("%s/python.exe test.py\n",
                             PythonPath.toStdString().data());
    process.write(scmd.toStdString().data());
    process.write("exit\n");
    process.waitForFinished(60000);
    process.terminate();
    process.waitForFinished();
    QString sOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    qInfo("out %s", sOut.toStdString().data());
    int ret=process.exitCode();
    MesCheckItem item;
    if(ret == 0) {
        logNormal("授权成功");
        item.sResult = MESPASS;
    }else {
        item.sResult = MESFAILED;
        m_errorCode = -3;
        m_errMsg = "授权失败";
        logFail("授权失败");
    }
    item.sItem = "授权";
    item.sValue="/";
    item.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(item);
}

void BurningMcuCB1000::restart()
{
    if (!mFixtureMcu->controlArmPower(0)) {
        m_errMsg = "给MCU下电电失败";
        logFail(m_errMsg);
        m_errorCode = -1;
        return;
    }
    QThread::msleep(1500);
    if (!mFixtureMcu->controlArmPower(12)) {
        m_errMsg = "给MCU上电电失败";
        logFail(m_errMsg);
        m_errorCode = -1;
        return;
    }
    int iCount = 12;
    while(iCount-- > 0) {
        showProcess("等待重启...." + QString::number(iCount));
        QThread::sleep(1);
    }
}

void BurningMcuCB1000::slotsStoped()
{
    if(m_workStatus) {
        startProcess();
    }
}

void BurningMcuCB1000::slotStartWorkBtnClicked()
{
    tClear();
    m_errMsg.clear();
    mVoltageList.clear();
    logClear();
    QDateTime dateTime = QDateTime::currentDateTime();
    mTestStartTimeStr = dateTime.toString("yyyy/MM/dd hh:mm:ss:zzz");
    m_workStatus = true;
    start();
}

void BurningMcuCB1000::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_workStatus = false;
    m_signal.signal();
    hideMsgBox();
    showProcess("请先点检环境是否正常");
}

void BurningMcuCB1000::slotConnectBtnClicked()
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

    tHeader(headTitles);
    if(!mFixtureMcuCheck)
        mFixtureMcuCheck = new McuSerialManager();
    mFixtureMcuCheck->closeSerial();
    if(!mFixtureMcuCheck->openSerial(ConfigInfo::getInstance()->getValueString("McuCom"))) {
        emit showMsgBox("MCU打开失败，请确认配置", 3);
        if("调试模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {
            return;
        }
    }
    mFixtureMcuCheck->closeSerial();

    BaseProduce::slotConnectBtnClicked();
}

void BurningMcuCB1000::getSn()
{
    CHECKSTATUS
    //只有生产模式存在
    if("生产模式" != GETCONFIG("调试模式")) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        m_sn = msgBox("请扫码sn");
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
        return;
    }
    showProcess("获取 SN");
    QString sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    if(GETCONFIG("是否网络获取SN").toInt()) {
        if(0 != MesCom::instance()->getOrderSns(info.orderId, sOut)) {
            logFail("获取SN失败" + sOut);
            m_errorCode = -101;
            m_errMsg = "获取SN失败 " + sOut;
            return;
        }else {
            m_sn = sOut;
        }
    }else {
        m_sn = msgBox("请扫码下一个sn");
    }
    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);
    logClear();
}

void BurningMcuCB1000::outMes()
{
    CHECKSTATUS
    //只有生产模式存在
    if(isOnlineMode() && !m_skipMesUpload) {
        QString sMes;
        MesCom::instance()->setItemResult(g_mesCheckPackage);
        if(!MesCom::instance()->outProduce((m_errorCode == 0) ? 0 : 1, sMes)) {
            logFail("出站失败 " + sMes);
            m_errorCode = -15;
            m_errMsg = "出站失败 " + sMes;
        }else {
            logNormal("出站成功");
        }
    }else if("点检模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {//点检模式上传
        if(m_checkItem) {

        }
    }
}

void BurningMcuCB1000::enterMes()
{
    CHECKSTATUS
    //只有生产模式存在
    if("生产模式" != ConfigInfo::getInstance()->getValueString("调试模式")) {
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
        m_errMsg = "进站失败 " + sMes;
        m_skipMesUpload = true;
    }else {
        tem.sResult=MESPASS;
    }
    showItem(tem);
}

void BurningMcuCB1000::checkJiaju()
{
    CHECKSTATUS
    QString sWait = GETCONFIG("是否等待夹具");
    if(0 == sWait.toInt()) {
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
        m_errorCode = -2;
        m_errMsg = "开启MCU串口失败";
        tem.sResult = MESFAILED;
        tem.tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(tem);
        return;
    }
//    if (!mFixtureMcu->setSysMode(MODE_USB_ADC)) {
//        logNormal(m_errMsg);
//    }
    showProcess("请开始下压夹具");

    logNormal("Start detect the fixture pressure");
    logNormal("自动侦测夹具下压，请开始下压夹具...");

    mFixtureMcu->controlArmPower(0);
    while (!mFixtureMcu->pressed() ) {
        if(!m_workStatus) {
            m_errorCode = -2;
            m_errMsg = "停止测试";
            return;
        }
    }
    logNormal(QString("当前工装MCU版本位: %1").arg(mFixtureMcu->version(), 2, QLatin1Char('0')));

    logNormal("产品上电");
    if (!mFixtureMcu->controlArmPower(12)) {
        logFail("上电失败");
        m_errMsg = "上电失败";
        m_errorCode = -2;
        return;
    }
    QThread::msleep(2000);

    float voltage= mFixtureMcu->getArmVoltage();
    qDebug() << "current voltage " << voltage;

    tem.sExp = "/";
    tem.sValue = "/";
    tem.sResult = MESPASS;
    tem.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(tem);
}

void BurningMcuCB1000::writeSn()
{
    CHECKSTATUS
    if(isGoldenMode()) {
        return;
    }
    MesCheckItem tem;
    tem.sItem = "SN烧写";
    tem.sResult = MESFAILED;
    QElapsedTimer tStart;
    tStart.start();
    showProcess("SN烧写");

    logNormal("Write SN " + m_sn);
    m_readSn.clear();
    if (!mFixtureMcu->writeSn(m_sn)) {
        logFail("写入sn失败");
        m_errorCode = -1;
        m_errMsg = "写入SN失败";
        return;
    }else {
        QString sn;
        if (!mFixtureMcu->readSn(sn)) {
            logFail("读取SN失败!!!");
            m_errorCode = -2;
            m_errMsg = "读取SN失败";
            return;
        }
        m_readSn = sn;
    }
    logNormal("read " + m_readSn);

    if(m_sn != m_readSn) {
        logFail("SN比对失败，读取的SN与写入SN不一致 " + m_readSn);
        m_errMsg = "SN比对失败，读取的SN与写入SN不一致 " + m_readSn;
        m_errorCode = -2;
        return;
    }
    tem.sExp = m_sn;
    tem.sValue = m_readSn;
    tem.sResult = (m_errorCode == 0) ? MESPASS : MESFAILED;
    tem.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(tem);
}

void BurningMcuCB1000::checkCan()
{
    CHECKSTATUS

    MesCheckItem tem;
    tem.sItem = "检测授权";
    tem.sResult = MESFAILED;
    QElapsedTimer tStart;
    tStart.start();
    showProcess("检测授权");
    // getWorkInfo();

    if (isGoldenMode()) {
        return;
    }

    if(!GETCONFIG("检测授权").toInt()) {
        return;
    }

    ArmWorkInfo infor;
    int checkCount = 15;
    while (checkCount-- > 0) {
        m_errorCode = 0;
        logNormal("Read can info...");
        mFixtureMcu->getArmWorkInfo(infor);
        if (infor.canWorkEnable != 0x01 || infor.canWorkFlag != 0x01) {
            m_errorCode = -1;
            QThread::msleep(200);
            continue;
        }

        showProcess("授权信息比对");
        if (infor.mcuAuthorCompleted != 1 || infor.mcuAuthorValid != 1) {
            m_errorCode += -2;
            QThread::msleep(200);
            continue;
        }
        m_errorCode = 0;
        break;
    }

    switch (m_errorCode) {
    case 0:
        m_errMsg.clear();
        break;
    case -1:
        m_errMsg = "CAN通讯标志无效，软件版本获取失败";
        logFail(m_errMsg);
        break;
    case -2:
        m_errMsg = QString("授权信息比对失败, 授权有效：%1, 授权完成: %2").arg(infor.mcuAuthorValid).arg(infor.mcuAuthorCompleted);
        logFail(m_errMsg);
        break;
    default:
        m_errMsg = "CAN通讯与授权检测失败";
        break;
    }

    tem.sExp = "/";
    tem.sValue = "/";
    tem.sResult = (m_errorCode == 0) ? MESPASS : MESFAILED;
    tem.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(tem);
}

int BurningMcuCB1000::checkAutority()
{
    logNormal("读取授权信息...");
    ArmWorkInfo infor;

    float voltage = mFixtureMcu->getArmVoltage();
    float current = mFixtureMcu->getArmCurrent();

    logNormal(QString("Voltage: %1, Current: %2").arg(voltage).arg(current));

    bool checkResult = false;
    int checkCount = 30;
    while(checkCount-- > 0) {
        mFixtureMcu->getArmWorkInfo(infor);
        if(infor.canWorkFlag == 1) {
            if(infor.canWorkEnable) {
                checkResult = true;
                break;
            } else {
                checkResult = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    if (!checkResult) {
        m_errMsg = "CAN工作异常, canWorkEnable: " + QString::number(infor.canWorkEnable) +
                ", canWorkFlag: " + QString::number(infor.canWorkFlag);
        return -1; // 异常
    }

    if (infor.mcuAuthorCompleted == 1  && infor.mcuAuthorValid == 1) {
        return 1; // 授权
    }

    return -2; // 未授权
}

void BurningMcuCB1000::showItem(MesCheckItem item)
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

MesCheckItem *BurningMcuCB1000::findItem(QString sItem)
{
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(g_mesCheckPackage[i].sItem == sItem) {
            return &g_mesCheckPackage[i];
        }
    }
    return nullptr;
}

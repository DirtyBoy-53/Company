#include "burningarmce1001.h"
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
#include <qjsonobject.h>
#include <qjsondocument.h>

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
    {"arm_version", "V1.1.1", "","",""},
    {"arm_pcba_sn", "", "","",""},
    {""},
};

static int setConfig(QString sImgPath, QString srcConfigPath)
{
    QFile file(srcConfigPath+ "/user_config.cache");
    file.open(QFile::ReadOnly);
    QByteArray data;
    if(file.isOpen()) {
        data = file.readAll();
        file.close();
    }
    if(data.size()<=0) {
        qInfo("read config failed %s", srcConfigPath.toStdString().data());
        return -1;
    }
    QString sMsg(data);
    qInfo()<<sMsg;
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(data, &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        qInfo("parse config failed %s %s",
              srcConfigPath.toStdString().data(),
              jsonError.errorString().toStdString().data());
        return -1;
    }
    QJsonObject object = document.object();
    object.insert("com_0",GETCONFIG("烧录串口").toUpper());
    object.insert("upfiles_0", sImgPath);
    QString sPath = GETCONFIG("烧录工具路径");
    QFile fOut(sPath + "/user_config.cache");
    fOut.open(QFile::WriteOnly|QFile::Truncate);
    if(fOut.isOpen()) {
        fOut.write(QJsonDocument(object).toJson(QJsonDocument::Compact));
        fOut.close();
    }else {
        qInfo("write config failed %s", sPath.toStdString().data());
        return -1;
    }
    return 0;
}

BurningArmCE1001::BurningArmCE1001()
{
    ADDCONFIG("McuCom", "COM4", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否等待夹具", "1", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("烧录串口", "COM3", "String",LOGIN_ADMIN,"", false);
    ADDCONFIG("烧录工具路径", "D:\tool\hbupdate", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("PythonPath", "D:\tool\hbupdate", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("本地版本路径", "D:\tool\hbupdate", "FilePath",LOGIN_ADMIN,"", false);
    ADDCONFIG("是否烧录", "1","String",LOGIN_ADMIN,"", true);

    connect(pMachine(), &QStateMachine::stopped,
            this, &BurningArmCE1001::slotsStoped);

    add(0, "start", std::bind(&BurningArmCE1001::step0, this));
    add(5, "getSn", std::bind(&BurningArmCE1001::getSn, this));
    add(10, "judgeCustomLifeTime", std::bind(&BurningArmCE1001::jugdeCustomLifeTime, this, true));
    add(20, "enterMes", std::bind(&BurningArmCE1001::enterMes, this));
    add(75, "step6", std::bind(&BurningArmCE1001::fileCheck, this));
    add(80, "step6", std::bind(&BurningArmCE1001::checkJiaju, this));
    add(90, "step6", std::bind(&BurningArmCE1001::burning, this));
    add(91, "step6", std::bind(&BurningArmCE1001::checkSum, this));
    add(100, "step6", std::bind(&BurningArmCE1001::restart, this));
    add(110, "step7", std::bind(&BurningArmCE1001::comArmVersion, this));
    add(150, "step2", std::bind(&BurningArmCE1001::outMes, this));
    add(200, "stepEnd", std::bind(&BurningArmCE1001::stepEnd, this));

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

    m_armDebugSerial = nullptr;
}


void BurningArmCE1001::getWorkInfo()
{
    showProcess("获取设备信息");

//    QString sSerialNameMcu = GETCONFIG("McuCom");

//    logNormal("Start open mcu serial...");
//    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
//        logFail("开启MCU串口失败");
//        m_errMsg = "开启MCU串口失败";
//    }

//    if (!mFixtureMcu->controlArmPower(0)) {
//        m_errMsg = "给ASIC下电电失败";
//        logFail(m_errMsg);
//    }
//    QThread::usleep(100);
//    if (!mFixtureMcu->controlArmPower(12)) {
//        m_errMsg = "给ASIC上电失败";
//        logFail(m_errMsg);
//    }
//    int iCount = 15;
//    while(iCount > 0) {
//        showProcess("等待重启...." + QString::number(iCount));
//        QThread::sleep(1);
//        iCount--;
//    }
//    showProcess("等起启动");
//    if (!mFixtureMcu->setSysMode(MODE_USB_ADC)) {
//        m_errMsg = "CAN读取模式切换失败";
//        logFail(m_errMsg);
//    }
//    if (!mFixtureMcu->setSysMode(MODE_CAN_232)) {
//        m_errMsg = "CAN读取模式切换失败";
//        logFail(m_errMsg);
//    }
//    logNormal("Read asic version...");
//    ArmWorkInfo infor;
//    if (!mFixtureMcu->getArmWorkInfo(infor)) {
//        m_errMsg = "获取ARM信息失败";
//        logFail(m_errMsg);
//    }
//    mCrtVersion = infor.armVersion;

//    // 0. 控制ASIC下电
//    logNormal("Start control aisc pcb power off...");
//    if (!mFixtureMcu->controlSensorPower(0)) {
//        logFail("下电失败");
//    }
//    // -1. 关闭ASIC串口
//    // mFixtureMcu->closeSerial();
    showProcess("获取信息完成");
}


void BurningArmCE1001::restart()
{
    CHECKSTATUS

    QString sSerialNameMcu = GETCONFIG("McuCom");
    logNormal("烧录后再次打开MCU串口控制重启...");
    mFixtureMcu->closeSerial();
    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
        logFail("开启MCU串口失败");
        m_errorCode = -5;
        m_errMsg = "开启MCU串口失败";
        return;
    }

    QThread::msleep(1500);
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

void BurningArmCE1001::comArmVersion()
{
    CHECKSTATUS

    showProcess("比对版本号");
    // getWorkInfo();
    logNormal("Read asic version...");
    ArmWorkInfo infor;
    mFixtureMcu->setSysMode(MODE_CAN_232);
    QThread::msleep(2000);//延时很很总要，否则不会返回版本数据
    if (!mFixtureMcu->getArmWorkInfoB(infor)) {
        m_errorCode = -10;
        m_errMsg = "获取ARM信息失败";
        logFail(m_errMsg);
        return;
    }
    mCrtVersion = infor.armVersion;


    MesCheckItem *pItem = findItem("arm_version");
    if(!pItem) {
        return;
    }
    QElapsedTimer tStart;
    tStart.start();
    pItem->sResult = MESFAILED;
    showProcess("比对版本号");
    logNormal(mCrtVersion + " " +mSoftVersion );
    pItem->sValue = mCrtVersion;
    pItem->sResult = MESPASS;
    pItem->sExp = mSoftVersion;
    if(mCrtVersion.toUpper().trimmed().compare(mSoftVersion.toUpper().trimmed()) != 0) {
        logFail("版本号检测失败");
        pItem->sResult = MESFAILED;
        m_errorCode = -10;
        m_errMsg = "版本号检测失败";
        return;
    }
    logPass("版本号比对OK");
    pItem->tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(*pItem);
}

void BurningArmCE1001::saveResult()
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

void BurningArmCE1001::burning()
{
    CHECKSTATUS

    if(!ConfigInfo::getInstance()->getValueString("是否烧录").toInt()) {
        return;
    }
    showProcess("烧写中...");
    QElapsedTimer tStart;
    tStart.start();
    MesCheckItem item;
    item.sItem = "版本文件烧写";
    QProcess process;
    if(0 != setConfig(mImgPath, mWorkPath )) {
        logFail("写配置失败");
        m_errorCode= -2;
        m_errMsg = "写配置失败";
        item.sResult = MESFAILED;
        item.tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(item);
        return;
    }
    QString exePath = GETCONFIG("烧录工具路径");
    QString PythonPath = GETCONFIG("PythonPath");

    process.start("cmd");
    process.waitForStarted();
    QString sCmd = "cd /d "+mWorkPath+"\n";
    process.write(sCmd.toStdString().data());
    QString sCmdP = PythonPath + "/python armburn.py " + exePath +"\n";
    process.write(sCmdP.toStdString().data());
    qInfo("start to exit");
    process.write("exit\n");

    qInfo("start to wait.....");
    int iCount = 3000;
    while(iCount > 0) {
        iCount -- ;

        if(process.waitForReadyRead(100)) {
            QString sOut = QString::fromLocal8Bit(process.readAll());
            qInfo("recv %s", sOut.toStdString().data());

        }
        if(process.waitForFinished(10)) {
            qInfo("finish....");
            break;
        }
    }
    process.waitForFinished(2000000);
    int iRes = process.exitCode();

    process.waitForFinished();

    if(!iRes) {
        logNormal("烧写成功");
        item.sResult = MESPASS;
    }else {
        item.sResult = MESFAILED;
        logFail("烧写失败 " + QString::number(iRes));
        m_errorCode = -10;
        m_errMsg = "烧写失败 " + QString::number(iRes);
    }
    item.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(item);
}

void BurningArmCE1001::checkSum()
{
    CHECKSTATUS
    if(!ConfigInfo::getInstance()->getValueString("是否烧录").toInt()) {
        return;
    }
    showProcess("执行CheckSum...");
    QElapsedTimer tStart;
    tStart.start();
    if (openArmSerial()) {
        if (armCheckSum()) {
            logPass("Check Sum OK");
        } else {
            logFail("Check Sum Fail");
            m_errorCode = -10;
            m_errMsg = "Check Sum Fail";
        }

        closeArmSerial();
    }
}

void BurningArmCE1001::fileCheck()
{
    CHECKSTATUS
    MesCheckItem item;
    item.sItem = "版本文件核对";
    mWorkPath = QCoreApplication::applicationDirPath() + "/armtool";

    bool bRst = false;
    if(!isOnlineMode()) {
        if(GETCONFIG("本地版本路径").size() > 0) {
            m_versionPath = GETCONFIG("本地版本路径");
            bRst = true;
        }else {
            logFail("请设置本地版本路径");
            m_errorCode = -10;
            return;
        }
    }else {
        mSoftVersion.clear();
        bRst = MesCom::instance()->getUpgradeFileVersion(mSoftVersion, "arm");
        m_versionPath = mWorkPath + "/" + mSoftVersion;
        QString versionFilePath = m_versionPath + "/update.zip";
        if (!QFile::exists(versionFilePath)) {
            bRst &= MesCom::instance()->getVersionFile(versionFilePath, "arm");
        }
    }
    qDebug() << m_versionPath;
    qDebug() << mSoftVersion;

    if (!bRst) {
        logFail("获取升级版本文件失败");
        m_errorCode = -10;
        m_errMsg = "获取升级版本文件失败";
        return;
    }

    mImgPath = m_versionPath + "/disk.img";
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
                    qInfo()<<"list "<<listLine.at(0)<<" "<<listLine.at(1);
                    if(listLine.at(0) == "ARM") {
                        mSoftVersion = listLine.at(1);
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
    bool isCheckOk = true;
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

            QString log = QString("比对文件：%1, 原始MD5: %2, 计算MD5: %3").arg(fileName).arg(sSrcMd5).arg(sDstMd5);
            logNormal(log);
            qInfo() << sSrcMd5 << " " << sDstMd5;
            if(!sSrcMd5.toUpper().contains(sDstMd5)) {
                isCheckOk = false;
                m_errorCode = -10;
                m_errMsg = "MD5比对失败";
                logFail("比对MD5失败!!!");
                break;
            }
        }
    }

    item.sResult = isCheckOk ? MESPASS : MESFAILED;
    item.sValue = isCheckOk ? MESPASS : MESFAILED;
    item.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(item);
}


void BurningArmCE1001::stepEnd()
{
    // 0. 控制ASIC下电
    mFixtureMcu->controlSensorPower(0);
    // -1. 关闭ASIC串口
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

void BurningArmCE1001::step0()
{
    if(!mFixtureMcu)
        mFixtureMcu = new McuSerialManager();
    m_workingTm = QDateTime::currentMSecsSinceEpoch();

    m_errorCode = 0;
	m_errMsg.clear();

    mSnPCBPower.clear();
    mCrtVersion.clear();

}

void BurningArmCE1001::start()
{
    qInfo("start ");
    m_workingTm = QDateTime::currentMSecsSinceEpoch();
    mSnPCBPower.clear();
    mCrtVersion.clear();
    showDefault();
    showProcess("开始检测");
    startProcess();
}

void BurningArmCE1001::slotsStoped()
{
    if(m_workStatus) {
        startProcess();
    }
}

void BurningArmCE1001::slotStartWorkBtnClicked()
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

void BurningArmCE1001::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_workStatus = false;
    m_signal.signal();
    hideMsgBox();
    showProcess("请先点检环境是否正常");
}

void BurningArmCE1001::slotConnectBtnClicked()
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
        showFail("MCU打开失败，请确认配置");
        return;
    }
    mFixtureMcuCheck->closeSerial();


    if (!getArmSerialHdlc()) {
        showFail("获取串口资源句柄失败");
        return;
    }

    BaseProduce::slotConnectBtnClicked();
}

void BurningArmCE1001::getSn()
{
    CHECKSTATUS
    showProcess("获取 SN");
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    m_sn = msgBox("请扫码sn");
    info.sSn = m_sn;
    ConfigInfo::getInstance()->setCacheInfo(info);
    tableClearData();
    logClear();
}

void BurningArmCE1001::outMes()
{
     // CHECKSTATUS
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

void BurningArmCE1001::enterMes()
{
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
        m_errorCode = -16;
        m_errMsg = "进站失败" + sMes;
        logFail(m_errMsg);
        m_skipMesUpload = true;
    }else {
        tem.sResult = MESPASS;
    }
    showItem(tem);
}

void BurningArmCE1001::checkJiaju()
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
    mFixtureMcu->closeSerial();
    logNormal("Start open mcu serial..." + sSerialNameMcu);
    if (!mFixtureMcu->openSerial(sSerialNameMcu)) {
        logFail("开启MCU串口失败");
        m_errorCode= -2;
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
    int iDetectOk = 1;

    mFixtureMcu->controlArmPower(0);
    while (!mFixtureMcu->pressed() ) {
        if(!m_workStatus) {
            iDetectOk = 0;
            break;
        }
    }
    logNormal(QString("当前工装MCU版本位: %1").arg(mFixtureMcu->version(), 2, QLatin1Char('0')));

    // 不需要进行上电的操作，python脚本会做
    // mFixtureMcu->controlArmPower(12);
    // 串口必须关掉，因为python脚本会去打开串口控制上下电，在后面restart的地方再开启串口
    mFixtureMcu->closeSerial();

    if(!iDetectOk) {
        logFail("夹具检测失败");
        m_errorCode= -2;
        m_errMsg = "夹具检测失败";
        tem.sResult = MESFAILED;
        tem.tCheckTm = QString::number(tStart.elapsed()/1000);
        showItem(tem);
        return;
    }
    tem.sResult = MESPASS;
    tem.tCheckTm = QString::number(tStart.elapsed()/1000);
    showItem(tem);

}

void BurningArmCE1001::showItem(MesCheckItem item)
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

MesCheckItem *BurningArmCE1001::findItem(QString sItem)
{
    for(int i = 0; i < MESMAXCHECKNUM; i ++) {
        if(g_mesCheckPackage[i].sItem == sItem) {
            return &g_mesCheckPackage[i];
        }
    }
    return nullptr;
}

bool BurningArmCE1001::getArmSerialHdlc()
{
    if (m_armDebugSerial != nullptr) {
        m_armDebugSerial->linkRelease();
        m_armDebugSerial = nullptr;
    }

    QString crtPath = QCoreApplication::applicationDirPath() + "/";
    crtPath.append(DATALINKDLLNAME);

    QFile file(crtPath);
    if(!file.exists()) {
        qDebug() << "dll file is not exist in path: " << crtPath;
        return false;
    }

    QLibrary lib;
    lib.setFileName(crtPath);
    if (!lib.load()) {
        qDebug() << "dll file is not exist in path: " << lib.errorString();
        return false;
    }

    typedef bool (*OBJ)(char*, void**);
    OBJ getObj = (OBJ)lib.resolve("GetInterface");
    if (!getObj) {
        qDebug("get interface 'GetInterface' failed!!!");
        lib.unload();
        return false;
    }

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&m_armDebugSerial))) {
        qDebug("get read interface pointer failed!!!");
        lib.unload();
        return false;
    }

    if (m_armDebugSerial) {
        return true;
    }

    lib.unload();
    return false;
}

bool BurningArmCE1001::openArmSerial()
{
    if (!m_armDebugSerial) return false;

    QString port = ConfigInfo::getInstance()->getValueString("烧录串口");
    if (!m_armDebugSerial->linkSerialInit(port.toLocal8Bit().data(), 921600)) {
        qDebug() << "调试串口打开失败";
        return false;
    }

    m_armDebugSerial->linkSetFormat(FormatString);

    return true;
}

bool BurningArmCE1001::armCheckSum()
{
    // login:
    bool bRst = false;
    while (true) {
        QCoreApplication::processEvents();
        m_armDebugSerial->linkWrite("\r", 1);
        if (m_armDebugSerial->linkFind("login:", 6, 3000)) {
            m_armDebugSerial->linkWrite("root\r", 5);
            if (m_armDebugSerial->linkFind("hynix2G-3200:~#", strlen("hynix2G-3200:~#"))) {
                qDebug() << "login successfully!!!";
                bRst = true;
                break;
            }
        }

        QThread::msleep(100);
    }

    if (bRst) {
        char checkCmds[] = "/app/ddtest.sh\r";
        m_armDebugSerial->linkWrite(checkCmds, strlen(checkCmds));
        QThread::msleep(15000);
        if (m_armDebugSerial->linkFind("partitions md5 check success", strlen("partitions md5 check success"))) {
            qDebug() << "check sum ok";
            return true;
        } else {
            qDebug() << "check sum failed!!!";
        }
    }

    return false;
}

void BurningArmCE1001::closeArmSerial()
{
    if (!m_armDebugSerial) return;
    m_armDebugSerial->linkDeInit();
}



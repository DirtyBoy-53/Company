#include "autoprocessguide.h"
#include "StaticSignals.h"
#include <QTime>
#include "util.h"
#include "videoinfo.h"
#include "configinfo.h"
#include <QSettings>
#include <QTextCodec>
#include "CMyPrinter.h"
#include "imgCalY8.h"
#include "asiccontrol.h"
#include "yuv2rgb.h"
#include "mapping.h"
#include "mappingnew.h"
#include "Frock_cal.h"
#include <csvlogger.h>

#define CURCODE ConfigInfo::getInstance()->getValueString("当前组件编码")
#define ADDCONFIG(name,value,type,pro,enumNames,bSave) \
{ \
    XmlConfigInfo info={CURCODE,name,value,type,pro,enumNames}; \
    ConfigInfo::getInstance()->addConfig(info, bSave); \
}

#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)


AutoProcessComGuide::AutoProcessComGuide(const int type)
{
    ADDCONFIG("弹框格式", "选择框", "Enum",LOGIN_ADMIN,"确认框,选择框", true);
    m_type = type;
    initStateMachine();
}

void AutoProcessComGuide::initTipsInfo()
{
    ComGuideCfg::getGuideCfgList(m_tipsList);
}

void AutoProcessComGuide::slotRecvVideoFrameWs(QByteArray frameData, FrameInfo_S frameInfo)
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

    if (m_videoQueue.size() < 200) {
        m_videoQueue.push_back(frameData.mid(info.nvsLen, info.frameLen));
    }

    emit videoUpdate(0, frameData, frameInfo.uTimeTm);
    m_frameCount++;
}

void AutoProcessComGuide::slotConnectBtnClicked()
{
    emit StaticSignals::getInstance()->statusChangeVideoPage(m_type);
    if(m_type == 1) {
        m_ipAddress = ConfigInfo::getInstance()->getValueString("盒子IP");
        if(Util::testPing(m_ipAddress)) {
            if(m_webSocket == nullptr) {
                m_webSocket = new WebSocketClientManager();
                m_webSocket->connectToUrl(m_ipAddress, 6000);
                connect(m_webSocket,&WebSocketClientManager::signalRecvVideoFrame,this,&AutoProcessComGuide::slotRecvVideoFrameWs);
            }
        } else {
            addLog("网络异常, 请检查工装盒ip及网络连接", -1);
            return;
        }
    }
    if(CURCODE == "GP1000") {
        StaticSignals::getInstance()->statusChangeVideoPage(1);
    }
    emit tableAddHeader(QStringList() << "检测项" << "工作时间(ms)" << "结果");

    BaseProduce::slotConnectBtnClicked();
}

void AutoProcessComGuide::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &AutoProcessComGuide::slotsStoped);

    add(0, "check next", std::bind(&AutoProcessComGuide::checkNext, this));


    if ("DE1012" == ConfigInfo::getInstance()->cacheInfo().sCodeId) {
        add(10, "get sn", std::bind(&AutoProcessComGuide::getInnerSn, this));
    } else {
        add(10, "get sn", std::bind(&AutoProcessComGuide::getSn, this));
    }
    add(12, "judgeCustomLifeTime", std::bind(&AutoProcessComGuide::jugdeCustomLifeTime, this, true));
    add(15, "enter mes", std::bind(&AutoProcessComGuide::enterMes, this));

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "DE1002") {
        add(20, "getLenSn", std::bind(&AutoProcessComGuide::getLenSn, this));
    }
    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1006" ||
            ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1102") {
        add(21, "getSensorSn", std::bind(&AutoProcessComGuide::getSensorSn, this));
    }

    add(23, "check video", std::bind(&AutoProcessComGuide::checkVideo, this));
    add(30, "tips work", std::bind(&AutoProcessComGuide::tipsWork, this));

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1006") {
        add(31, "checkVideoAuto", std::bind(&AutoProcessComGuide::checkVideoAuto, this));
        add(32, "checkShutter", std::bind(&AutoProcessComGuide::checkShutterAuto, this));
        add(33, "checkVideoManual", std::bind(&AutoProcessComGuide::checkVideoManual, this));
        add(34, "printer", std::bind(&AutoProcessComGuide::printerWork, this));
        add(35, "compare sn", std::bind(&AutoProcessComGuide::workCompareSn, this));
    } else if (ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1102") {
        add(31, "checkRegister", std::bind(&AutoProcessComGuide::checkRegister, this));
        add(32, "checkVideoAuto", std::bind(&AutoProcessComGuide::checkVideoAuto, this));
        add(33, "checkVideoManual", std::bind(&AutoProcessComGuide::checkVideoManual, this));
    }

    add(40, "upload mes", std::bind(&AutoProcessComGuide::mesUpload, this));
    add(50, "out mes", std::bind(&AutoProcessComGuide::outMes, this));
    add(60, "stop", std::bind(&AutoProcessComGuide::stopWork, this));
}

void AutoProcessComGuide::checkNext()
{
//    if(m_errorCode != 0) {
//        msgBox("FAIL 请将不良品放入不良品盒", 3);
//    }
    if(!isWorking()) {
        return;
    }

    QString produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    QString configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(produceName);

    QSettings reader(configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));
    reader.beginGroup("config");

    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();

    initTipsInfo();
    tClear();
    logClear();
    m_errorCode = 0;
    m_frameCount = 0;
    m_seqImage = 0;
    m_errMsg.clear();
    m_skipMesUpload = false;

    m_dataHssd = 0;
    m_dataRaselLow = 0;
    m_dataRaselHigh = 0;
    m_dataRasel = 0;
}

void AutoProcessComGuide::stopWork()
{
    saveCsvReport();

    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    QString path = m_logPath + "/";
    if(m_errorCode == 0) {
        addLog("工作成功 SN: " + m_sn, 1);
        logSaveTxt(path + "PASS_" + dirName +".txt");
        tableSaveCsv(path + "PASS_" + dirName +".csv");
        showPass();
    } else {
        addLog("工作失败 SN: " + m_sn + QString::number(m_errorCode) + "," + m_errMsg, -1);
        if (!m_errMsg.isEmpty())
        showFail(m_errMsg);
        logSaveTxt(path + "FAIL_" + dirName +".txt");
        tableSaveCsv(path + "FAIL_" + dirName +".csv");
    }

//    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1006" ||
//            ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1102") {
//    }
    msgBox("请更换产品进行下一轮测试", 4);
    stopProcess();
}

void AutoProcessComGuide::checkVideo()
{
    if(m_type != 1) return;
    if(!isWorking()) {
        return;
    }

    if (m_errorCode != 0) return;

    addLog("等待正常出视频图像");
    int count = 20;
    int start = m_frameCount;
    while(count) {
        if(m_frameCount - start > 10) {
            break;
        }
        count--;
        QThread::msleep(500);
    }
    if(count <= 0) {
        addLog("视频图像出图异常", -1);
        m_errorCode = -13;
    }

    QThread::msleep(5000);

    AsicControl asic(m_ipAddress);
    asic.ChangeWorkMode(false);
    QThread::msleep(1000);
    asic.SetPattle(0);
    asic.changeSensorFormat(1);
    asic.AutoAdapteComp(0);
}

void AutoProcessComGuide::getSensorSn()
{
    if(!isOnlineMode() || m_errorCode != 0) {
        return;
    }

    m_sensorCode.clear();
    if (!MesCom::instance()->getSensorSN(m_sn, m_sensorCode, "sensor_code")) {
        addLog("获取探测器SN失败!", -1);
        m_errorCode = -8;
        return;
    }
    addLog("获取探测器SN: " + m_sensorCode);
}

void AutoProcessComGuide::getRegister()
{

}

void AutoProcessComGuide::checkVideoAuto()
{
    if(m_errorCode != 0) return;

    addLog("开始图像检测，摄像头对准黑体检测图像!!");
    // 0, 输入框 1, ERR确认框 2, 选择框 3/4, 提示框
    QString result = msgBox("对准黑体检测图像!!", 3);

    AsicControl asic(m_ipAddress);
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);


    asic.changeSensorFormat(3);
    QThread::sleep(2);
    m_videoQueue.clear();
    asic.ShutterComp();
    // 等待出图
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    QByteArray imgY16 = m_videoQueue.at(0);
    if (!dutSaveImage("自动检测图像Y16数据", imgY16)) {
        addLog("自动检测Y16图像保存失败", -1);
        m_errorCode = -13;
        return;
    }

    unsigned char* y8 = new unsigned char[info.width * info.height];
    memset(y8, 0, sizeof(y8));
    getBit8FromY16(imgY16, y8);


    imgCalY8 imgY8;
    addLog("横线检测。。。");
    std::vector<badLineData> vectorRst;
    imgY8.checkHorBadLine(y8, vectorRst, info.width, info.height);
    foreach(badLineData line, vectorRst) {
        QString log = QString("Hor line: %1, starIndex: %2, endIndex: %3").arg(line.line).arg(line.startIndex).arg(line.endIndex);
        addLog(log);
    }

    addLog("竖线检测。。。");
    vectorRst.clear();
    imgY8.checkVerBadLine(y8, vectorRst, info.width, info.height);
    foreach(badLineData line, vectorRst) {
        QString log = QString("Ver line: %1     , starIndex: %2, endIndex: %3").arg(line.line).arg(line.startIndex).arg(line.endIndex);
        addLog(log);
    }

    addLog("灰尘检测。。。");
    std::vector<tRect> outRect;
    imgY8.checkDirtHalcon(y8, outRect, info.width, info.height);
    foreach(tRect rect, outRect) {
        QString log = QString("Dirty (%1, %2), width: %3, height: %4").arg(rect.x).arg(rect.y).arg(rect.w).arg(rect.h);
        addLog(log);
    }
}

void AutoProcessComGuide::checkShutterAuto()
{
    if(m_errorCode != 0) return;

    addLog("开始快门检测...");

    bool result = checkShutter();

    QString resultStr = result ? "PASS" : "NG";
    if (result) {
        resultStr = "PASS";
        addLog("快门检测OK");
    } else {
        m_errorCode = -12;
        resultStr = "NG";
        addLog("快门检测NG", -1);
    }

    addLog("快门检测完成");
}

void AutoProcessComGuide::checkVideoManual()
{
    if(m_errorCode != 0) return;

    AsicControl asic(m_ipAddress);
    asic.changeSensorFormat(1);
    QThread::sleep(2);

    m_videoQueue.clear();

    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    asic.ShutterComp();
    addLog("开始图像检测，摄像头对准黑体检测图像!!");
    QString result = msgBox("对准黑体检测图像，确认是否有坏线!!", 2);
    addLog("图像检测坏线结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_坏线图片", imgY8)) {
            addLog("ERR_坏线保存失败", -1);
            return;
        }
    }


    asic.ShutterComp();
    result = msgBox("对准黑体检测图像，确认是否有灰尘!!", 2);
    addLog("图像检测灰尘结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_灰尘图片", imgY8)) {
            addLog("ERR_灰尘图片保存失败", -1);
            return;
        }
    }

    asic.ShutterComp();
    result = msgBox("对准黑体检测图像，确认是否有其他图像问题!!", 2);
    addLog("图像检测结果： " + result);
    if(result != "PASS") {
        m_errorCode = -13;
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_其他问题图片", imgY8)) {
            addLog("ERR_其他问题图片保存失败", -1);
            m_errorCode = -13;
            return;
        }
    }

}

void AutoProcessComGuide::tipsWork()
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

void AutoProcessComGuide::addLog(QString log, const int  result)
{
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
        showProcess(log);
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

void AutoProcessComGuide::updateTable(QString name, qint64 workTime, bool result)
{
    if(result) {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "PASS");
        emit tableUpdateTestStatus(0);
    } else {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "FAIL");
        emit tableUpdateTestStatus(1);
    }
}

void AutoProcessComGuide::slotsStoped()
{
    addLog("停止工作");
    if(!isWorking()) {
        return;
    }
    startProcess();
}

void AutoProcessComGuide::getSn()
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

    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    m_logPath = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd")
            + "/" + dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
    QDir dir(m_logPath);
    if (!dir.exists()) {
        dir.mkpath(m_logPath);
    }

    addLog("SN: " + m_sn);
}

void AutoProcessComGuide::getLenSn()
{
    if(!isWorking()) {
        return;
    }
    addLog("扫镜头组件SN");
    QString path = m_imgPath + "/SN1.png";
    StaticSignals::getInstance()->statusUpdateImg(path);
    m_lenSn = msgBox("扫镜头组件SN");
    MesCheckItem item;
    item.sItem = "sensor_complete_sn";
    if(m_lenSn.size()< 11) {
        logFail("镜头组件编码失败");
        item.sResult = MESFAILED;
    }else {
        m_lenSn = m_lenSn.left(11);
    }
    addLog("获取镜头编码 " + m_lenSn);

    item.sValue = m_lenSn;
    item.sResult = MESPASS;
    MesCom::instance()->addItemResultEx(item);
}

bool AutoProcessComGuide::dutPing(QString ip)
{
    int iContinue = 0;
    for (int foo = 0; foo < 20; foo++) {
        QCoreApplication::processEvents();
        if (CPingTest::pingTest(nullptr, ip.toLocal8Bit().data())) {
            logNormal("ping ip " + ip + " ok!");
            iContinue++;
            if (iContinue > 3) {
                return true;
            }
        } else {
            logAddNormalLog("ping ip " + ip + " failed!");
            iContinue = 0;
        }
    }

    m_errMsg = "ping 待测IP失败, 检查工装盒是否有上电";
    return false;
}

void AutoProcessComGuide::getInnerSn()
{
    QString ip = dutIP();
    addLog("ping 控制盒: " + ip);
    if (!dutPing(ip)) {
        m_errorCode = -1;
        m_errMsg = "ping 控制盒失败";
        return;
    }
    addLog("ping 控制盒: " + ip + " 完成");

    addLog("开始获取整机SN");
    AsicControl asic(ip);
    for (int foo = 0; foo < 3; foo++) {
        QString sn = asic.GetSn();
        if (sn.size() == 0) {
            QThread::msleep(200);
            continue;
        }
        addLog("获取到产品SN: " + m_sn);
        setCurrentBarcode(sn);
        return;
    }

    m_errorCode = -2;
    m_errMsg = "获取产品SN失败";
}

void AutoProcessComGuide::mesUpload()
{
    if(!isWorking()) {
        return;
    }
    if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
        return;
    }

    if(m_errorCode != 0) {
        m_bResultCode = false;
    } else {
        m_bResultCode = true;
    }

    if (ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1006") {
        if (m_completeSn.isEmpty()) {
            m_skipMesUpload = true; // 这个参数用来跳MES，true为不过站
        }
    }
}

void AutoProcessComGuide::printerWork()
{
    if(!isWorking()) {
        return;
    }
    if(m_errorCode != 0) return ;

    QString orderId = ConfigInfo::getInstance()->cacheInfo().orderId.simplified().left(16);
    QString produceAACode = MesCom::instance()->getMaterialAACode(orderId).simplified();

    m_completeSn = m_sensorCode + produceAACode + QDateTime::currentDateTime().toString("yyMMdd") + orderId;
//    MesCheckItem item;
//    item.sItem = "sensor_complete_sn";
//    item.sValue = m_completeSn;
//    item.sResult = "PASS";
//    MesCom::instance()->addItemResultEx(item);
//    return;

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
//        if(ConfigInfo::getInstance()->sysInfo().mode != 0) {
            modulePath = QCoreApplication::applicationDirPath() + "/res/镜头组件标贴.btw";
//        } else {
//            MesCom::instance()->getproductPrintTemplate(modulePath);
//        }
        qDebug() << "file path is " << modulePath;

        if(!printer.load(printer.defalutPrinterName(), modulePath, 1, 1)) {
            logFail("加载打印机模板失败");
            m_errorCode = -8;
            break;
        }

        QStringList itemList = printer.items();
        for(int i=0; i<itemList.size(); i++) {
            if(itemList[i].contains("ID015")) {
                printer.setItem("ID015", m_completeSn);
            }
            if(itemList[i].contains("ID014")) {
                printer.setItem("ID014", m_sensorCode);
            }
            if(itemList[i].contains("ID003")) {
                printer.setItem("ID003", QDateTime::currentDateTime().toString("yyMMdd"));
            }
        }

        qDebug() << printer.items();


        MesCheckItem item;
        item.sItem = "sensor_complete_sn";
        item.sValue = m_completeSn;
        item.sResult = "PASS";
        MesCom::instance()->addItemResultEx(item);

        addLog("开始打印");
        printer.print();
        addLog("打印完成");

    } while(false);
}

void AutoProcessComGuide::workCompareSn()
{
    if(m_errorCode != 0) return;

    QTime timer;
    timer.start();

    if(m_errorCode == 0) {
        addLog("请扫码比较SN");
        QString sn = msgBox("请扫码比较SN");
        addLog("扫码SN: " + sn);
        addLog("打印SN: " + m_completeSn);
        if(sn != m_completeSn) {
            m_errorCode = -9;
            addLog("SN比对失败", -1);
        }
    }

    QString resultStr = (m_errorCode == 0) ? "PASS" : "NG";
    emit tableAddRowData(QStringList() << "对比标贴" << QString::number(timer.elapsed()) << resultStr);
    addLog("SN比较完成");
}

void AutoProcessComGuide::checkRegister()
{
    if(m_errorCode != 0) return;

    QString deviceName;
    AsicControl asic(m_ipAddress);
    if (!asic.GetDeviceName(deviceName)) {
        m_errMsg = "获取配测产品型号失败";
        m_errorCode = -1;
        return;
    }

    if (deviceName.toUpper().indexOf("X") != 0) {
        m_errMsg = "获取配测产品型号失败";
        m_errorCode = -1;
        return;
    }

    if (deviceName.toUpper().indexOf("XD01A") >= 0) {
        if (!getRegisterDataCom()) {
            m_errMsg = "获取寄存器值失败";
            m_errorCode = -1;
            return;
        }
    } else {
        if (!getRegisterDataI2c()) {
            m_errMsg = "获取寄存器值失败";
            m_errorCode = -1;
            return;
        }
    }
}

bool AutoProcessComGuide::dutSaveImage(QString name, QByteArray data)
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

    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(0, videoInfo);
    unsigned char* m_frameData = new unsigned char[videoInfo.width * videoInfo.height * 4];
    unsigned char* m_rgbData = new unsigned char[videoInfo.width * videoInfo.height * 4];
    unsigned char* m_y8Data = new unsigned char[videoInfo.width * videoInfo.height * 4];
    memset (m_frameData, 0, videoInfo.width * videoInfo.height * 4);
    memset (m_rgbData, 0, videoInfo.width * videoInfo.height * 4);
    memset (m_y8Data, 0, videoInfo.width * videoInfo.height * 4);

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

void AutoProcessComGuide::getBit8FromY16(QByteArray src, unsigned char *&dest)
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

bool AutoProcessComGuide::dutGetRegister_i2c(int page, int addr, int &data)
{

    AsicControl asic(m_ipAddress);
    if (!asic.SendCustom(0x6F0, page)) {
        return false;
    }
    QThread::msleep(50);
    if (!asic.SendCustom(0x6F1, addr)) {
        return false;
    }
    QThread::msleep(50);
    if (!asic.ReadCustom(0x6F2, data)) {
        return false;
    }

    return true;
}

bool AutoProcessComGuide::dutGetRegister_com(int addr, int &data)
{
    QString searchCmd = "55 AA 07 A0 00 80 00 00 00 00 27 F0";
    QJsonObject json;
    json.insert("value", searchCmd);

    HttpClient client;
    client.init(m_ipAddress.toStdString(), 80);
    std::string sOut;
    qDebug() << "执行POST指令：" << QString(QJsonDocument(json).toJson().simplified());
    if (RET_OK != client.post("/uart/test", QString(QJsonDocument(json).toJson().simplified()).toStdString(), sOut)) {
        qDebug() << sOut.c_str();
        return false;
    }
    qDebug() << sOut.c_str();
    QJsonObject object;
    QString sData;
    if(sOut.size() > 0) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
        if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }else {
            object = document.object();
            if(object.contains("value")) {
                sData = object.value("value").toString();
            }
        }
    }

    QByteArray baData = QByteArray::fromHex(sData.toLatin1());
    qDebug() << (unsigned char)baData[0] << (unsigned char)baData[1];
    if ((unsigned char)baData.at(0) != 0x55 || (unsigned char)baData.at(1) != 0xAA) {
        return false;
    }

    data = baData.at(addr);
    return true;
}

bool AutoProcessComGuide::getRegisterDataI2c()
{
    int data = 0;
    if (!dutGetRegister_i2c(3, 9, data)) {
        logFail("读取HSSD数据失败");
        return false;
    }
    m_dataHssd = data;
    logNormal("读取到HSSD数据：" + QString::number((unsigned int)data));


    int data_h = 0, data_l = 0;
    if (!dutGetRegister_i2c(3, 32, data_l)) {
        logFail("读取RASEL_LOW数据失败");
        return false;
    }
    logNormal("读取到RASEL低位数据：" + QString::number((unsigned char)data_l));
    m_dataRaselLow = data_l;

    if (!dutGetRegister_i2c(3, 33, data_h)) {
        logFail("读取RASEL_HIGH数据失败");
        return false;
    }
    logNormal("读取到RASEL高位数据：" + QString::number((unsigned char)data_h));
    m_dataRaselHigh = data_h;

    int dataValue = (data_h << 8) & 0xFF00 | (data_l & 0x00FF);
    logNormal("转换RASEL高低位记录为：" + QString::number(dataValue));

    QString dataStr = QString::number(dataValue, 2);
    logNormal("转换RASEL二进制值为：" + dataStr);
    int count_1 = 0, count_0 = 0;
    for (int foo = 0; foo < dataStr.size(); foo++) {
        if (dataStr.at(foo) == '1') {
            count_1++;
        } else if (dataStr.at(foo) == '0') {
            count_0++;
        }
    }
    m_dataRasel = count_1;
    logNormal("转换RASEL值为：" + QString::number(m_dataRasel));
    if (count_0) {
        logFail("读取数据存在非连续1，RASEL读取失败");
        return false;
    }

    return true;
}

bool AutoProcessComGuide::getRegisterDataCom()
{
    int data_l = 0, data_h = 0, hssd = 0;
    if (!dutGetRegister_com(10, hssd)) {
        logFail("读取HSSD失败");
        return false;
    }
    m_dataHssd = hssd;
    logNormal("读取到HSSD数据：" + QString::number((unsigned int)hssd));

    if (!dutGetRegister_com(6, data_l)) {
        logFail("读取RASEL_LOW失败");
        return false;
    }
    logNormal("读取到RASEL低位数据：" + QString::number((unsigned char)data_l));
    m_dataRaselLow = data_l;
    if (!dutGetRegister_com(5, data_h)) {
        logFail("读取RASEL_HIGH失败");
        return false;
    }
    logNormal("读取到RASEL高位数据：" + QString::number((unsigned char)data_h));
    m_dataRaselHigh = data_h;

    int dataValue = (data_h << 8) & 0xFF00 | (data_l & 0x00FF);
    logNormal("转换RASEL高低位记录为：" + QString::number(dataValue));

    QString dataStr = QString::number(dataValue, 2);
    logNormal("转换RASEL二进制值为：" + dataStr);
    int count_1 = 0, count_0 = 0;
    for (int foo = 0; foo < dataStr.size(); foo++) {
        if (dataStr.at(foo) == '1') {
            count_1++;
        } else if (dataStr.at(foo) == '0') {
            count_0++;
        }
    }
    m_dataRasel = count_1;
    logNormal("转换RASEL值为：" + QString::number(m_dataRasel));
    if (count_0) {
        logFail("读取数据存在非连续1，RASEL读取失败");
        return false;
    }

    return true;
}

void AutoProcessComGuide::saveCsvReport()
{
    QString mTmpLog = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd") + "/";
    QDir dir(mTmpLog);
    if (!dir.exists()) {
        dir.mkpath(mTmpLog);
    }
    logNormal("CSV log path: " + mTmpLog);

    CsvLogger csv;
    csv.setCsvLogPath(mTmpLog + QDate::currentDate().toString("yyyyMMdd") + ".csv");

    bool rst = (m_errorCode == 0);
    QStringList title;
    title << "测试时间" << "整机SN" << "测试结果" << "错误信息"
          << "测试模式" << "HSSD" << "RASEL_HIGH" << "RASEL_LOW" << "RASEL";
    csv.addCsvTitle(title);

    QStringList values;
    values << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")
           << m_sn << (rst ? "PASS" : "FAIL") << m_errMsg << testModeStr()
           << QString::number(m_dataHssd) << QString::number((unsigned char)m_dataRaselHigh)
           << QString::number((unsigned char)m_dataRaselLow) << QString::number(m_dataRasel);

    csv.addCsvLog(values);
}

bool AutoProcessComGuide::checkShutter()
{
    QByteArray Y16_1, Y16_2, Y16_3;
    // 1.
    AsicControl asic(m_ipAddress);
    asic.changeSensorFormat(3); // Y16

    QThread::msleep(1500);
    m_videoQueue.clear();

    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    addLog("获取快门检测Y16图片1!!");
    Y16_1 = m_videoQueue.at(0);
    if(!dutSaveImage("快门检测1", Y16_1)) {
        return false;
    }

    addLog("关闭快门，获取快门检测Y16图片2!!");
    asic.ChangeWorkMode(true);
    QThread::msleep(1500);
    m_videoQueue.clear();
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    Y16_2 = m_videoQueue.at(0);
    if(!dutSaveImage("快门检测2", Y16_2)) {
        return false;
    }

    addLog("开启快门，获取快门检测Y16图片3!!");
    asic.ChangeWorkMode(false);
    QThread::msleep(1500);
    m_videoQueue.clear();
    while (m_videoQueue.size() == 0) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
    }
    Y16_3 = m_videoQueue.at(0);
    if(!dutSaveImage("快门检测3", Y16_3)) {
        return false;
    }

    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);
    int WIDTH = info.width;
    int HEIGHT = info.height;

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
    bRst &= (rang1 > 0 && rang1 <= 2000);
    bRst &= (rang2 > 0 && rang2 <= 150);
    bRst &= (rang3 > 0 && rang3 <= 2000);
    bRst &= (avg_diff2 >= 800);
    bRst &= (avg_diff3 > 0 && avg_diff3 <= 200);

    QString log;
    log.sprintf("快门检测结果，rang1: %d, rang2: %d, rang3: %d, avg_diff2: %d, avg_diff3: %d",
                rang1, rang2, rang3, avg_diff2, avg_diff3);
    addLog(log);

    return bRst;
}

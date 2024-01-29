#include "dustimagedetection.h"
#include <QSettings>
#include <QTextCodec>
#include <QTime>

#include "StaticSignals.h"

#include "util.h"
#include "videoinfo.h"
#include "configinfo.h"
#include "CMyPrinter.h"
#include "imgCalY8.h"
#include "asiccontrol.h"
#include "yuv2rgb.h"
#include "mapping.h"
#include "mappingnew.h"
#include "Frock_cal.h"
#include "csvlogger.h"
#include <WinBase.h>
DustImageDetection::DustImageDetection()
{
    m_mcuSerial = new McuSerialManager();
    m_handleScanner = new CSerialDirect();
    connect(pMachine(), &QStateMachine::stopped,
            this, &DustImageDetection::slotStartWorkBtnClicked);

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    qDebug() << "Current Station Name: " << m_StationName;
    mMesObj.setUser(MesClient::instance()->sUser());
    int step{0};

    add(step++, "judgeCustomLifeTime", std::bind(&DustImageDetection::jugdeCustomLifeTime, this, true));
    add(step++, "do_init", std::bind(&DustImageDetection::do_init, this));
    add(step++, "do_work", std::bind(&DustImageDetection::do_work, this));
    add(step++, "do_finish", std::bind(&DustImageDetection::do_finish, this));

    add(step++, "mes_upload", std::bind(&DustImageDetection::mes_upload, this));
    add(step++, "enterMes", std::bind(&DustImageDetection::enterMes, this));
    add(step++, "stop_work", std::bind(&DustImageDetection::stop_work, this));

}

DustImageDetection::~DustImageDetection()
{
    m_dustImageOpt.close();
    m_dustImageOpt.freeHandle();
}

void DustImageDetection::do_init()
{
    msgBox("请放入产品，并按按钮进行本轮测试", 4);
    StaticSignals::getInstance()->statusChangeVideoPage(0);
    StaticSignals::getInstance()->statusUpdateImg("/blank.png");
    m_sn.clear();
    m_errorCode = 0;
    m_errMsg.clear();
    tClear();
    logClear();
    m_errorCode = 0;
    m_frameCount = 0;
    m_seqImage = 0;
    m_errMsg.clear();
    m_completeSn.clear();

    m_skipMesUpload = false;

    m_dataHssd = 0;
    m_dataRaselLow = 0;
    m_dataRaselHigh = 0;
    m_dataRasel = 0;

    QString path = m_imgPath + "/空白.png";
    StaticSignals::getInstance()->statusUpdateImg(path);
    //MCU串口检测
    addLog("开启MCU串口");
    int ret = m_mcuSerial->openSerial(m_mcuCom);
    if(!ret) {
        showFail("开启MCU串口失败");
        return;
    }

}

void DustImageDetection::do_work()
{
    using namespace DustImageFunctionName;
    bool ret{false};

    addLog("正在查询模具后检测信号");
    while(!m_dustImageOpt.is_status(D800,D800_MOLD_BACK)){
        QThread::msleep(100);
    }

    addLog("锁模上锁中");
    while(!m_dustImageOpt.clamping_control(ON)){
        QThread::msleep(100);
    }
    show_picture(0);
    addLog("请将镜头组件放入到治具中,正在检测产品有无");
    while(!m_dustImageOpt.is_status(D800,D800_PRODUCT_DEC)){
        QThread::msleep(100);
    }

    addLog("请扣好排线和快门线,扣好后，请手动按下OK指示按钮");
    show_picture(1);
    while(!m_dustImageOpt.is_status(D801,D801_CLAMPING_START)){//D802,D802_OK_LIGHT 此处与文档不符
        QThread::msleep(100);
    }
    StaticSignals::getInstance()->statusChangeVideoPage(1);
    addLog("正在给产品上电");
    if(!mcu_power_on()){
        addLog("产品上电失败",-1);
        m_errorCode = -1;
        return;
    }

    addLog("锁模解锁中");
    while(!m_dustImageOpt.clamping_control(OFF)){
        QThread::msleep(100);
    }
    addLog("请将运动滑台推到检测位");
    while(!m_dustImageOpt.is_status(D800,D800_MOLD_FORWARD)){
        QThread::msleep(100);
    }
    addLog("锁模上锁中");
    while(!m_dustImageOpt.clamping_control(ON)){
        QThread::msleep(100);
    }
    QThread::msleep(500);
    addLog("气缸翻转中");
    while(!m_dustImageOpt.flip_control(ON)){
        QThread::msleep(100);
    }

    addLog("开始自动扫码,并绑定入栈");
    get_sn();
    enterMes();
    if(!get_sensor_sn()){
        m_errorCode = -2;
        return;
    }

    if(!check_video()){
        addLog("视频接收失败",-1);
        m_errorCode = -2;
        return;
    }

    QThread::msleep(1000);
    addLog("气缸旋转中");
    while(!m_dustImageOpt.rotate_control(ON)){
        QThread::msleep(100);
    }
    addLog("等待气缸旋转到工作位置");
    while(!m_dustImageOpt.is_status(D800,D801_FLIP_WORK)){
        QThread::msleep(100);
    }
    QThread::msleep(3000);
    addLog("开始检测");
    QString codeId = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    //灰尘检测
    if("CL1102"==codeId){
        if(!check_register()){
            m_errorCode = -3;
            return;
        }
        if(!check_video_auto()){
            m_errorCode = -3;
            return;
        }
        if(!check_video_manual()){
            m_errorCode = -3;
            return;
        }
    }//EOL
    else if("CL1006"==codeId){
        if(!check_shutter()){
            m_errorCode = -3;
            return;
        }
        if(!check_video_auto()){
            m_errorCode = -3;
            return;
        }
        if(!check_video_manual()){
            m_errorCode = -3;
            return;
        }
//        if(!printer_work()){
//            m_errorCode = -3;
//            return;
//        }
//        if(!work_compare_sn()){
//            m_errorCode = -3;
//            return;
//        }
    }

    while(!m_dustImageOpt.green_control(ON)){
        QThread::msleep(100);
    }
    addLog("产品下电中");
    if(!mcu_power_off()){
        m_errorCode = -4;
        m_errMsg = "产品下电失败";
        return;
    }
}

void DustImageDetection::do_finish()
{
    using namespace DustImageFunctionName;
    bool ret{false};

    if(-3 == m_errorCode){
      ng_step1();
    }
    if(0 == m_errorCode || -3 == m_errorCode){

      finish_step1();
    }

    if(-2 == m_errorCode){
        ng_step2();
    }

    finish_step2();

}
void DustImageDetection::save_csv_report()
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
void DustImageDetection::stop_work()
{
    save_csv_report();

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

    m_mcuSerial->closeSerial();

//    msgBox("请更换产品进行下一轮测试", 4);
    stopProcess();
}
void DustImageDetection::finish_step1()
{
    using namespace DustImageFunctionName;
    QThread::msleep(1000);
    addLog("气缸回旋中");
    while(!m_dustImageOpt.rotate_control(OFF)){
        QThread::msleep(100);
    }
    QThread::msleep(1000);
    addLog("气缸回翻中");
    while(!m_dustImageOpt.flip_control(OFF)){
        QThread::msleep(100);
    }
}

void DustImageDetection::finish_step2()
{
    using namespace DustImageFunctionName;
    addLog("锁模解锁中");
    while(!m_dustImageOpt.clamping_control(OFF)){
        QThread::msleep(100);
    }
    addLog("请将产品拉到上下料位，查询模具后检测信号中");
    while(!m_dustImageOpt.is_status(D800,D800_MOLD_BACK)){
        QThread::msleep(100);
    }
    addLog("锁模上锁中");
    while(!m_dustImageOpt.clamping_control(ON)){
        QThread::msleep(100);
    }

    addLog("请取出镜头组件,正在检测产品有无");
    while(!m_dustImageOpt.is_not_status(D800,D800_PRODUCT_DEC)){
        QThread::msleep(100);
    }
    addLog("锁模解锁中");
    while(!m_dustImageOpt.clamping_control(OFF)){
        QThread::msleep(100);
    }
    //检测中存在错误则关闭红灯 反之则关闭绿灯
    if(-2 == m_errorCode  || -3 == m_errorCode){
        while(!m_dustImageOpt.red_control(OFF)){
            QThread::msleep(100);
        }
    }else{
        while(!m_dustImageOpt.green_control(OFF)){
            QThread::msleep(100);
        }
    }
}

void DustImageDetection::ng_step1()
{
    using namespace DustImageFunctionName;
    addLog("开启NG灯");
    while(!m_dustImageOpt.red_control(ON)){
        QThread::msleep(100);
    }
    addLog("请手动长按红色NG灯");
    while(!m_dustImageOpt.is_status(D800,D800_NG_RESET)){
        QThread::msleep(100);
    }
    addLog("产品下电中");
    if(!mcu_power_off()){
        m_errorCode = -4;
        m_errMsg = "产品下电失败";
        return;
    }

}

void DustImageDetection::ng_step2()
{
    addLog("开启NG灯",-1);
    while(!m_dustImageOpt.red_control(ON)){
        QThread::msleep(100);
    }
    addLog("请手动长按红色NG灯");
    while(!m_dustImageOpt.is_status(D800,D800_NG_RESET)){
        QThread::msleep(100);
    }
    addLog("产品下电中");
    if(!mcu_power_off()){
        m_errorCode = -4;
        m_errMsg = "产品下电失败";
        return;
    }
    QThread::msleep(1000);
    addLog("气缸回翻中");
    while(!m_dustImageOpt.flip_control(OFF)){
        QThread::msleep(100);
    }
}

void DustImageDetection::slotConnectBtnClicked()
{
    int iContinue{0};
    emit resultShowDefault();
    emit tableAddHeader(QStringList() << "检测项" << "工作时间(ms)" << "结果");
    QString codeId = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    QString produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    QString configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(produceName);

    QSettings reader(configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));
    reader.beginGroup("config");

    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();

    ComGuideCfg::getGuideCfgList(m_tipsList);
    ConfigInfo::getInstance()->getValueString(codeId, "MCU串口", m_mcuCom);
    ConfigInfo::getInstance()->getValueString(codeId, "工装串口", m_machineCom);
    ConfigInfo::getInstance()->getValueString(codeId, "扫码串口", m_portScanner);
    m_ipAddress = ConfigInfo::getInstance()->getValueString("盒子IP");
    addLog("MCU串口:"+m_mcuCom);
    addLog("工装串口:"+m_machineCom);
    addLog("盒子IP:"+m_ipAddress);

    //盒子连接检测

    if(Util::testPing(m_ipAddress)) {
        if(m_webSocket == nullptr) {
            m_webSocket = new WebSocketClientManager();
            m_webSocket->connectToUrl(m_ipAddress, 6000);
            connect(m_webSocket,&WebSocketClientManager::signalRecvVideoFrame,this,&DustImageDetection::slotRecvVideoFrameWs);
        }
    } else {
        addLog("网络异常, 请检查工装盒ip及网络连接", -1);
        return;
    }

    addLog("开启PLC工装串口");
    bool rst = m_dustImageOpt.open(m_machineCom, 19200, true,8,1,0,2);
    if (!rst) {
        //m_dustImageOpt.close();
        //m_dustImageOpt.freeHandle();
        showFail("开启PLC工装串口失败");
        return;
    }
    addLog("发送联机");
    rst = m_dustImageOpt.status_control(ON);
    if(!rst){
        m_dustImageOpt.close();
//        m_dustImageOpt.freeHandle();
        showFail("工装连机失败");
        return;
    }
//    m_dustImageOpt.close();

    BaseProduce::slotConnectBtnClicked();
}

void DustImageDetection::slotStartWorkBtnClicked()
{
    emit tableClearData();
    emit logClearData();
    emit resultShowDefault();
    m_bWorkingStatus = true;
    qInfo("start ");
    startProcess();
}

void DustImageDetection::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
}

void DustImageDetection::slotRecvVideoFrameWs(QByteArray frameData, FrameInfo_S frameInfo)
{
//    qDebug() << (QString("接受图像数据格式：uNvsLen:%1 uY16Len:%2 uYuvLen:%3 uParamLen:%4 uFormat:%5")
//                 .arg(frameInfo.uNvsLen).arg(frameInfo.uY16Len)
//                 .arg(frameInfo.uYuvLen).arg(frameInfo.uParamLen).arg(frameInfo.uFormat));
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



void DustImageDetection::addLog(QString log, const int  result)
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

void DustImageDetection::show_picture(int idx)
{
    if(m_tipsList.size() > idx){
        StaticSignals::getInstance()->statusUpdateImg(m_tipsList.at(idx).path);
    }else{
        addLog("无法显示图片，请检查图片配置参数");
    }
}

bool DustImageDetection::mcu_power_on()
{
    QString proj = projectName();
    int vol = 12;
    if (proj.contains("XD01A")) {
        vol = 15;
    } else {
        vol = 12;
    }
    addLog("设置电压");
    bool ret = m_mcuSerial->controlSensorPower(vol, true, false);
    if(!ret){
        m_errMsg = "电压设置失败.";
        addLog("电压设置失败.", -1);
        return false;
    }
    QThread::msleep(2000);// 等待稳定
    addLog("读取电压");
    float dataVoltage = m_mcuSerial->getSensorVoltage();
    QString str = QString("供给电压(V):%1,获取到实际电压(V):%2").arg(vol).arg(QString::number(dataVoltage, 'f', 4));
    addLog(str);
//    if (dataVoltage > (vol + 0.5) || dataVoltage < (vol - 0.5)) {
//        m_errMsg = "上电电压比对失败, 超门限";
//        addLog("上电电压比对失败, 超门限", -1);
//        return false;
//    }
    return true;
}

bool DustImageDetection::mcu_power_off()
{
    return m_mcuSerial->controlSensorPower(0, true, false);
}

bool DustImageDetection::get_sensor_sn()
{
    if(!isOnlineMode() || m_errorCode != 0) {
        return true;
    }

    m_sensorCode.clear();
    if (!MesCom::instance()->getSensorSN(m_sn, m_sensorCode, "sensor_code")) {
        addLog("获取探测器SN失败!", -1);
        m_errMsg = "获取探测器SN失败!";
        return false;
    }
    addLog("获取探测器SN: " + m_sensorCode);

    if(!isWorking()) {
       return false;
    }
    if(m_errorCode != 0) return false;

    QString orderId = ConfigInfo::getInstance()->cacheInfo().orderId.simplified().left(16);
    QString produceAACode = MesCom::instance()->getMaterialAACode(orderId).simplified();

    m_completeSn = m_sensorCode + produceAACode + QDateTime::currentDateTime().toString("yyMMdd") + orderId;
    MesCheckItem item;
    item.sItem = "sensor_complete_sn";
    item.sValue = m_completeSn;
    item.sResult = "PASS";
    MesCom::instance()->addItemResultEx(item);
    return true;
}

void DustImageDetection::get_sn()
{
//    m_sn = msgBox("获取产品SN");
    getAutoBarcode(m_sn);
    emit logAddNormalLog("产品SN:" + m_sn);
    setCurrentBarcode(m_sn); // 显示到界面


    QString dirName = m_sn.left(15);
    dirName = dirName.replace("*", "_");
    m_logPath = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd")
                + "/" + dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
    QDir dir(m_logPath);
    if (!dir.exists()) {
       if(!dir.mkpath(m_logPath)){
           addLog(QString("创建路径失败:%1").arg(m_logPath));
       }
    }
}

void DustImageDetection::mes_upload()
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

bool DustImageDetection::dutSaveImage(QString name, QByteArray data)
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
       }else addLog("image save to " + path + " fail");
    }else{
        addLog("image is null");
    }
    addLog(QString("快门检测中：sensorFormat:%1 format:%2 width:%3 height:%4 nvsLen:%5 frameLen:%6 paramLen:%7")
           .arg(videoInfo.sensorFormat).arg(videoInfo.format).arg(videoInfo.width)
           .arg(videoInfo.height).arg(videoInfo.nvsLen).arg(videoInfo.frameLen).arg(videoInfo.paramLen));

    delete []m_rgbData;
    delete []m_frameData;
    delete []m_y8Data;
    return false;
}

bool DustImageDetection::check_shutter()
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
        m_errorCode = -3;
        m_errMsg = "快门检测1失败";
        addLog("快门检测1失败",-1);
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
        m_errorCode = -3;
        m_errMsg = "快门检测2失败";
        addLog("快门检测2失败",-1);
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
        m_errorCode = -3;
        m_errMsg = "快门检测3失败";
        addLog("快门检测3失败",-1);
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
    if(!bRst){
        addLog("快门检测结果不合格",-1);
    }
    return bRst;
}
bool DustImageDetection::check_video_auto()
{
    if(m_errorCode != 0) return false;

    addLog("开始图像检测，摄像头对准黑体检测图像!!");
    // 0, 输入框 1, ERR确认框 2, 选择框 3/4, 提示框
    msgBox("对准黑体检测图像!!", 3);

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
        m_errorCode = -3;
        m_errMsg = "自动检测Y16图像保存失败";
        return false;
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
    return true;
}

bool DustImageDetection::check_video()
{
    if(!isWorking()) {
        addLog("is not Working", -1);
        return false;
    }

    if (m_errorCode != 0) {
        addLog("已产生错误代码", -1);
        return false;
    }

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
        m_errMsg = "视频图像出图异常";
        return false;
    }

    QThread::msleep(5000);

    AsicControl asic(m_ipAddress);
    asic.ChangeWorkMode(false);
    QThread::msleep(1000);
    asic.SetPattle(0);
    asic.changeSensorFormat(1);
    asic.AutoAdapteComp(0);
    return true;
}
void DustImageDetection::getBit8FromY16(QByteArray src, unsigned char *&dest)
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
void DustImageDetection::updateTable(QString name, qint64 workTime, bool result)
{
    if(result) {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "PASS");
        emit tableUpdateTestStatus(0);
    } else {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "FAIL");
        emit tableUpdateTestStatus(1);
    }
}

bool DustImageDetection::printer_work()
{
    if(!isWorking()) {
        return false;
    }
    if(m_errorCode != 0) return false;

    addLog("打开打印机");
    do {
        CMyPrinter printer;
        QString printerName = printer.defalutPrinterName();
        if(printerName.isEmpty()) {
            logFail("获取打印机失败");
            m_errorCode = -8;
            return false;
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
            return false;
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


        addLog("开始打印");
        printer.print();
        addLog("打印完成");

    } while(false);
    return true;
}
bool DustImageDetection::work_compare_sn()
{
    if(m_errorCode != 0) return false;

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
            return false;
        }
    }

    QString resultStr = (m_errorCode == 0) ? "PASS" : "NG";
    emit tableAddRowData(QStringList() << "对比标贴" << QString::number(timer.elapsed()) << resultStr);
    addLog("SN比较完成");
    return true;
}
bool DustImageDetection::check_video_manual()
{
    if(m_errorCode != 0) return false;

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
    QString result = msgBox("对准黑体检测图像，确认是否有坏线!!", 4);
    addLog("图像检测坏线结果： " + result);
    if(result != "PASS") {
        m_errorCode = -3;
        m_errMsg = "图像检测存在坏线";
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_坏线图片", imgY8)) {
            addLog("ERR_坏线保存失败", -1);
            return false;
        }
        return false;
    }


    asic.ShutterComp();
    result = msgBox("对准黑体检测图像，确认是否有灰尘!!", 4);
    addLog("图像检测灰尘结果： " + result);
    if(result != "PASS") {
        m_errorCode = -3;
        m_errMsg = "图像检测存在灰尘";
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_灰尘图片", imgY8)) {
            addLog("ERR_灰尘图片保存失败", -1);
            return false;
        }
        return false;
    }

    asic.ShutterComp();
    result = msgBox("对准黑体检测图像，确认是否有其他图像问题!!", 4);
    addLog("图像检测结果： " + result);
    if(result != "PASS") {
        m_errorCode = -3;
        m_errMsg = "图像检测存在其它灰尘";
        QByteArray imgY8 = m_videoQueue.at(0);
        if (!dutSaveImage("ERR_其他问题图片", imgY8)) {
            addLog("ERR_其他问题图片保存失败", -1);
            return false;
        }
        return false;
    }
    return true;
}
bool DustImageDetection::check_register()
{
    if(m_errorCode != 0) return false;

    QString deviceName{""};
    AsicControl asic(m_ipAddress);
    if (!asic.GetDeviceName(deviceName)) {
        m_errMsg = "获取配测产品型号失败";
        m_errorCode = -3;
        return false;
    }

    if (deviceName.toUpper().indexOf("X") != 0) {
        m_errMsg = "获取配测产品型号失败";
        m_errorCode = -3;
        return false;
    }

    if (deviceName.toUpper().indexOf("XD01A") >= 0) {
        if (!get_register_data_com()) {
            m_errMsg = "获取寄存器值失败";
            m_errorCode = -3;
            return false;
        }
    } else {
        if (!get_register_data_iic()) {
            m_errMsg = "获取寄存器值失败";
            m_errorCode = -3;
            return false;
        }
    }
    return true;
}
bool DustImageDetection::get_register_data_com()
{
    int data_l = 0, data_h = 0, hssd = 0;
    if (!dut_get_register_com(10, hssd)) {
        logFail("读取HSSD失败");
        return false;
    }
    m_dataHssd = hssd;
    logNormal("读取到HSSD数据：" + QString::number((unsigned int)hssd));

    if (!dut_get_register_com(6, data_l)) {
        logFail("读取RASEL_LOW失败");
        return false;
    }
    logNormal("读取到RASEL低位数据：" + QString::number((unsigned char)data_l));
    m_dataRaselLow = data_l;
    if (!dut_get_register_com(5, data_h)) {
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
bool DustImageDetection::get_register_data_iic()
{
    int data = 0;
    if (!dut_get_register_iic(3, 9, data)) {
        logFail("读取HSSD数据失败");
        return false;
    }
    m_dataHssd = data;
    logNormal("读取到HSSD数据：" + QString::number((unsigned int)data));


    int data_h = 0, data_l = 0;
    if (!dut_get_register_iic(3, 32, data_l)) {
        logFail("读取RASEL_LOW数据失败");
        return false;
    }
    logNormal("读取到RASEL低位数据：" + QString::number((unsigned char)data_l));
    m_dataRaselLow = data_l;

    if (!dut_get_register_iic(3, 33, data_h)) {
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
bool DustImageDetection::dut_get_register_com(int addr, int &data)
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
bool DustImageDetection::dut_get_register_iic(int page, int addr, int &data)
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
bool DustImageDetection::getAutoBarcode(QString &sn)
{
    uint8_t cmd[3]{0x01,0x54,0x04};
    if (!m_handleScanner) {
        return false;
    }

    qDebug() << "open scanner";
    m_handleScanner->close();
    if (!m_handleScanner->open(m_portScanner, 9600, false)) {
        return false;
    }

    char szBarcode[32] = {0};
    int nLen = 0;
    char* pBarcode = (char*)szBarcode;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        QThread::msleep(200);
        m_handleScanner->write(cmd, sizeof(cmd));
        QThread::msleep(200);
        if (m_handleScanner->get(nullptr, 0, nullptr, 15, pBarcode, nLen, 3000)) {
            sn = QString::fromLatin1(pBarcode, nLen);
            sn = sn.split('\n').at(0);
            qDebug() << "size: " << nLen << " pBarcode_SN: " << sn;
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

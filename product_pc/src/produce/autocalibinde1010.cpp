#include "autocalibinde1010.h"
#include "asiccontrol.h"
#include <plcautocalibin.h>
#include <IRCalibIntrinsic.h>
#include <videoinfo.h>
#include <yuv2rgb.h>
#include <QImage>
#include <util.h>
#include <tcpclient.h>
#include <messageid.h>
#include <QJsonArray>
#include <QSerialPortInfo>
#include <CPingTest.h>
#include <QElapsedTimer>
#include <QPainter>
#include <csvlogger.h>

//static const QMap<int, QString> g_errorCodeMap1010 = {
//    {-100, "视频检测失败"},
//    {-101, "光轴偏差失败"},
//    {-102, "PLC串口打开失败"},
//    {-103, "运动轴运动失败"},
//    {-104, "内参计算失败"},
//    {-105, "内参写入错误"},
//    {-106, "获取SN失败"},
//};

typedef enum {
    ItemRadus = 0,
    ItemFx,
    ItemFy,
    ItemCx,
    ItemCy,
    ItemK1,
    ItemK2,
    ItemK3,
    ItemP1,
    ItemP2,
    ItemRms
}ItemCheck;

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item  value minvalue maxvalue exp result
    {"calib_radius", "", "0","26.48","0~26.48"},
    {"calib_fx", "", "730","790","730~790"},
    {"calib_fy", "", "730","790","730~790"},
    {"calib_cx", "", "270","370","270~370"},
    {"calib_cy", "", "226","286","226~286"},
    {"calib_k1", "", "-1","1","-1~1"},
    {"calib_k2", "", "-1","1","-1~1"},
    {"calib_k3", "", "-1","1","-1~1"},
    {"calib_p1", "", "-1","1","-1~1"},
    {"calib_p2", "", "-1","1","-1~1"},
    {"calib_rms", "", "0","0.72","0~0.72"},
    {""},
};
void AutoCalibInDe1010::slotVideoSlot(QByteArray frameData, FrameInfo_S frameInfo)
{
    m_frameCount = m_frameCount + 1;
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

    mByteData = frameData;

    memcpy(m_frameData, frameData.data() + info.nvsLen, info.frameLen);

    emit videoUpdate(0, frameData, frameInfo.uTimeTm);
}

bool AutoCalibInDe1010::plcInit()
{
    if (!plcRelative) return false;
    if (!mPlcSupported) return true;


    //plcRelative->deInitResource();
    plcRelative->freeSerialResource();
    if (!plcRelative->getSerialResource()) {
        mErrString = "PLC获取串口句柄失败";
        return false;
    }
    logNormal("Read: " + mPlcReadSerial + " Write: " + mPlcWriteSerial);
    if (!plcRelative->initResource(mPlcWriteSerial, 9600, mPlcReadSerial, 9600)) {
        mErrString = plcRelative->errorString();
        return false;
    }

    return true;
}

AutoCalibInDe1010::AutoCalibInDe1010()
{
    if(!m_videoClient) {
        m_videoClient = new WebSocketClientManager;
    }
    plcRelative = new CPlcRelative;

    connect(pMachine(), &QStateMachine::stopped,this, &AutoCalibInDe1010::slotsStoped);
//    WSFRAMECALL funFrame = std::bind(&AutoCalibInDe1010::recvFrame, this, std::placeholders::_1, std::placeholders::_2);
//    m_videoClient->regFrameCall(funFrame);

    QObject::connect(m_videoClient, &WebSocketClientManager::signalRecvVideoFrame, this, &AutoCalibInDe1010::slotVideoSlot);

    m_rgbData = new unsigned char[640*512*4];
    m_frameY = new unsigned char[640*512 * 4];
    m_frameData = new unsigned char[640*512*4];
    add(0, "workInit", std::bind(&AutoCalibInDe1010::workInit, this));
    add(1, "Life Time", std::bind(&AutoCalibInDe1010::jugdeCustomLifeTime, this, true));
    add(2, "workCheckVideo", std::bind(&AutoCalibInDe1010::workCheckVideo, this));
    add(3, "workMoveRaise", std::bind(&AutoCalibInDe1010::workMoveRaise, this));
    add(4, "workMoveCheckAxis", std::bind(&AutoCalibInDe1010::workMoveCheckAxis, this));
    add(5, "workCheckAxisCalculate", std::bind(&AutoCalibInDe1010::workCheckAxisCalculate, this));
    add(6, "workMoveCapture", std::bind(&AutoCalibInDe1010::workMoveCapture, this));
    add(7, "workCalculateParam", std::bind(&AutoCalibInDe1010::workCalculateParam, this));
    add(8, "workHandleDown", std::bind(&AutoCalibInDe1010::workHandleDown, this));
    add(9, "workMoveOrigin", std::bind(&AutoCalibInDe1010::workMoveOrigin, this));
    add(10, "reportResult", std::bind(&AutoCalibInDe1010::reportResult, this));

    bool round = false;
    QList<int> distances;
    QList<int> directions;
    directions << PLC_AXIS_DIRECITON_30N << PLC_AXIS_DIRECITON_15N << PLC_AXIS_DIRECITON_0
              << PLC_AXIS_DIRECITON_15P << PLC_AXIS_DIRECITON_30P;
    distances << PLC_AXIS_DISTANCE_45CM << PLC_AXIS_DISTANCE_50CM << PLC_AXIS_DISTANCE_60CM
              << PLC_AXIS_DISTANCE_70CM << PLC_AXIS_DISTANCE_80CM << PLC_AXIS_DISTANCE_90CM
              << PLC_AXIS_DISTANCE_100CM;
    int i=0,j=0;
    for(i=0; i<directions.size();i++) {
        if(round) {
            for(j=0; j<=distances.size()-1; j++) {
                WorkPosition position;
                position.direction = directions[i];
                position.distance = distances[j];
                m_workList.append(position);
            }
        } else {
            for(j=distances.size()-1; j>=0; j--) {
                WorkPosition position;
                position.direction = directions[i];
                position.distance = distances[j];
                m_workList.append(position);
            }
        }
        round = !round;
    }

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;

}

void AutoCalibInDe1010::workInit()
{
    writeLog("初始化操作");
    // 记录开始时间
    mStartPoint = std::chrono::system_clock::now();

    { // 界面和数据初始化
        emit tableClearData();
        emit logClearData();
        mErrString.clear();
        m_errorCode = 0;
        m_ageErrorCode = 0;
        mBoolIgnoreUpload = false;
        memset(&m_calibParam, 0, sizeof(m_calibParam));
        mMainLogPath = mLogPath + "/" + QDate::currentDate().toString("yyyyMMdd") + "/";
        emit waterPrintVisible(1, false);
        mBoolFixtureHasRaised = false;
        mOffsetAxis = QPoint(0, 0);
    }


    int fixOffsetX = 0, fixOffsetY = 0;
    bool bRst = ConfigInfo::getInstance()->getValueInt32(m_StationName, "offsetX", fixOffsetX);
    bRst &= ConfigInfo::getInstance()->getValueInt32(m_StationName, "offsetY", fixOffsetY);
    if (!bRst) {
        m_errorCode = -9;
        writeLog("获取环境偏移坐标失败", -1);
        return;
    }
    m_fixEnvOffset = QPoint(fixOffsetX, fixOffsetY);
    writeLog("当前环境往中心固定偏移为：(" + QString::number(m_fixEnvOffset.x()) + ", " + QString::number(m_fixEnvOffset.y()) + ")", 2);

    crossAxisInit();

    writeLog("开始 ping ip " + mBoxIP);
    if (!dutPing()) {
        writeLog(mErrString, -1);
        mBoolIgnoreUpload = true;
        m_errorCode = -99;
        return;
    }

    writeLog("获取SN");
    if (false == getCurrentSN()) {
        writeLog(mErrString, -1);
        mBoolIgnoreUpload = true;
        m_errorCode = -12;
        return;
    }


    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    mImageSavePath = mMainLogPath + dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
    QDir dir;
    if (!dir.exists(mImageSavePath)) {
        writeLog("创建图片保存路径：" + mImageSavePath);
        dir.mkpath(mImageSavePath);
        if (!dir.exists()) {
            mErrString = "创建图片路径失败：" + mImageSavePath;
            mBoolIgnoreUpload = true;
            m_errorCode = -12;
            return;
        }
    }

    AsicControl asic(mBoxIP);
    asic.ChangeWorkMode(false);

    writeLog("连接视频");
    m_videoClient->stopConnect();
    m_videoClient->connectToUrl(mBoxIP, 6000);

    m_productName.clear();
    asic.GetDeviceName(m_productName);
    if (m_productName.at(0) != 'X') {
        mErrString = "获取产线型号失败";
        writeLog(mErrString, -1);
        mBoolIgnoreUpload = true;
        m_errorCode = -12;
        return;
    }

    writeLog("切白热模式");
    if (!dutSwitchPattle(0)) {
        m_errorCode = -99;
        writeLog("切换白热模式失败", -1);
        return;
    }

    writeLog("执行入站操作：" + m_sn);
    if (isOnlineMode()) {
        mMesObj.setUser("gd2222");
        writeLog("执行入栈操作 SN：" + m_sn);
        QString out;
        if (!mMesObj.enterProduce(m_sn, out)) {
            writeLog("入站失败: " + out, -1);
            m_errorCode = -99;
            mBoolIgnoreUpload = true;
            return;
        }
        writeLog("入站OK: " + m_sn);
    } else {
        logWarming("离线模式不进行入栈操作");
    }
}

void AutoCalibInDe1010::workMoveRaise()
{
    if ((m_errorCode != 0) || !mPlcSupported) return;

    showProcess("抬起待测夹具");
    bool bRet = plcRelative->plcMoveUp();
    if(!bRet) {
        writeLog("抬起夹具失败", -1);
        m_errorCode = -99;
        mBoolIgnoreUpload = true;
    }else {
        logNormal("抬起夹具PASS" );
        mBoolFixtureHasRaised = true;
    }
}

void AutoCalibInDe1010::workCheckVideo()
{
    writeLog("error code " + QString::number(m_errorCode));
    if (m_errorCode != 0) return;

    writeLog("检测视频是否有输出");
    m_frameCount = 0;
    int lstCount = m_frameCount;
    bool bCheckOk = false;
    QElapsedTimer timer;
    timer.start();
    writeLog("检测视频...");
    while(timer.elapsed() < 20 * 1000) {
        QCoreApplication::processEvents();
        if( lstCount < m_frameCount && (int)25 < m_frameCount )
        {
            bCheckOk = true;
            break;
        }
        lstCount = m_frameCount;
        QThread::usleep(100);
    }

    if(!bCheckOk) {
        writeLog("检测视频 失败", -1);
        m_errorCode = -2;
        return;
    }
    writeLog("检测视频 成功");
}

void AutoCalibInDe1010::workMoveCheckAxis()
{
    if (m_errorCode != 0) return;

    writeLog("移动到光轴偏差位置100cm, 0度");
    if (mPlcSupported) {
        if(!plcRelative->plcMoveToAxis(POSITION_100CM_0)) {
            m_errorCode = -99;
            writeLog("移动到光轴偏差位置失败", -1);
            mBoolIgnoreUpload = true;
            return;
        }
    }
    logNormal("移动到光轴偏差位置 OK");
}

void AutoCalibInDe1010::workCheckAxisCalculate()
{
    if (m_errorCode != 0) return;

    writeLog("光轴偏差检测");
    tRowData(QStringList() << "光轴偏差" << "0" << QString::number(m_offsetAxisMaximum) << "-" << "TEST");
    emit tableUpdateTestStatus(2);

    IRCalibIntrinsic::Instance()->setTypeValue("XD01A");
    IRPoint points;

    {
        QByteArray imgData;
        for (int foo = 0; foo < 3; foo++)
        {
            QCoreApplication::processEvents();
            {
                imgData = mByteData;
                mByteData = QByteArray();
            }
            if (imgData.size() > 0)
                break;
            if (foo == 2) {
                m_errorCode = -1;
                mErrString = "获取图片失败";
                emit tableUpdateTestStatus(1);
                return;
            }
        }
        unsigned char* src_data = new unsigned char[640 * 512 * 2];
        memcpy(src_data, imgData.data() + 640 * 4, 640 * 512 * 2);

        memset (m_frameY, 0, 640 * 512);
        for(int i=0; i < 640 * 512; i++) {
            m_frameY[i] = src_data[2*i+1];
        }
    }
    int result = IRCalibIntrinsic::Instance()->getCenterPoint(m_frameY, points);
    if(result == 0) {
        int w = 640;
        int h = 512;
        float offsetX = points.x - (w/2 + m_fixEnvOffset.x());
        float offsetY = points.y - (h/2 + m_fixEnvOffset.y());
        mOffsetAxis =QPoint(offsetX, offsetY);

        writeLog("光轴偏移量，x=" + QString::number(offsetX, 'f', 2) + " y=" + QString::number(offsetY, 'f', 2));

        crossAxisDiffInit();

        int offsetValue = pow(offsetX,2) + pow(offsetY, 2);
        qInfo() << offsetValue << " compare " << pow(m_offsetAxisMaximum, 2);
        m_offsetValue = fabs(sqrt(offsetValue));
        emit tableUpdateData(QString::number(m_offsetValue, 'f', 2), 3);


        if(offsetValue < pow(m_offsetAxisMaximum, 2)) {
            writeLog("光轴偏差检测完成" );
            mesAddData(ItemRadus, QString::number(m_offsetValue, 'f', 2), "PASS");
            emit tableUpdateTestStatus(0);
        } else {
            m_errorCode = -101;
            writeLog("光轴偏差检测失败,期望值" + QString::number(m_offsetAxisMaximum) + ", 实际值" + QString::number(m_offsetValue), -1);
            mesAddData(ItemRadus, QString::number(m_offsetValue, 'f', 2), "NG");
            emit tableUpdateTestStatus(1);
            return;
        }
    } else {
        m_errorCode = -101;
        writeLog("光轴偏差检测失败！图像异常", -1);
        emit tableUpdateTestStatus(1);
    }
}


bool AutoCalibInDe1010::workCapture(int index)
{
    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(0, videoInfo);
    if(videoInfo.format == VIDEO_FORMAT_UYUV) {
        Yuv2Rgb::YuvToRgbUYVY(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    } else {
        Yuv2Rgb::YuvToRgbNV12(m_frameData, m_rgbData, videoInfo.width, videoInfo.height);
    }

    QImage image(m_rgbData, videoInfo.width, videoInfo.height, QImage::Format_RGB888);

    if (image.isNull()) {
        mErrString = "空的图片数据";
        return false;
    }

    QString path = mImageSavePath;
    qDebug() << path;

    QString filePath = QString("%1IRCalib_%2_%3.bmp").arg(path)
            .arg(m_workList[index].distance).arg(m_workList[index].direction);
    writeLog("保存图片：" + filePath);
    if (!image.save(filePath)) {
        mErrString = "图片保存失败：" + filePath;
        mBoolIgnoreUpload = true;
        return false;
    }
    if(!QFile::exists(filePath)) {
        mErrString = "查找保存的图片失败";
        mBoolIgnoreUpload = true;
        return false;
    }
    m_imagePathList.push_back(filePath.toStdString());

    return true;
}

void AutoCalibInDe1010::workCalculateParam()
{
    if (m_errorCode != 0) return;
    showProcess("计算内参值");
    IRCalibIntrinsic::Instance()->setTypeValue("XD01A");
    int iRet = IRCalibIntrinsic::Instance()->irCalibIntrinsicAuto(m_imagePathList, m_calibParam);
    if(iRet == 0) {//succ
        int iStart = 1;
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.fx);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.fy);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.cx);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.cy);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.distCoff[0]);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.distCoff[1]);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.distCoff[4]);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.distCoff[2]);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.distCoff[3]);
        g_mesCheckPackage[iStart++].sValue = QString::asprintf("%.6f", m_calibParam.rms);
        if (checkLimit()) {
            return;
        }
    }

    m_errorCode = -99;
    writeLog("计算内参值 失败: " + QString::number(iRet), -1);
}

void AutoCalibInDe1010::workHandleDown()
{
    if (!mPlcSupported) return;
    if (!mBoolFixtureHasRaised) return;
    writeLog("抬起待测夹具");
    bool bRet = plcRelative->plcMoveDown();
    if(!bRet) {
        writeLog("抬起待测夹具 失败", -1);
        mBoolIgnoreUpload = true;
    }else {
        writeLog("抬起待测夹具 OK" );
    }
}

void AutoCalibInDe1010::workMoveOrigin()
{
    if (!mPlcSupported) return;
    if (!mBoolFixtureHasRaised) return;
    showProcess("PLC回原点");
    bool bRet = plcRelative->plcMoveOrign();
    if(!bRet) {
        writeLog("PLC回原点 失败", -1);
        mBoolIgnoreUpload = true;
    }else {
        writeLog("PLC回原点 OK" );
    }
}

void AutoCalibInDe1010::reportResult()
{
    if (m_errorCode == 0) {
        writeLog("进行写内参操作");
        AsicControl asic(mBoxIP);
        bool ret = asic.SetParamIn(m_calibParam.fx, m_calibParam.fy, m_calibParam.cx, m_calibParam.cy, m_calibParam.rms, m_calibParam.distCoff);
        if (!ret) {
            m_errorCode = -105;
            logFail("设置内参 失败");
        }

        writeLog("进行比对内参操作");
        QString coff = asic.GetParamIn();
        logNormal("读取内参为: " + coff);

        if (m_productName == "XD01A") {
            writeLog("执行老化标记位清楚操作...");
            if (!asic.ClearAgeErrorCode()) {
                m_errorCode = -105;
                logFail("执行清老化标记位失败!!!");
            } else {
                writeLog("回读老化标记位清楚操作...");
                int ageErrorCode = 0;
                if (!asic.GetAgeErrorCode(ageErrorCode)) {
                    m_errorCode = -105;
                    logFail("回读老化标记位失败!!!");
                } else {
                    m_ageErrorCode = ageErrorCode;
                    writeLog("回读老化标记位：" + QString::number(ageErrorCode));
                    if (ageErrorCode != 0) {
                        m_errorCode = -105;
                        logFail("老化标记位清楚后依然包含错误码，失败!!!");
                    }
                }
            }
        } else {
            writeLog("非XD01A项目忽略执行老化标记位清楚操作，暂不支持...");
        }
    }

    mesUpload();
    if (isOnlineMode() && !mBoolIgnoreUpload) {
        QString out;
        logNormal("出站: " + m_sn);
        if (!mMesObj.outProduce((m_errorCode == 0) ? 0 : 1, out)) {
            writeLog("出站失败: " + out);
            m_errorCode = -99;
        }
    } else {
        logNormal("离线模式不执行出站操作");
    }

    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    QString logName = "";
    if(m_errorCode == 0) {
        logName = "PASS_" + dirName + QTime::currentTime().toString("hhmmss");
        writeLog("工作成功 SN: " + m_sn, 1);
    } else {
        logName = "FAIL_" + dirName + QTime::currentTime().toString("hhmmss");
        writeLog("FAIL: " + mErrString, -1);
    }

    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(end);
    struct tm* local = localtime(&t);
    QString endTimeStr;
    endTimeStr.sprintf("测试结束时间：%04d/%02d/%02d %02d:%02d:%02d",
                       local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                       local->tm_hour, local->tm_min, local->tm_sec);
    logNormal(endTimeStr);

    QString startTimeStr;
    t = std::chrono::system_clock::to_time_t(mStartPoint);
    local = localtime(&t);
    startTimeStr.sprintf("测试开始时间：%04d/%02d/%02d %02d:%02d:%02d",
                       local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                       local->tm_hour, local->tm_min, local->tm_sec);
    logNormal(startTimeStr);
    float cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - mStartPoint).count() / 1000;
    logNormal("测试耗时：" + QString::number(cost, 'f', 2) + "s");

    emit tableSaveCsv(mImageSavePath + logName + ".csv");
    emit logSaveTxt(mImageSavePath + logName + ".txt");

    CsvLogger csvlog;
    csvlog.setCsvLogPath(mMainLogPath + QDate::currentDate().toString("yyyyMMdd") + ".csv");
    QStringList title;
    title << "开始时间" << "结束时间" << "SN" << "测试模式" << "测试时间" << "测试结果" << "错误信息" << "offsetX" << "offsetY" << "光轴偏差"
          << "fx" << "fy" << "cx" << "cy" << "rms";
    csvlog.addCsvTitle(title);
    QStringList values;
    values << startTimeStr << endTimeStr << m_sn << testModeStr() << QString::number(cost, 'f', 2) << (m_errorCode == 0 ? "PASS" : "FAIL")
           << mErrString << QString::number(mOffsetAxis.x(), 'f', 2) << QString::number(mOffsetAxis.y(), 'f', 2)
           << QString::number(m_offsetValue, 'f', 2) << QString::number(m_calibParam.fx, 'f', 2)
           << QString::number(m_calibParam.fy, 'f', 2) << QString::number(m_calibParam.cx, 'f', 2)
           << QString::number(m_calibParam.cy, 'f', 2) << QString::number(m_calibParam.rms, 'f', 2);
    csvlog.addCsvLog(values);

    msgBox("请更换产品并按Enter按键，进行下一轮测试", 4);

    stopProcess();
}

void AutoCalibInDe1010::workMoveCapture()
{
    if (m_errorCode != 0) return;

    for(int i = 0; i < m_workList.size(); i ++) {
        WorkPosition position = m_workList[i];
        QString desp = "";
        writeLog("开始移动位置");

        unsigned short pos = getPos(position.distance, position.direction, desp);
        writeLog("移动到位置：" + desp);
        if (mPlcSupported) {
            if (!plcRelative->plcMoveToAxis(pos)) {
                m_errorCode = -99;
                writeLog("移动到位置：" + desp + "失败", -1);
                return;
            }
        }
        if (!workCapture(i)) {
            m_errorCode = -99;
            writeLog(mErrString, -1);
            return;
        }
    }

}

void AutoCalibInDe1010::recvFrame(char *pData, int iLen)
{
    m_frameCount = m_frameCount + 1;
    FrameInfo_S frameInfo;
    memcpy(&frameInfo, pData, sizeof(frameInfo));
//    qInfo() << frameInfo.uFormat << frameInfo.uHeadFlag << frameInfo.uNvsLen << frameInfo.uParamLen
//             << frameInfo.uTimeTm << frameInfo.uTotalLen << frameInfo.uY16Len << frameInfo.uYuvLen;

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

    {
        QMutexLocker lock(&m_mutex);
        memcpy(m_frameData, pData + info.nvsLen, info.frameLen);
        //memset (m_frameY, 0, 640 * 512);
        //for(int i=0; i < 640 * 512; i++) {
        //    m_frameY[i] = m_frameData[2*i+1];
        //}
    }
    VideoInfo::getInstance()->setVideoInfo(0, info);
    emit videoUpdate(0, QByteArray::fromRawData((const char*)m_frameData, info.frameLen), 0);
}

void AutoCalibInDe1010::getParameters()
{
    // log path 获取
    mLogPath = logPath() + "/";
    QDir dir(mLogPath);
    if (!dir.exists())
        dir.mkpath(mLogPath);

    mBoxIP = dutIP();
    ConfigInfo::getInstance()->getValueString("GLOBAL", "ReadCom", mPlcReadSerial);
    ConfigInfo::getInstance()->getValueString("GLOBAL", "WriteCom", mPlcWriteSerial);
}

bool AutoCalibInDe1010::getCurrentSN()
{
    emit tableAddRowData(QStringList() << "获取SN" << "-" << "-" << "-" << "TEST");
    emit tableUpdateTestStatus(2);
    SnGetMethod method = snGetMethod();
    QString sn;
    if (method == SnGetMothod_Production) {
        AsicControl asic(mBoxIP);
        sn = asic.GetSn();
        if (sn.size() == 0) {
            mErrString = "从产品读取SN失败";
            return false;
        }
    } else {
        sn = msgBox("请输入SN");
    }

    if (!isGoldenMode()) {
        int snlen = snGetLength();
        if (snlen > 0) {
            if (sn.length() != snlen) {
                mErrString = "SN比对失败，需求长度为" + QString::number(snlen);
                emit tableUpdateTestStatus(1);
                return false;
            }
            QString prefix = snGetPrefix();
            if (prefix.size() > 0) {
                if (sn.indexOf(prefix) != 0) {
                    mErrString = "SN前缀比对失败，需求前缀为" + prefix;
                    emit tableUpdateTestStatus(1);
                    return false;
                }
            }
        }
    }

    emit tableUpdateData(sn, 3);
    emit tableUpdateTestStatus(0);

    logNormal("读取到SN：" + sn);
    setCurrentBarcode(sn);
    return true;
}

bool AutoCalibInDe1010::dutPing()
{
    int iContinue = 0;
    for (int foo = 0; foo < 20; foo++) {
        QCoreApplication::processEvents();
        if (CPingTest::pingTest(nullptr, mBoxIP.toLocal8Bit().data())) {
            logNormal("ping ip " + mBoxIP + " ok!");
            iContinue++;
            if (iContinue > 3) {
                return true;
            }
        } else {
            logFail("ping ip " + mBoxIP + " failed!");
            iContinue = 0;
        }
    }

    mErrString = "ping 待测IP失败, 检查工装盒是否有上电";
    return false;
}

bool AutoCalibInDe1010::dutSwitchPattle(int mode)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mBoxIP);
        if (asic.SetPattle(mode)) {
            return true;
        }
        QThread::msleep(100);
    }
    return false;
}

void AutoCalibInDe1010::writeLog(QString log, int status)
{
    switch(status) {
    case 0:
        logNormal(log);
        showProcess(log);
        break;
    case 1:
        logPass(log);
        showPass();
        break;
    case -1:
        logFail(log);
        showFail(log);
        mErrString = log;
        break;
    case 2:
        logWarming(log);
        showProcess(log);
        break;
    }
}

void AutoCalibInDe1010::mesClear()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
        if (!g_mesCheckPackage[foo].sItem.isEmpty()) {
            g_mesCheckPackage[foo].sResult = "NG";
        }
    }
}

void AutoCalibInDe1010::mesAddData(int item, QString value, QString rst, QString err)
{
    g_mesCheckPackage[item].sValue = value;
    g_mesCheckPackage[item].sResult = rst;
    g_mesCheckPackage[item].sNote = err;
}

void AutoCalibInDe1010::mesUpload()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
        if (!g_mesCheckPackage[foo].sItem.isEmpty()) {
            mMesObj.addItemResultEx(g_mesCheckPackage[foo]);
        }
    }
}

bool AutoCalibInDe1010::checkLimit()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
        QCoreApplication::processEvents();
        if (!g_mesCheckPackage[foo].sItem.isEmpty()) {
            MesCheckItem& item= g_mesCheckPackage[foo];
            QStringList list;
            list << item.sItem << item.sMin << item.sMax << item.sValue << "TEST";
            emit tableAddRowData(list);
            emit tableUpdateTestStatus(2);
            writeLog("内参值合法值比较：" + list.join(","));
            float min = item.sMin.toFloat();
            float max = item.sMax.toFloat();
            float val = item.sValue.toFloat();
            if (val >= min && val <= max) {
                item.sResult = "PASS";
                writeLog("内参值合法值比较：PASS");
                emit tableUpdateTestStatus(0);
            } else {
                emit tableUpdateTestStatus(1);
                writeLog(QString("内参值%1为%2, 超出%3~%4范围").arg(item.sItem).arg(val).arg(min).arg(max));
                return false;
            }
        }
    }

    return true;
}
unsigned short AutoCalibInDe1010::getPos(int distance, int direction, QString& posDesp)
{
    unsigned short send = -999;
    QString posDesc;
    switch(direction)
    {
    case PLC_AXIS_DIRECITON_30N:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = POSITION_45CM_30N;
            posDesc = "45CM_30N";
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = POSITION_50CM_30N;
            posDesc = "50CM_30N";
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = POSITION_60CM_30N;
            posDesc = "60CM_30N";
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = POSITION_70CM_30N;
            posDesc = "70CM_30N";
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = POSITION_80CM_30N;
            posDesc = "80CM_30N";
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = POSITION_90CM_30N;
            posDesc = "90CM_30N";
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = POSITION_100CM_30N;
            posDesc = "100CM_30N";
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_15N:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = POSITION_45CM_15N;
            posDesc = "45CM_15N";
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = POSITION_50CM_15N;
            posDesc = "50CM_15N";
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = POSITION_60CM_15N;
            posDesc = "60CM_15N";
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = POSITION_70CM_15N;
            posDesc = "70CM_15N";
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = POSITION_80CM_15N;
            posDesc = "80CM_15N";
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = POSITION_90CM_15N;
            posDesc = "90CM_15N";
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = POSITION_100CM_15N;
            posDesc = "100CM_15N";
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_0:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = POSITION_45CM_0;
            posDesc = "45CM_0";
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = POSITION_50CM_0;
            posDesc = "50CM_0";
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = POSITION_60CM_0;
            posDesc = "60CM_0";
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = POSITION_70CM_0;
            posDesc = "70CM_0";
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = POSITION_80CM_0;
            posDesc = "80CM_0";
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = POSITION_90CM_0;
            posDesc = "90CM_0";
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = POSITION_100CM_0;
            posDesc = "100CM_0";
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_15P:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = POSITION_45CM_15P;
            posDesc = "45CM_15P";
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = POSITION_50CM_15P;
            posDesc = "50CM_15P";
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = POSITION_60CM_15P;
            posDesc = "60CM_15P";
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = POSITION_70CM_15P;
            posDesc = "70CM_15P";
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = POSITION_80CM_15P;
            posDesc = "80CM_15P";
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = POSITION_90CM_15P;
            posDesc = "90CM_15P";
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = POSITION_100CM_15P;
            posDesc = "100CM_15P";
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_30P:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = POSITION_45CM_30P;
            posDesc = "45CM_30P";
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = POSITION_50CM_30P;
            posDesc = "50CM_30P";
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = POSITION_60CM_30P;
            posDesc = "60CM_30P";
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = POSITION_70CM_30P;
            posDesc = "70CM_30P";
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = POSITION_80CM_30P;
            posDesc = "80CM_30P";
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = POSITION_90CM_30P;
            posDesc = "90CM_30P";
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = POSITION_100CM_30P;
            posDesc = "100CM_30P";
            break;
        }
        break;
    }
    default:
        posDesc = "None";
        send = -999;
        break;
    }

    posDesp = posDesc;
    return send;
}



void AutoCalibInDe1010::crossAxisInit(int diameter)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);

    QImage img(diameter, diameter, QImage::Format_ARGB32);
    img.fill(qRgba(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&img);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::red);
    painter.setPen(pen);
    QPoint center(diameter / 2, diameter / 2);

    // 中心周围画十字架
    int crossR = (int)diameter / 2 * 0.8 ;
    painter.drawLine(center + QPoint(-1 * crossR , 0), center + QPoint(crossR, 0));
    painter.drawLine(center + QPoint(0, -1 * crossR), center + QPoint(0, crossR));

    // 中心周围画圆
    int cycleR = (int)(diameter - 4 ) / 2;
    painter.drawEllipse(QRect(center + QPoint(-1 * cycleR, -1 * cycleR), center + QPoint(cycleR, cycleR)));
    painter.end();

    // 水印从左上角画，mCenterPoint为左上角坐标
    mCenterPoint.setX(info.width / 2 - center.x() + m_fixEnvOffset.x());
    mCenterPoint.setY(info.height / 2 - center.y() + m_fixEnvOffset.y());
    emit waterPrintAddin(0, img, mCenterPoint.x(), mCenterPoint.y());
    emit waterPrintVisible(0, true);
}

void AutoCalibInDe1010::crossAxisDiffInit(int diameter)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);

    QImage img(diameter, diameter, QImage::Format_ARGB32);
    img.fill(qRgba(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&img);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::green);
    painter.setPen(pen);
    QPoint center(diameter / 2, diameter / 2);
    painter.drawLine(center + QPoint(-15, 0), center + QPoint(15, 0));
    painter.drawLine(center + QPoint(0, -15), center + QPoint(0, 15));
    painter.end();

    mCenterPoint.setX(info.width / 2 - center.x() + m_fixEnvOffset.x() + mOffsetAxis.x());
    mCenterPoint.setY(info.height / 2 - center.y() + m_fixEnvOffset.y() + mOffsetAxis.y());
    emit waterPrintAddin(1, img, mCenterPoint.x(), mCenterPoint.y());
    emit waterPrintVisible(1, true);
}

void AutoCalibInDe1010::getSn()
{
    if(!m_lstRes) {
        msgBox("NG 请确认进行下一项", 1);
    }
    tableClearData();
    logClear();
    showProcess("获取SN");
    QString sIp = "192.168.1.10";
    sIp = ConfigInfo::getInstance()->sysInfo().ip;
    AsicControl control(sIp);
    m_sn = control.GetSn();
    if(m_sn.length() <= 0) {
        m_errorCode = -106;
        stopProcess();
    }else {
        logNormal("获取sn" + m_sn);
    }
    showPackageTable(&g_mesCheckPackage[0], m_sn);
}

void AutoCalibInDe1010::slotsStoped()
{
    QString dirName = m_sn;
    dirName = dirName.replace("*", "_");
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath");
    logPath += "/" + dirName + ".csv";
    tableSaveCsv(logPath);

    m_videoClient->stopConnect();
    m_imagePathList.clear();
    m_lstRes = (m_errorCode == 0);
    emit videoClose();
//    showMsgBox("点击确认进行下一项目测试", 1);
    startProcess();
}

void AutoCalibInDe1010::slotConnectBtnClicked()
{
    tHeader(QStringList() << "项目" << "最小值" << "最大值" << "当前值" << "结果");
    tClear();
    logClear();

    getParameters();

    mesClear();

    writeLog("初始化串口");

    if (!plcInit()) {
        logFail(mErrString);
        showFail(mErrString);
        return;
    }


    BaseProduce::slotConnectBtnClicked();
    //writeLog("初始化OK");
    //showPass();
    //showProcess("初始化完成，点击开始测试");
    //updateStatus(1);
}

void AutoCalibInDe1010::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
}

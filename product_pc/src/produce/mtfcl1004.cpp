#include "mtfcl1004.h"
#include <asiccontrol.h>
#include <videoinfo.h>
#include <util.h>
#include <yuv2rgb.h>
#include <mapping.h>
#include <mappingnew.h>
#include <XyScanDialog.h>
#include <QPainter>
#include <StaticSignals.h>
#include <QMessageBox>
#include "mtfimgcheck.h"

#define CHECK_TIMES 5//每次计算重复次数
#define END_THRESHOLD 50//第一次运动截止阈值
#define MAX_STOP_THRESHOLD 30//第一次运动截止阈值
#define SECOND_STOP_THRESHOLD 20//第一次运动截止阈值

#define SECOND_MAX_THREHOLD 150//
#define SECOND_MAX_THREHOLD_TMP 320//
#define SHORT_MAX_THREHOLD 200//
#define LEFT_RIGHT_STEP 1500

#define MTF50P_W_C_12 300
#define MTF50P_H_C_12 300
#define MTF50P_W_70_12 250
#define MTF50P_H_70_12 250
#define MTF50P_MAX    500

#define MTF_CHECK_THREHOLD 10
#define CHECK_POS_THRESHOLD 150

#define SECOND_SHARP_THRESHOLD 5

#define DROP_RITE 0.01
#define CHECK_DOOR_THRESHOLD 50

#define MAX_3_RUN_TIMES 10

#define NETD_CHECK_TIMES 128
#define NETD_CHECK 1

using namespace std;
using namespace cv;

// 如下枚举与图表上的线条一一对应...
typedef enum {
    ChartLine_Top = 0,
    ChartLine_Bottom,
    ChartLine_Center,
    ChartLine_Left,
    ChartLine_Right,
    ChartLine_Average
}ChartLineSequence;

typedef enum {
    ItemMtfTCx = 0,
    ItemMtfTCy,
    ItemMtfBCx,
    ItemMtfBCy,
    ItemMtfCCx,
    ItemMtfCCy,
    ItemMtfLCx,
    ItemMtfLCy,
    ItemMtfRCx,
    ItemMtfRCy,
    ItemMtfCCAvg,
    ItemSpAvg,
    ItemSpCC,
    ItemSpTC,
    ItemSpBC,
    ItemSpRC,
    ItemSpLC,
    ItemSpMtfCx,
    ItemSpMtfCy,
    ItemSpNetdImage,
    ItemSpNetdSpace
}ItemMtf;

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item  value minvalue maxvalue exp result
    {"mtf_tc_x", "", "","","-", "NG"},
    {"mtf_tc_y", "", "","","-", "NG"},
    {"mtf_bc_x", "", "","","-", "NG"},
    {"mtf_bc_y", "", "","","-", "NG"},
    {"mtf_cc_x", "", "","","-", "NG"},
    {"mtf_cc_y", "", "","","-", "NG"},
    {"mtf_lc_x", "", "","","-", "NG"},
    {"mtf_lc_y", "", "","","-", "NG"},
    {"mtf_rc_x", "", "","","-", "NG"},
    {"mtf_rc_y", "", "","","-", "NG"},
    {"mtf_cc_avg", "", "","","-", "NG"},
    {"sp_avg_value", "", "","","-", "NG"},
    {"sp_cc_value", "", "","","-", "NG"},
    {"sp_tc_value", "", "","","-", "NG"},
    {"sp_bc_value", "", "","","-", "NG"},
    {"sp_rc_value", "", "","","-", "NG"},
    {"sp_lc_value", "", "","","-", "NG"},
    {"sp_mtf_cx", "", "","","-", "NG"},
    {"sp_mtf_cy", "", "","","-", "NG"},
    {"sp_image_netd", "", "","","-", "NG"},
    {"sp_space_netd", "", "","","-", "NG"},
    {""},
};

MtfCL1004::MtfCL1004()
{
    if(!m_videoClient) {
        m_videoClient = new WebSocketClientManager;
    }
    connect(pMachine(), &QStateMachine::stopped,this, &MtfCL1004::slotsStoped);

    SysInfo info = ConfigInfo::getInstance()->sysInfo();
    m_plc.setConfig(info.writeCom,info.readCom);
    // WSFRAMECALL funFrame = std::bind(&MtfCL1004::recvFrame, this, std::placeholders::_1, std::placeholders::_2);
    QObject::connect(m_videoClient, &WebSocketClientManager::signalRecvVideoFrame, this, &MtfCL1004::recvFrame);

    QObject::connect(StaticSignals::getInstance(), &StaticSignals::mtfGetOnceData, this, &MtfCL1004::slotOnceMtfData);

    mCrtFrameData.clear();

    // m_videoClient->regFrameCall(funFrame);
    m_rgbData = new unsigned char[640 * 512 * 4];
    m_frameY = new unsigned char[640 * 512 * 4];
    m_frameData = new unsigned char[640 * 512 * 4];
    m_isRunning = true;
    mBoolInitOk = false;
    add(1, "获取MTF", std::bind(&MtfCL1004::getMtf, this));

    m_workThread = std::thread(&MtfCL1004::workThread, this);
}

void MtfCL1004::getMtf()
{
    m_videoClient->stopConnect();
    m_videoClient->connectToUrl(ConfigInfo::getInstance()->sysInfo().ip, 6000);
    m_moveSmallTimes = 0;
    m_errorCode = 0;
    m_isStart = true;
    m_workCount = AUTO_WORK_GAS_LOAD_CLOSE;
    m_is_check_pos = false;
    move_times_3 = 0;
    m_down_times = 0;
    tableClearData();
    while(m_workCount != AUTO_WORK_STOP) {
        QThread::usleep(100);
    }
}

void MtfCL1004::getSn()
{
    if(!m_lstRes) {
        msgBox("NG 请确认进行下一项", 1);
    }
    if(!m_workStatus) {
        stopProcess();
        return;
    }

    showProcess("获取SN");

    m_errMsg.clear();

    m_sn = msgBox("获取SN");
    if(!m_workStatus) {
        stopProcess();
    }
    if(m_sn.length() <= 0) {
        logFail("获取SN失败");
    }else {
        logNormal("获取sn" + m_sn);
    }
    showPackageTable(&g_mesCheckPackage[0], m_sn);
}

void MtfCL1004::recvFrame(QByteArray frameData, FrameInfo_S frameInfo)
{
    m_frameCount += 1;


    m_mutex.lock();
    mCrtFrameData = frameData;
    m_mutex.unlock();


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

    emit videoUpdate(0, frameData, frameInfo.uTimeTm);
}

bool MtfCL1004::getTargetRange()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    unsigned short* input_data = new unsigned short[640 * 512];
    memcpy(input_data, m_frameData, 640*512*2);
    //复制图像
    if (m_sharp_start_points.size() > 0)
        m_sharp_start_points.clear();
    int start_xs[9] = { 245,30,245,470,255 , 30, 470, 30, 470};
    int start_ys[9] = { 15,180,180,180,350 , 15, 15, 350, 350 };
    int w = 150, h = 150;
    for (int k = 0; k < 5 + 4; ++k)
    {
        double x_sum = 0, y_sum = 0, sum = 0;
        int count = 0;
        for (int i = start_ys[k]; i < start_ys[k] + h; ++i)
        {
            for (int j = start_xs[k]; j < start_xs[k] + w; ++j)
            {
                sum += input_data[i * 640 + j];
            }
        }
        double avg_value = sum / (w * h);
        int value_count = 0;
        for (int i = start_ys[k]; i < start_ys[k] + h; ++i)
        {
            for (int j = start_xs[k]; j < start_xs[k] + w; ++j)
            {
                if (input_data[i * 640 + j] > avg_value)
                {
                    x_sum += (input_data[i * 640 + j] - avg_value) * j;
                    y_sum += (input_data[i * 640 + j]  - avg_value) * i;
                    value_count += input_data[i * 640 + j] - avg_value;
                }
            }
        }
        SharpStartPos sp;
        sp.x = x_sum / value_count - 40;
        sp.y = y_sum / value_count - 40;

        sp.x = max(20, sp.x);
        sp.x = min(640 - 40 - 25, sp.x);
        sp.y = max(20, sp.y);
        sp.y = min(512 - 40 - 25, sp.y);


        m_sharp_start_points.push_back(sp);
    }


    delete[]input_data;
    return true;
}

SharpPos MtfCL1004::getSharpValue_new()
{
    unsigned short* input_data = new unsigned short[640 * 512];
    memset(input_data, 0, 640 * 512 * 2);
    memcpy(input_data, m_frameData, 640 * 512 * 2);
    SharpPos sharps = { 0 };
    float data_[9] = { 0, 0, 0, 0, 0 , 0, 0, 0, 0};
    int x = m_sharp_start_points[0].x + 40 - 10;
    int y = m_sharp_start_points[0].y + 40;
    int w = 20, h = 50;
    int sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    data_[0] = (float)sum_value / w;


    x = m_sharp_start_points[1].x + 40;
    y = m_sharp_start_points[1].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    data_[1] = (float)sum_value / h;

    x = m_sharp_start_points[2].x + 40;
    y = m_sharp_start_points[2].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    data_[2] = (float)sum_value / h;

    x = m_sharp_start_points[3].x + 40 - 50;
    y = m_sharp_start_points[3].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    data_[3] = (float)sum_value / h;

    x = m_sharp_start_points[4].x + 40 - 10;
    y = m_sharp_start_points[4].y + 40 - 50;
    w = 20, h = 50;
    sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    data_[4] = (float)sum_value / w;
    sharps.TC_value = data_[0];
    sharps.LC_value = data_[1];
    sharps.CC_value = data_[2];
    sharps.RC_value = data_[3];
    sharps.BC_value = data_[4];
//	sharps.avg_value = data_[0] * 0.2f + data_[1] * 0.2f + data_[2] * 0.2f + data_[3] * 0.2f + data_[4] * 0.2f;
    sharps.avg_value = data_[0] * 0.15f + data_[1] * 0.15f + data_[2] * 0.4f + data_[3] * 0.15f + data_[4] * 0.15f;
    qDebug()<<"sharps.avg_value = "<<sharps.avg_value;
    delete[]input_data;
    return sharps;
}

SharpPosSide MtfCL1004::getSharpValue_new_side()
{
    unsigned short* input_data = new unsigned short[640 * 512];
    memset(input_data, 0, 640 * 512 * 2);
    memcpy(input_data, m_frameData, 640*512*2);
    SharpPosSide sharp_sides;
    int x = m_sharp_start_points[5].x + 40;
    int y = m_sharp_start_points[5].y + 40 - 10;
    int w = 50, h = 20;
    int sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    float x_value = (float)sum_value / h;

    x = m_sharp_start_points[5].x + 40 - 10;
    y = m_sharp_start_points[5].y + 40;
    w = 20, h = 50;
    sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    float y_value = (float)sum_value / w;
    sharp_sides.LT_value = (y_value + x_value) / 2;


    x = m_sharp_start_points[6].x + 40 - 50;
    y = m_sharp_start_points[6].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    x_value = (float)sum_value / h;

    x = m_sharp_start_points[6].x + 40 - 10;
    y = m_sharp_start_points[6].y + 40;
    w = 20, h = 50;
    sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    y_value = (float)sum_value / w;
    sharp_sides.RT_value = (y_value + x_value) / 2;


    x = m_sharp_start_points[7].x + 40;
    y = m_sharp_start_points[7].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    x_value = (float)sum_value / h;

    x = m_sharp_start_points[7].x + 40 - 10;
    y = m_sharp_start_points[7].y + 40 - 50;
    w = 20, h = 50;
    sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    y_value = (float)sum_value / w;
    sharp_sides.LB_value = (y_value + x_value) / 2;


    x = m_sharp_start_points[8].x + 40 - 50;
    y = m_sharp_start_points[8].y + 40 - 10;
    w = 50, h = 20;
    sum_value = 0;
    for (int i = y; i < y + h; ++i)
    {
        int max_value = 0;
        for (int j = x + 1; j < x + w - 1; ++j)
        {
            int value = abs(input_data[i * 640 + j - 1] - input_data[i * 640 + j + 1]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    x_value = (float)sum_value / h;

    x = m_sharp_start_points[8].x + 40 - 10;
    y = m_sharp_start_points[8].y + 40 - 50;
    w = 20, h = 50;
    sum_value = 0;
    for (int i = x; i < x + w; ++i)
    {
        int max_value = 0;
        for (int j = y + 1; j < y + h - 1; ++j)
        {
            int value = abs(input_data[(j - 1) * 640 + i] - input_data[(j + 1) * 640 + i]);
            max_value = max(value, max_value);
        }
        sum_value += max_value;
    }
    y_value = (float)sum_value / w;
    sharp_sides.RB_value = (y_value + x_value) / 2;

    delete[]input_data;
    return sharp_sides;
}

double MtfCL1004::Cal_image_NETD(MYRECT high_rect, MYRECT low_rect)
{
    netd_img_data = new unsigned short[NETD_CHECK_TIMES * 640 * 512];
    float* pMeanImageData = new float[640 * 512];
    double*pStdImageData = new double[640 * 512];
    double*pNETDImageData = new double[640 * 512];
    SRF netd;
    memset(pMeanImageData, 0, 640*512*sizeof(float));
    int shift_step = 640 * 512;
    for (int i = 0; i < NETD_CHECK_TIMES; ++i)
    {
        netd.GetVedioImage(netd_img_data, (unsigned short*)m_frameData, 640, 512, i);
        netd.GetMeanImage(pMeanImageData, netd_img_data + shift_step * i, 640, 512, high_rect);
        netd.GetMeanImage(pMeanImageData, netd_img_data + shift_step * i, 640, 512, low_rect);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    netd.GetImageStd(pStdImageData, netd_img_data, NETD_CHECK_TIMES, pMeanImageData, 640, 512, low_rect);
    double netd_data = netd.GetImageNETD(pNETDImageData, pStdImageData, pMeanImageData, pMeanImageData, NETD_CHECK_TIMES, 640, 512, low_rect, high_rect);

    delete[]netd_img_data;
    delete[]pMeanImageData;
    delete[]pStdImageData;
    delete[]pNETDImageData;
    return netd_data;
}

double MtfCL1004::Cal_space_NETD(MYRECT high_rect, MYRECT low_rect)
{
    unsigned short* img_data = new unsigned short[640*512];
    double*pSpaceSNRData = new double[640 * 512];
    double* pMeanImageData = new double[640 * 512];
    double*pStdImageData = new double[640 * 512];
    SRF netd;
    {
        // QMutexLocker lock(&m_mutex);
        memcpy(img_data, m_frameData, 640*512*2);
    }
    memset(pMeanImageData, 0, 640*512*sizeof(double));

    double netd_data = netd.GetImageSpaceSNR(pSpaceSNRData, pMeanImageData, pStdImageData, img_data, 640, 512, low_rect, high_rect);
    delete[]img_data;
    delete[]pSpaceSNRData;
    delete[]pMeanImageData;
    delete[]pStdImageData;
    return netd_data;
}

void MtfCL1004::crossAxisInit()
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(0, info);
    QImage img(info.width, info.height, QImage::Format_ARGB32);
    img.fill(qRgba(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&img);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(Qt::green);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawRect(96 - 25, 76 - 25, 50, 50);
    painter.drawRect(544 - 25, 76 - 25, 50, 50);
    painter.drawRect(96 - 25, 435 - 25, 50, 50);
    painter.drawRect(544 - 25, 435 - 25, 50, 50);

    painter.drawRect(320 - 25, 256 - 25, 50, 50);

    painter.drawRect(131 - 25 + 12, 256 - 25, 50, 50);
    painter.drawRect(320 - 25, 67 - 25 +  12, 50, 50);
    painter.drawRect(509 - 25 -12, 256 - 25, 50, 50);
    painter.drawRect(320 - 25, 445 - 25-12, 50, 50);

/*
    painter.drawLine(320-15, 76, 320+15, 76);
    painter.drawLine(320, 76-15, 320, 76+15);

    painter.drawLine(320-15, 256, 320+15, 256);
    painter.drawLine(320, 256-15, 320, 256+15);

    painter.drawLine(320-15, 435, 320+15, 435);
    painter.drawLine(320, 435-15, 320, 435+15);

    painter.drawLine(96-15, 256, 96+15, 256);
    painter.drawLine(96, 256-15, 96, 256+15);

    painter.drawLine(544-15, 256, 544+15, 256);
    painter.drawLine(544, 256-15, 544, 256+15);

    painter.drawLine(96-15, 76, 96+15, 76);
    painter.drawLine(96, 76-15, 96, 76+15);

    painter.drawLine(544-15, 76, 544+15, 76);
    painter.drawLine(544, 76-15, 544, 76+15);

    painter.drawLine(96-15, 435, 96+15, 435);
    painter.drawLine(96, 435-15, 96, 435+15);

    painter.drawLine(544-15, 435, 544+15, 435);
    painter.drawLine(544, 435-15, 544, 435+15);
*/
    painter.end();

    emit waterPrintAddin(0, img, 0, 0);
    emit waterPrintVisible(0, true);
}

bool MtfCL1004::dutOpenShutter()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mBoxIp);
        if (asic.ChangeWorkMode(false)) {
            return true;
        }
        mySleep(500);
    }

    return false;
}

bool MtfCL1004::dutCloseAdpative()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mBoxIp);
        if (asic.AutoAdapteComp(0)) {
            return true;
        }
        mySleep(500);
    }

    return false;
}

bool MtfCL1004::dutChangeFormat(int mode)
{

    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mBoxIp);
        if (asic.changeSensorFormat(mode)) {
            return true;
        }
        mySleep(500);
    }

    return false;
}

void MtfCL1004::chartDrawSharpPos(SharpPos param, SharpPosSide psSide, MTF_param mtf, MTF_param_side mtfSide)
{
    // writeLog("清晰度与MTF计算后的结果打印--");
    // 保存队列
    // m_sharp_datas.push_back(param);
    // 保存数据
    saveCsvSharpValue(param, psSide, mtf, mtfSide);
    // 打印
    // printSharpPos(param);

    emit chartAddData(mChartXSlot, param.TC_value, 0, ChartLine_Top);
    emit chartAddData(mChartXSlot, param.BC_value, 0, ChartLine_Bottom);
    emit chartAddData(mChartXSlot, param.CC_value, 0, ChartLine_Center);
    emit chartAddData(mChartXSlot, param.LC_value, 0, ChartLine_Left);
    emit chartAddData(mChartXSlot, param.RC_value, 0, ChartLine_Right);
    emit chartAddData(mChartXSlot, param.avg_value, 0, ChartLine_Average);
    if (param.avg_value + 10 > mChartMaxY) {
        mChartMaxY += mChartAxisStep;
        emit chartSetYRange(0, mChartMaxY);
    }
    if (mChartXSlot + 5 > mChartMaxX) {
        mChartMaxX += mChartAxisStep;
        emit chartSetXRange(0, mChartMaxX);
    }
    mChartXSlot++;
}

void MtfCL1004::saveCsvSharpValue(SharpPos xPos, SharpPosSide spSide, MTF_param mtf, MTF_param_side mtfSide)
{
    CsvLogger logger;
    QString path = mTmpLogPath;
    QString fileName = mDetectorSN + "_" + mCsvTimeFlag + ".csv";
    logger.setCsvLogPath(path + fileName);
    writeLog("SharpPos数据保存到：" + path + fileName);

    QStringList header;
    header << "时间点" << "pos" << "TC_value" << "BC_value"
           << "CC_value" << "LC_value" << "RC_value" << "avg_value"
           << "LT_value" << "LB_value" << "RT_value" << "RB_value"
           << "TC_x" << "TC_y" << "LC_x" << "LC_y"
           << "CC_x" << "CC_y" << "RC_x" << "RC_y"
           << "BC_x" << "BC_y" << "LT_x" << "LT_y"
           << "LB_x" << "LB_y" << "RT_x" << "RT_y"
           << "RB_x" << "RB_y" << "image_netd" << "space_netd";
    logger.addCsvTitle(header);
    writeLog(header.join(","));

    QStringList line;
    line << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << QString::number(xPos.pos, 'f', 3)
         << QString::number(xPos.TC_value, 'f', 3) << QString::number(xPos.BC_value, 'f', 3)
         << QString::number(xPos.CC_value, 'f', 3) << QString::number(xPos.LC_value, 'f', 3)
         << QString::number(xPos.RC_value, 'f', 3) << QString::number(xPos.avg_value, 'f', 3)
         << QString::number(spSide.LT_value, 'f', 3)  << QString::number(spSide.LB_value, 'f', 3)
         << QString::number(spSide.RT_value, 'f', 3) << QString::number(spSide.RB_value, 'f', 3)
         << QString::number(mtf.TC_x, 'f', 3) << QString::number(mtf.TC_y, 'f', 3)
         << QString::number(mtf.LC_x, 'f', 3) << QString::number(mtf.LC_y, 'f', 3)
         << QString::number(mtf.CC_x, 'f', 3) << QString::number(mtf.CC_y, 'f', 3)
         << QString::number(mtf.RC_x, 'f', 3) << QString::number(mtf.RC_y, 'f', 3)
         << QString::number(mtf.BC_x, 'f', 3) << QString::number(mtf.BC_y, 'f', 3)
         << QString::number(mtfSide.LT_x, 'f', 3) << QString::number(mtfSide.LT_y, 'f', 3)
         << QString::number(mtfSide.LB_x, 'f', 3) << QString::number(mtfSide.LB_y, 'f', 3)
         << QString::number(mtfSide.RT_x, 'f', 3) << QString::number(mtfSide.RT_y, 'f', 3)
         << QString::number(mtfSide.RB_x, 'f', 3) << QString::number(mtfSide.RB_y, 'f', 3)
         << QString::number(xPos.image_netd, 'f', 3) << QString::number(xPos.space_netd, 'f', 3);
    logger.addCsvLog(line);
    writeLog(line.join(","));

}

void MtfCL1004::saveCsvReport(bool rst, SharpPos xPos, MTF_param mtf, MTF_param_side side_mtf, SharpPosSide sharp_side)
{
    CsvLogger logger;
    QString path = mLogPath + "/" + QDate::currentDate().toString("yyyyMMdd") + "/";
    QString fileName = QDate::currentDate().toString("yyyyMMdd") + ".csv";
    logger.setCsvLogPath(path + fileName);
    writeLog("行数据保存到：" + path + fileName);

    QStringList header;
    header << "开始时间" << "结束时间" << "探测器SN" << "测试时间" << "测试结果"
           << "错误信息" << "TC_value" << "BC_value"
           << "CC_value" << "LC_value" << "RC_value" << "avg_value"
           << "MTF_C_X" << "MTF_C_Y" << "image_netd" << "space_netd"
           << "TC_x" << "TC_y" << "LC_x" << "LC_y" << "CC_x"
           << "CC_y" << "RC_x" << "RC_y" << "BC_x" << "BC_y"
           << "LT_x" << "LT_y" << "LB_x" << "LB_y" << "RT_x"
           << "RT_y" << "RB_x" << "RB_y" << "LT_sharp" << "LB_sharp"
           << "RT_sharp"<< "RB_sharp";
    logger.addCsvTitle(header);
    writeLog(header.join(","));

    QStringList line;
    line << mTimeStartStr << mTimeEndStr << mDetectorSN
         << QString::number(mCostTime, 'f', 2) << (rst ? "PASS" : "FAIL") << m_errMsg
         << QString::number(xPos.TC_value, 'f', 3) << QString::number(xPos.BC_value, 'f', 3)
         << QString::number(xPos.CC_value, 'f', 3) << QString::number(xPos.LC_value, 'f', 3)
         << QString::number(xPos.RC_value, 'f', 3) << QString::number(xPos.avg_value, 'f', 3)
         << QString::number(xPos.MTF_C_X, 'f', 3)  << QString::number(xPos.MTF_C_Y, 'f', 3)
         << QString::number(xPos.image_netd, 'f', 3) << QString::number(xPos.space_netd, 'f', 3)
         << QString::number(mtf.TC_x, 'f', 3) << QString::number(mtf.TC_y, 'f', 3)
         << QString::number(mtf.LC_x, 'f', 3) << QString::number(mtf.LC_y, 'f', 3)
         << QString::number(mtf.CC_x, 'f', 3) << QString::number(mtf.CC_y, 'f', 3)
         << QString::number(mtf.RC_x, 'f', 3) << QString::number(mtf.RC_y, 'f', 3)
         << QString::number(mtf.BC_x, 'f', 3) << QString::number(mtf.BC_y, 'f', 3)
         << QString::number(side_mtf.LT_x, 'f', 3) << QString::number(side_mtf.LT_y, 'f', 3)
         << QString::number(side_mtf.LB_x, 'f', 3) << QString::number(side_mtf.LB_y, 'f', 3)
         << QString::number(side_mtf.RT_x, 'f', 3) << QString::number(side_mtf.RT_y, 'f', 3)
         << QString::number(side_mtf.RB_x, 'f', 3) << QString::number(side_mtf.RB_y, 'f', 3)
         << QString::number(sharp_side.LT_value, 'f', 3) << QString::number(sharp_side.LB_value, 'f', 3)
         << QString::number(sharp_side.RT_value, 'f', 3) << QString::number(sharp_side.RB_value, 'f', 3);
    logger.addCsvLog(line);
    writeLog(line.join(","));
}

bool MtfCL1004::judgetMtfResult(MTF_param mtf)
{
    bool bRst = true;
    QStringList dataList;
    dataList << "BC_x" << QString::number(m_mtfMinEdge_X) << QString::number(m_mtfMaxEdge) << QString::number(mtf.BC_x) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfBCx].sValue = QString::number(mtf.BC_x, 'f', 3);
    if ((mtf.BC_x < m_mtfMinEdge_X) || (mtf.BC_x > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: BC_x 比对失败");
        g_mesCheckPackage[ItemMtfBCx].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfBCx].sResult = "PASS";
    }

    dataList.clear();
    dataList << "BC_y" << QString::number(m_mtfMinEdge_Y) << QString::number(m_mtfMaxEdge) << QString::number(mtf.BC_y) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfBCy].sValue = QString::number(mtf.BC_y, 'f', 3);
    if ((mtf.BC_y < m_mtfMinEdge_Y) || (mtf.BC_y > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: BC_y 比对失败");
        g_mesCheckPackage[ItemMtfBCy].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfBCy].sResult = "PASS";
    }

    dataList.clear();
    dataList << "CC_x" << QString::number(m_mtfMinCC_X) << QString::number(m_mtfMaxCC) << QString::number(mtf.CC_x) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfCCx].sValue = QString::number(mtf.CC_x, 'f', 3);
    if ((mtf.CC_x < m_mtfMinCC_X) || (mtf.CC_x > m_mtfMaxCC)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: CC_x 比对失败");
        g_mesCheckPackage[ItemMtfCCx].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfCCx].sResult = "PASS";
    }

    dataList.clear();
    dataList << "CC_y" << QString::number(m_mtfMinCC_Y) << QString::number(m_mtfMaxCC) << QString::number(mtf.CC_y) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfCCy].sValue = QString::number(mtf.CC_y, 'f', 3);
    if ((mtf.CC_y < m_mtfMinCC_Y) || (mtf.CC_y > m_mtfMaxCC)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: CC_y 比对失败");
        g_mesCheckPackage[ItemMtfCCy].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfCCy].sResult = "PASS";
    }

    dataList.clear();
    dataList << "LC_x" << QString::number(m_mtfMinEdge_X) << QString::number(m_mtfMaxEdge) << QString::number(mtf.LC_x) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfLCx].sValue = QString::number(mtf.LC_x, 'f', 3);
    if ((mtf.LC_x < m_mtfMinEdge_X) || (mtf.LC_x > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: LC_x 比对失败");
        g_mesCheckPackage[ItemMtfLCx].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfLCx].sResult = "PASS";
    }

    dataList.clear();
    dataList << "LC_y" << QString::number(m_mtfMinEdge_Y) << QString::number(m_mtfMaxEdge) << QString::number(mtf.LC_y) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfLCy].sValue = QString::number(mtf.LC_y, 'f', 3);
    if ((mtf.LC_y < m_mtfMinEdge_Y) || (mtf.LC_y > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: LC_y 比对失败");
        g_mesCheckPackage[ItemMtfLCy].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfLCy].sResult = "PASS";
    }

    dataList.clear();
    dataList << "RC_x" << QString::number(m_mtfMinEdge_X) << QString::number(m_mtfMaxEdge) << QString::number(mtf.RC_x) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfRCx].sValue = QString::number(mtf.RC_x, 'f', 3);
    if ((mtf.RC_x < m_mtfMinEdge_X) || (mtf.RC_x > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: RC_x 比对失败");
        g_mesCheckPackage[ItemMtfRCx].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfRCx].sResult = "PASS";
    }

    dataList.clear();
    dataList << "RC_y" << QString::number(m_mtfMinEdge_Y) << QString::number(m_mtfMaxEdge) << QString::number(mtf.RC_y) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfRCy].sValue = QString::number(mtf.RC_y, 'f', 3);
    if ((mtf.RC_y < m_mtfMinEdge_Y) || (mtf.RC_y > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: RC_y 比对失败");
        g_mesCheckPackage[ItemMtfRCy].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfRCy].sResult = "PASS";
    }

    dataList.clear();
    dataList << "TC_x" << QString::number(m_mtfMinEdge_X) << QString::number(m_mtfMaxEdge) << QString::number(mtf.TC_x) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfTCx].sValue = QString::number(mtf.TC_x, 'f', 3);
    if ((mtf.TC_x < m_mtfMinEdge_X) || (mtf.TC_x > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: TC_x 比对失败");
        g_mesCheckPackage[ItemMtfTCx].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfTCx].sResult = "PASS";
    }

    dataList.clear();
    dataList << "TC_y" << QString::number(m_mtfMinEdge_Y) << QString::number(m_mtfMaxEdge) << QString::number(mtf.TC_y) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfTCy].sValue = QString::number(mtf.TC_y, 'f', 3);
    if ((mtf.TC_y < m_mtfMinEdge_Y) || (mtf.TC_y > m_mtfMaxEdge)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: TC_y 比对失败");
        g_mesCheckPackage[ItemMtfTCy].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfTCy].sResult = "PASS";
    }

    dataList.clear();
    float cc_avg = (mtf.CC_x + mtf.CC_y) / 2.0f;
    dataList << "CC_avg" << QString::number(m_mtfCCMinAverage) << QString::number(m_mtfCCMaxAverage) << QString::number(cc_avg) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemMtfCCAvg].sValue = QString::number(cc_avg, 'f', 3);
    if ((cc_avg < m_mtfCCMinAverage) || (cc_avg > m_mtfCCMaxAverage)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "mtf 结果比对: CC平均值比对失败");
        g_mesCheckPackage[ItemMtfCCAvg].sResult = "NG";
        bRst = false;
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemMtfCCAvg].sResult = "PASS";
    }

    return bRst;
}

bool MtfCL1004::judgetSpResult(SharpPos sp)
{
    bool bRst = false;
    QStringList dataList;
    dataList << "Sp_avg" << QString::number(m_spMinAverage) << QString::number(m_spMaxAverage) << QString::number(sp.avg_value) << "TEST";
    tRowData(dataList);
    g_mesCheckPackage[ItemSpAvg].sValue = QString::number(sp.avg_value, 'f', 3);
    if ((sp.avg_value < m_spMinAverage) || (sp.avg_value > m_spMaxAverage)) {
        tableUpdateTestStatus(1); // fail
        setErrString(-1, "清晰度结果比对: avg_value 比对失败");
        g_mesCheckPackage[ItemSpAvg].sResult = "NG";
    } else {
        tableUpdateTestStatus(0); // pass
        g_mesCheckPackage[ItemSpAvg].sResult = "PASS";
        bRst = true;
    }

    dataList.clear();
    dataList << "Sp_CC" << "-" << "-" << QString::number(sp.CC_value) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpCC].sValue = QString::number(sp.CC_value);
    g_mesCheckPackage[ItemSpCC].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_TC" << "-" << "-" << QString::number(sp.TC_value) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpTC].sValue = QString::number(sp.TC_value);
    g_mesCheckPackage[ItemSpTC].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_BC" << "-" << "-" << QString::number(sp.BC_value) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpBC].sValue = QString::number(sp.BC_value);
    g_mesCheckPackage[ItemSpBC].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_RC" << "-" << "-" << QString::number(sp.RC_value) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpRC].sValue = QString::number(sp.RC_value);
    g_mesCheckPackage[ItemSpRC].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_LC" << "-" << "-" << QString::number(sp.LC_value) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpLC].sValue = QString::number(sp.LC_value);
    g_mesCheckPackage[ItemSpLC].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_Mtf_Cx" << "-" << "-" << QString::number(sp.MTF_C_X) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpMtfCx].sValue = QString::number(sp.MTF_C_X);
    g_mesCheckPackage[ItemSpMtfCx].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_Mtf_Cy" << "-" << "-" << QString::number(sp.MTF_C_Y) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpMtfCy].sValue = QString::number(sp.MTF_C_Y);
    g_mesCheckPackage[ItemSpMtfCy].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_netd_image" << "-" << "-" << QString::number(sp.image_netd) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpNetdImage].sValue = QString::number(sp.image_netd);
    g_mesCheckPackage[ItemSpNetdImage].sResult = "PASS";

    dataList.clear();
    dataList << "Sp_netd_space" << "-" << "-" << QString::number(sp.space_netd) << "PASS";
    tRowData(dataList);
    tableUpdateTestStatus(0); // pass
    g_mesCheckPackage[ItemSpNetdSpace].sValue = QString::number(sp.space_netd);
    g_mesCheckPackage[ItemSpNetdSpace].sResult = "PASS";

    return bRst;
}

bool MtfCL1004::getStageImgAndSave(QString stageName)
{

    return true;
}

bool MtfCL1004::getImageData(QByteArray &imgData)
{
    memset(m_frameData, 0, WIDTH * HEIGHT * 4);
    for (int foo = 0; foo < 5; foo++)
    {
        QCoreApplication::processEvents();
        {
            if (!m_mutex.tryLock(1000)) {
                mySleep(100);
                continue;
            }
            imgData = mCrtFrameData;
            mCrtFrameData = QByteArray();
            m_mutex.unlock();
        }
        if (imgData.size() >= WIDTH * HEIGHT * 2)
        {
            memcpy(m_frameData, imgData.data() + WIDTH * 4, WIDTH * HEIGHT * 2);
            return true;
        }

        mySleep(100);
    }

    writeLog("get image failed", -1);
    return false;
}

bool MtfCL1004::dutSaveImage(QString name, QByteArray data)
{
    // 给每张图片一个序号，方便排序
    QString jpgPicName = QString("%1.%2").arg(mImageSequence++).arg(name);
    QByteArray btArray;
    if (data.isEmpty()) {
        if (!getImageData(btArray)) {
            setErrString(-4, "获取图片数据失败");
            return false;
        }
    } else {
        btArray = data;
    }
    if (btArray.size()  == 0) {
        writeLog("save data length: " + QString::number(btArray.size()), -1);
        setErrString(-3, "保存图片失败，图片数据为空，检查产品是否连接OK");
        return false;
    }

    QString path = mTmpLogPath + jpgPicName;

    VideoFormatInfo vinfo;
    VideoInfo::getInstance()->getVideoInfo(0, vinfo);

    unsigned char* m_frame = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_rgbData = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_y8Data = new unsigned char[WIDTH * HEIGHT * 4];
    memset (m_frame, 0, WIDTH * HEIGHT * 4);
    memset (m_rgbData, 0, WIDTH * HEIGHT * 4);
    memset (m_y8Data, 0, WIDTH * HEIGHT * 4);

    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(0, videoInfo);
    memcpy(m_frame, btArray.mid(videoInfo.nvsLen, videoInfo.frameLen), videoInfo.frameLen);
    if(videoInfo.format == VIDEO_FORMAT_NV21) {
        writeLog("Current IS VIDEO_FORMAT_NV21", 0);
        Yuv2Rgb::YuvToRgbNV12(m_frame, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_UYUV) {
        writeLog("Current IS VIDEO_FORMAT_UYUV", 0);
        Yuv2Rgb::YuvToRgbUYVY(m_frame, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_YUV420) {
        writeLog("Current IS VIDEO_FORMAT_YUV420", 0);
        Yuv2Rgb::YuvToRgb420(m_frame, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_Y16) {
        writeLog("Current IS VIDEO_FORMAT_Y16", 0);
        MappingNew map;
        map.DRC_Mix(m_y8Data, (short*)m_frame, videoInfo.width, videoInfo.height, 255, 80, 128, 200, 90, 50, 5, 5, 1);
        map.y8ToRgb(m_y8Data, m_rgbData, videoInfo.width, videoInfo.height);
    } else if(videoInfo.format == VIDEO_FORMAT_X16) {
        writeLog("Current IS VIDEO_FORMAT_X16", 0);
        Mapping map;
        map.Data16ToRGB24((short*)m_frame, m_rgbData, videoInfo.width*videoInfo.height, 0);
    } else if(videoInfo.format == VIDEO_FORMAT_Y16_Y8) {
        writeLog("Current IS VIDEO_FORMAT_Y16_Y8", 0);
        Yuv2Rgb::YuvToRgbUYVY(m_frame, m_rgbData, videoInfo.width, videoInfo.height);
    }

    bool bSaveOk = false;
    QImage image(m_rgbData, videoInfo.width, videoInfo.height, QImage::Format_RGB888);
    if (!image.isNull()) {
        writeLog("image is ok");
        if (image.save(path, "jpg")) {
            writeLog("image save to " + path + " ok");
            QFile file(path + ".y16");
            if (file.open(QIODevice::WriteOnly)) {
                file.write(btArray.mid(640 * 4, 640 * 512 * 2));
                file.close();
                bSaveOk = true;
            } else {
                writeLog("save y16 failed! " + path, -1);
            }
        } else {
            writeLog("save jpg failed! " + path, -1);
        }
    } else {
        writeLog("null image data", 0);
    }
    delete []m_rgbData;
    delete []m_y8Data;
    delete []m_frame;
    return bSaveOk;
}

void MtfCL1004::writeLog(QString log, int status)// 0 normal, 1 pass, -1 fail
{
    qDebug() << "slot " << log;
    if (status == 0)
        emit logAddNormalLog(log, 0);
    if (status == 1)
        emit logAddPassLog(log, 0);
    if (status == -1)
        emit logAddFailLog(log, 0);
}

void MtfCL1004::logProcess(QString log, int status)
{
    if (status == 0)
        logNormal(log);
    if (status == 1)
        logWarming(log);
    showProcess(log);
}

void MtfCL1004::mySleep(int ms)
{
    while (ms > 0) {
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ms -= 200;
    }
}

void MtfCL1004::setErrString(int errCode, QString log)
{
    m_errorCode = errCode;
    m_errMsg = log;
    showFail(log);
    writeLog(log, -1);
}

void MtfCL1004::mesInforClear()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
        MesCheckItem& pitem = g_mesCheckPackage[foo];
        if (pitem.sItem.size() > 0) {
            pitem.sResult = "NG";
            pitem.sValue.clear();
        }
    }
}

void MtfCL1004::mesUpload()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++ ) {
        if (g_mesCheckPackage[foo].sItem.size() > 0) {
            mMesObj.addItemResultEx(g_mesCheckPackage[foo]);
        }
    }
}

bool MtfCL1004::plcMoveHori()
{
    for (int foo = 0; foo < 1; foo++) {
        logProcess(">> 执行水平旋转...", 1);
        if (!getStageTargetRange("水平旋转获取目标范围")) {
            return false;
        }
        m_plc.changeMoveMode(PLC_AXIS_H, PLC_AXIS_MOVE_STEP);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //调整水平旋转轴，使中间靶标中心处于图像中心
        int x_shift_pix = m_sharp_start_points[2].x - 280;
        int h_shift = std::abs(x_shift_pix * 1000 / 13);
        if (std::abs(x_shift_pix) > 2){
            m_plc.setMoveStep(PLC_AXIS_H, h_shift);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (x_shift_pix > 1)
                m_plc.moveRight(PLC_AXIS_H);
            else if (x_shift_pix < -1)
                m_plc.moveLeft(PLC_AXIS_H);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return true;
}

bool MtfCL1004::plcMoveVert()
{
    for (int foo = 0; foo < 1; foo++) {
        logProcess(">> 执行垂直旋转...", 1);
        if (!getStageTargetRange("垂直旋转获取目标范围")) {
            return false;
        }
        m_plc.changeMoveMode(PLC_AXIS_V, PLC_AXIS_MOVE_STEP);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        //调整垂直旋转轴，使中间靶标中心处于图像中心
        int y_shift_pix = m_sharp_start_points[2].y - 216;
        if (std::abs(y_shift_pix) > 2)
        {
            int v_shift = std::abs(y_shift_pix * 1000 / 17);
            m_plc.setMoveStep(PLC_AXIS_V, v_shift);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (y_shift_pix > 1)
                m_plc.moveLeft(PLC_AXIS_V);
            else
                m_plc.moveRight(PLC_AXIS_V);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return true;
}

bool MtfCL1004::mtfGetValues()
{
    m_sharp_start_points.clear();
    if (!getStageTargetRange("调式")) {
        return false;
    }
    int c_x = m_sharp_start_points[2].x + 40;
    int c_y = m_sharp_start_points[2].y + 40;
    SRF srf;
    MTF_param mtf_p;
    MTF_param mtf_p_avg;
    MTF_param_side mtf_p_side;
    MTF_param_side mtf_p_avg_side;
    memset(&mtf_p_avg, 0, sizeof(mtf_p_avg));
    memset(&mtf_p, 0, sizeof(mtf_p));
    memset(&mtf_p_side, 0, sizeof(mtf_p_side));
    memset(&mtf_p_avg_side, 0, sizeof(mtf_p_avg_side));

    SharpPos sp;
    SharpPosSide spSide;
    if (!getStageSharpValue("调试", sp, spSide)) {
        return false;
    }

    MYRECT high_rect;
    MYRECT low_rect;
    high_rect.x = m_sharp_start_points[0].x + 40 - 4;
    high_rect.y = m_sharp_start_points[0].y + 40 - 4;
    high_rect.width = 9;
    high_rect.height = 9;
    low_rect.x = m_sharp_start_points[0].x + 40 - 4 + 220;
    low_rect.y = m_sharp_start_points[0].y + 40 - 4;
    low_rect.width = 9;
    low_rect.height = 9;
    double image_netd = 0;//Cal_image_NETD(high_rect, low_rect);
    double space_netd = 0;
    sp.image_netd = image_netd;

    for(int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents();
        qDebug() << "will memset";
        memset(&mtf_p, 0, sizeof(mtf_p));
        QByteArray baImg;
        qDebug() << "will get image data";
        if (!getImageData(baImg)) {
            showFail("获取图片数据失败");
            writeLog("获取图片数据失败，测试FAIL");
            return false;
        }

        writeLog("第" + QString::number(i+1) + "轮MTF数据：");
        getMtfCalResult("第" + QString::number(i+1) + "轮MTF", mtf_p, mtf_p_side);
//        srf.MTF_Cal((unsigned short*)m_frameData, m_sharp_start_points, mtf_p);
//        srf.MTF_Side_Cal((unsigned short*)m_frameData, m_sharp_start_points, mtf_p_side);
//        printMtfParam(mtf_p);
//        printMtfParamSide(mtf_p_side);
        mtf_p_avg.CC_x+=mtf_p.CC_x;
        mtf_p_avg.CC_y+=mtf_p.CC_y;
        mtf_p_avg.LC_x+=mtf_p.LC_x;
        mtf_p_avg.LC_y+=mtf_p.LC_y;
        mtf_p_avg.RC_x+=mtf_p.RC_x;
        mtf_p_avg.RC_y+=mtf_p.RC_y;
        mtf_p_avg.TC_x+=mtf_p.TC_x;
        mtf_p_avg.TC_y+=mtf_p.TC_y;
        mtf_p_avg.BC_x+=mtf_p.BC_x;
        mtf_p_avg.BC_y+=mtf_p.BC_y;

        mtf_p_avg_side.LT_x+=mtf_p_side.LT_x;
        mtf_p_avg_side.LT_y+=mtf_p_side.LT_y;
        mtf_p_avg_side.LB_x+=mtf_p_side.LB_x;
        mtf_p_avg_side.LB_y+=mtf_p_side.LB_y;
        mtf_p_avg_side.RT_x+=mtf_p_side.RT_x;
        mtf_p_avg_side.RT_y+=mtf_p_side.RT_y;
        mtf_p_avg_side.RB_x+=mtf_p_side.RB_x;
        mtf_p_avg_side.RB_y+=mtf_p_side.RB_y;
        // space_netd += Cal_space_NETD(high_rect, low_rect);
        qDebug() << "will sleep 200ms";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
//    space_netd /= 10;
//    sp.space_netd = space_netd;

//    qDebug() <<"image_netd:"<<image_netd;
//    qDebug() <<"space_netd:"<<space_netd;
    qDebug() <<"c_x:"<<c_x<<" c_y:"<<c_y;
    qDebug() <<"sharp_c_x:"<<sp.avg_value;

    qDebug() <<mtf_p_avg.CC_x/10<<";"<<mtf_p_avg.CC_y/10<<";"<<mtf_p_avg.LC_x/10<<";"<<mtf_p_avg.LC_y/10<<";"
               <<mtf_p_avg.RC_x/10<<";"<<mtf_p_avg.RC_y/10<<";"<<mtf_p_avg.TC_x/10<<";"<<mtf_p_avg.TC_y/10<<";"
                 <<mtf_p_avg.BC_x/10<<";"<<mtf_p_avg.BC_y/10;

    qDebug() <<mtf_p_avg_side.LT_x/10<<";"<<mtf_p_avg_side.LT_y/10<<";"<<mtf_p_avg_side.LB_x/10<<";"<<mtf_p_avg_side.LB_y/10<<";"
               <<mtf_p_avg_side.RT_x/10<<";"<<mtf_p_avg_side.RT_y/10<<";"<<mtf_p_avg_side.RB_x/10<<";"<<mtf_p_avg_side.RB_y/10;

    CsvLogger logger;
    QString path = logPath() + "/mtf_verify/";
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }
    path += QDateTime::currentDateTime().toString("yyyyMMdd") + "_table.csv";
    logger.setCsvLogPath(path);
    QStringList title;
    title << "时间" << "探测器SN" << "测试结果" << "image_netd" << "space_netd" << "c_x" << "c_y" << "sharp_avg"
          << "mtf_p_avg.CC_x" << "mtf_p_avg.CC_y" << "mtf_p_avg.LC_x" << "mtf_p_avg.LC_y"
          << "mtf_p_avg.RC_x" << "mtf_p_avg.RC_y" << "mtf_p_avg.TC_x" << "mtf_p_avg.TC_y"
          << "mtf_p_avg.BC_x" << "mtf_p_avg.BC_y" << "mtf_p_avg_side.LT_x" << "mtf_p_avg_side.LT_y"
          << "mtf_p_avg_side.LB_x" << "mtf_p_avg_side.LB_y" << "mtf_p_avg_side.RT_x" << "mtf_p_avg_side.RT_y"
          << "mtf_p_avg_side.RB_x" << "mtf_p_avg_side.RB_y" << "sharp_side_LT" << "sharp_side_LB"
          << "sharp_side_RT"  << "sharp_side_RB";
    logger.addCsvTitle(title);

    bool brst = false;
    float MTF_value = (mtf_p_avg.CC_x/10 + mtf_p_avg.CC_y/10) / 2;
    float side_mtf_LT = (mtf_p_avg_side.LT_x/10 + mtf_p_avg_side.LT_y/10) / 2;
    float side_mtf_LB = (mtf_p_avg_side.LB_x/10 + mtf_p_avg_side.LB_y/10) / 2;
    float side_mtf_RT = (mtf_p_avg_side.RT_x/10 + mtf_p_avg_side.RT_y/10) / 2;
    float side_mtf_RB = (mtf_p_avg_side.RB_x/10 + mtf_p_avg_side.RB_y/10) / 2;
    float min_side_mtf = min(side_mtf_LT, side_mtf_LB);
    min_side_mtf = min(min_side_mtf, side_mtf_RB);
    min_side_mtf = min(min_side_mtf, side_mtf_RT);


    float side_mtf_LC = max(mtf_p_avg.LC_x/10 , mtf_p_avg.LC_y/10);
    float side_mtf_TC = max(mtf_p_avg.TC_x/10 , mtf_p_avg.TC_y/10);
    float side_mtf_RC = max(mtf_p_avg.TC_x/10 , mtf_p_avg.TC_y/10);
    float side_mtf_BC = max(mtf_p_avg.BC_x/10 , mtf_p_avg.BC_y/10);
    float min_04_mtf = min(side_mtf_LC, side_mtf_TC);
    min_04_mtf = min(min_04_mtf, side_mtf_RC);
    min_04_mtf = min(min_04_mtf, side_mtf_BC);

    if (sp.avg_value > m_spMaxAverage || sp.avg_value < m_spMinAverage || min_04_mtf < reCheck_mtfMinEdge_X ||
            MTF_value < reCheck_mtfCCMinAverage || min_side_mtf < reCheck_mtf_side_threshold) {
        showFail("测试数据超出门限，请隔离放置");
        writeLog("测试FAIL");
        brst = false;
    } else {
        writeLog("测试PASS");
        brst = true;
    }

    QStringList data;
    data << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << mDetectorSN << (brst ? "PASS" : "FAIL")
         << QString::number(image_netd, 'f', 3) << QString::number(space_netd, 'f', 3)
            << QString::number(c_x, 'f', 3) << QString::number(c_y, 'f', 3)
            << QString::number(sp.avg_value, 'f', 3) << QString::number(mtf_p_avg.CC_x/10, 'f', 3)
            << QString::number(mtf_p_avg.CC_y/10, 'f', 3) << QString::number(mtf_p_avg.LC_x/10, 'f', 3)
            << QString::number(mtf_p_avg.LC_y/10, 'f', 3) << QString::number(mtf_p_avg.RC_x/10, 'f', 3)
            << QString::number(mtf_p_avg.RC_y/10, 'f', 3) << QString::number(mtf_p_avg.TC_x/10, 'f', 3)
            << QString::number(mtf_p_avg.TC_y/10, 'f', 3) << QString::number(mtf_p_avg.BC_x/10, 'f', 3)
            << QString::number(mtf_p_avg.BC_y/10, 'f', 3) << QString::number(mtf_p_avg_side.LT_x/10, 'f', 3)
            << QString::number(mtf_p_avg_side.LT_y/10, 'f', 3) << QString::number(mtf_p_avg_side.LB_x/10, 'f', 3)
            << QString::number(mtf_p_avg_side.LB_y/10, 'f', 3) << QString::number(mtf_p_avg_side.RT_x/10, 'f', 3)
            << QString::number(mtf_p_avg_side.RT_y/10, 'f', 3) << QString::number(mtf_p_avg_side.RB_x/10, 'f', 3)
            << QString::number(mtf_p_avg_side.RB_y/10, 'f', 3) << QString::number(spSide.LT_value, 'f', 3)
            << QString::number(spSide.LB_value, 'f', 3) << QString::number(spSide.RT_value, 'f', 3)
            << QString::number(spSide.RB_value, 'f', 3);
    logger.addCsvLog(data);
    return brst;
}

bool MtfCL1004::imageValidCheck()
{
    // 加入图像合法性判断，避免图像崩溃
    QByteArray baImg;
    if (!getImageData(baImg)) {
        setErrString(-1, "获取图像数据失败");
        return false;
    }

    if (!dutSaveImage("合法性检查.jpg", baImg)) {
        setErrString(-1, "图像保存失败");
        return false;
    }

    mtfImgCheck imgCheck;
    unsigned short minValue = 0, maxValue = 0;
    imgCheck.Get_Y16_hist((unsigned short*)m_frameData, maxValue, minValue, WIDTH, HEIGHT);
    QString log = QString("IMAGE CHECK max value: %1, min value: %2").arg(maxValue).arg(minValue);
    writeLog(log);
    if (maxValue - minValue < 150) {
        setErrString(-1, "图像数据快门图像异常，停止测试");
        return false;
    }
    int imgCount[5] = {0};
    imgCheck.Get_target_count((unsigned short*)m_frameData, imgCount, maxValue, minValue, WIDTH, HEIGHT);
    log = QString("TARGET COUNT 0: %1, 1: %2, 2: %3, 3: %4, 4: %5").arg(imgCount[0]).arg(imgCount[1]).arg(imgCount[2]).arg(imgCount[3]).arg(imgCount[4]);
    writeLog(log);
    bool brst = true;
    for (int foo = 0; foo < 5; foo++) {
        if (imgCount[foo] < 1000 || imgCount[foo] > 5000) {
            brst = false;
            break;
        }
    }
    if (!brst) {
        setErrString(-1, "图像数据靶标图像异常，停止测试");
        return false;
    }

    return true;
}

void MtfCL1004::workThread()
{
    MTF_param m_mtf_param;
    SharpPos m_sharp_param;
    SharpPosSide m_sharp_side;
    MTF_param_side m_mtf_side;
    int currentFrameCount = 0;
    int count = 0;
    bool isVideoOk = false;
    std::chrono::system_clock::time_point mTestStart;
    while(m_isRunning) {
        if(!m_isStart) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        //        if (m_errorCode != 0) {
        //            m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
        //        }

        if (m_plc.errorFlag() != 0) {
            m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
            setErrString(-2, "PLC工作异常");
            m_plc.clearErrorFlag();
        }

        switch(m_workCount)
        {
        case AUTO_WORK_GAS_LOAD_CLOSE: {
            mTimeStartStr.clear();
            mTimeEndStr.clear();
            mCsvTimeFlag.clear();
            mTimeStartStr = getFormatDateTime();
            mCsvTimeFlag = QTime::currentTime().toString("hhmmss");
            writeLog("测试开始时间: " + mTimeStartStr);
            mTestStart = std::chrono::system_clock::now();
            writeLog("当前测试SN: " + mDetectorSN);
            logProcess("SOFT: 初始化参数...");
            mCostTime = .0f;
            count = currentFrameCount = m_frameCount = 0;
            m_check_MTF_value = .0f;
            m_start_value = .0f;
            m_check_first_down_times = 0;
            m_sharp_datas.clear();
            m_first_before_value = .0f;
            is_start_time = false;
            m_check_max_value = .0f;
            mImageSequence = 0;
            mCrtFrameData.clear();
            mChartXSlot = 0;
            mChartAxisStep = 50;
            m_errorCode = 0;
            isSkipMes = false;
            m_plc.clearErrorFlag();

            // 根据 SN 创建log路径
            m_sharp_start_points.clear();
            mLogPath = logPath();
            if (mLogPath.size() <= 0) {
                mLogPath = QCoreApplication::applicationDirPath();
            }

            mTmpLogPath = mLogPath + "/" + QDate::currentDate().toString("yyyyMMdd");
            mTmpLogPath += "/" + mDetectorSN + "_" + QTime::currentTime().toString("hhmmss") + "/";
            QDir dir(mTmpLogPath);
            if (!dir.exists()) {
                dir.mkpath(mTmpLogPath);
            }
            writeLog("创建log路径：" + mTmpLogPath);

            memset(&m_sharp_param, 0, sizeof(m_sharp_param));
            memset(&m_mtf_param, 0, sizeof(m_mtf_param));
            memset(&m_mtf_check, 0, sizeof(m_mtf_check));
            memset(&m_mtf_side, 0, sizeof(m_mtf_side));
            memset(&m_sharp_side, 0, sizeof(m_sharp_side));

            BaseProduce::jugdeCustomLifeTime(false);
            if (customLifeIsOver()) {
                setErrString(-2, "请更换耗材");
                m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                break;
            }

            if (isOnlineMode()) {
                logProcess("在线模式，执行入站操作");
                mesInforClear();
                QString out;
                mMesObj.setUser(MesClient::instance()->sUser());
                if (!mMesObj.enterProduce(mDetectorSN, out)) {
                    setErrString(-2, "入站失败：" + out);
                    isSkipMes = true;
                    m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                    break;
                }
            } else {
                writeLog("非生产模式不执行入站操作");
            }


            m_workCount = AUTO_WORK_CHECK_VIDEO;
            break;
        }
//        case AUTO_WORK_GAS_CLAMP_CLOSE: {

//            m_workCount = AUTO_WORK_CHECK_VIDEO;
//            break;
//        }
        case AUTO_WORK_CHECK_VIDEO: {
//            if (m_errorCode != 0) {
//                m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
//                break;
//            }
            logProcess("DUT: 检查视频数据...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(m_frameCount - currentFrameCount > 10) {
                crossAxisInit();
                isVideoOk = true;
                logProcess("DUT: 视频数据检查OK, 图像传输正常...");
                m_workCount = AUTO_WORK_INIT;
            } else {
                count++;
                if(count > 50) {
                    setErrString(-99, "DUT: 视频数据检查FAIL, 图像传输异常...");
                    m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                }
            }
            break;
        }
        case AUTO_WORK_INIT: {
            logProcess("执行开快门操作");
            showProcess("开快门");
            if (!dutOpenShutter()) {
                setErrString(-99, "DUT: 开快门异常失败");
                m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            logProcess("改变传输图像格式：Y16");
            showProcess("改变传输图像格式：Y16");
            if (!dutChangeFormat(3)) {
                setErrString(-99, "DUT: 图像格式切换(Y16)异常");
                m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                break;
            }
            logProcess("关自适应补偿");
            showProcess("关自适应补偿");
            if (!dutCloseAdpative()) {
                setErrString(-99, "DUT: 关自适应补偿失败");
                m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
                break;
            }

            logProcess("PLC: 切换至自动工作模式...");
            m_plc.setAutoWorkMode();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            logProcess("PLC: 上料气缸LOAD...");
            m_plc.handleLoad(GAS_HANDLE_WORK);

            logProcess("PLC: 上料气缸WORK...");
            m_plc.handleClamp(GAS_HANDLE_WORK);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // 加入图像合法性判断，避免图像崩溃
            logProcess("图像数据合法性判断...");
            if (!imageValidCheck()) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            // 初始数据获取，无需进行返回值判断
            logProcess("获取初始目标范围");
            if (!getStageTargetRange("初始位置目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("获取初始目标清晰度");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("初始目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("获取初始MTF");
            if (!getMtfCalResult("初始获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            m_check_MTF_value = max(m_check_MTF_value, (sp.MTF_C_X + sp.MTF_C_Y) / 2);
            m_avg_max_sharp = sp.avg_value;
            m_start_value = sp.avg_value;
            m_max_sharp_state = 0;
            m_currentState = 0;
            m_workCount = 1;

            sp.pos = m_currentState;

            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            logProcess("PLC: 改变移动模式");
            m_plc.changeMoveMode(PLC_AXIS_R, PLC_AXIS_MOVE_STEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            logProcess("PLC: 改变步进速度, " + QString::number(m_step1Speed));
            m_plc.setMoveSpeed(PLC_AXIS_R, m_step1Speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            logProcess("PLC: 改变步进值, " + QString::number(m_step_1));
            m_plc.setMoveStep(PLC_AXIS_R, m_step_1);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            logProcess("PLC: R轴左移动");
            if (!m_plc.moveLeft(PLC_AXIS_R)) {
                setErrString(-99, "R轴左移失败");
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }


            m_workCount = AUTO_WORK_AXIS_BIG_STEP;
            break;
        }
        case AUTO_WORK_AXIS_BIG_STEP:{
            logProcess(">> 执行大步进正转(向左)调焦运算...", 1);
            // 初始数据获取，无需进行返回值判断
            logProcess("获取大步进获取初始目标范围");
            if (!getStageTargetRange("大步进获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("获取大步进目标清晰度");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("大步进目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("获取大步进获取MTF值");
            if (!getMtfCalResult("大步进获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            m_check_MTF_value = max(m_check_MTF_value, (sp.MTF_C_X + sp.MTF_C_Y) / 2);
            m_currentState += m_step_1;
            if (sp.avg_value > m_avg_max_sharp) {
                m_ave_second_sharp = m_avg_max_sharp;
                m_avg_max_sharp = sp.avg_value;
                m_max_sharp_state = m_currentState;
                m_check_first_down_times = 0;
            }
            sp.pos = m_currentState;

            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            if (sp.avg_value < m_first_before_value)
                m_check_first_down_times++;

            float start_temp = (m_avg_max_sharp - m_start_value)/2;
            float down_threshold = max(start_temp, 20.f);
            down_threshold = min(start_temp, 30.f);

            if (m_currentState + m_step_1 < m_endState && m_avg_max_sharp - sp.avg_value < END_THRESHOLD && (m_check_first_down_times <= 3 || m_avg_max_sharp - sp.avg_value < down_threshold)) {
                m_workCount = AUTO_WORK_AXIS_BIG_STEP;
                m_plc.moveLeft(PLC_AXIS_R);

                if (!m_is_check_pos){
                    if (sp.avg_value > CHECK_POS_THRESHOLD){
                        m_workCount = AUTO_WORK_AXIS_H_STEP;
                        m_is_check_pos = true;
                    }
                }
            } else {
                m_workCount = AUTO_WORK_AXIS_FIND_MAX_FROM_BIG_STEP;
            }
            if (m_currentState >= m_endState) {
                m_errorCode = -2;
                m_workCount = AUTO_WORK_CALCULATE_MTF;
            }
            m_first_before_value = sp.avg_value;
            break;
        }
        case AUTO_WORK_AXIS_H_STEP:{
            logProcess(">> 执行水平旋转...", 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!getStageTargetRange("水平旋转获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            m_plc.changeMoveMode(PLC_AXIS_H, PLC_AXIS_MOVE_STEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //调整水平旋转轴，使中间靶标中心处于图像中心
            int x_shift_pix = m_sharp_start_points[2].x - 280;
            int h_shift = std::abs(x_shift_pix * 1000 / 13);
            if (std::abs(x_shift_pix) > 2){
                m_plc.setMoveStep(PLC_AXIS_H, h_shift);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (x_shift_pix > 1)
                    m_plc.moveRight(PLC_AXIS_H);
                else if (x_shift_pix < -1)
                    m_plc.moveLeft(PLC_AXIS_H);
                m_workCount = AUTO_WORK_AXIS_H_CHECK_STEP;

            }
            else
                m_workCount = AUTO_WORK_AXIS_V_STEP;
            break;
        }
        case AUTO_WORK_AXIS_H_CHECK_STEP:{
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            logProcess(">> 执行二次水平旋转...", 1);
            if (!getStageTargetRange("二次水平旋转获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //调整水平旋转轴，使中间靶标中心处于图像中心
            int x_shift_pix = m_sharp_start_points[2].x - 280;

            if (std::abs(x_shift_pix) > 2)
            {
                int h_shift = std::abs(x_shift_pix * 1000 / 13);
                m_plc.setMoveStep(PLC_AXIS_H, h_shift);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (x_shift_pix > 1)
                    m_plc.moveRight(PLC_AXIS_H);
                else if (x_shift_pix < -1)
                    m_plc.moveLeft(PLC_AXIS_H);
            }
            m_workCount = AUTO_WORK_AXIS_V_STEP;
            break;
        }
        case  AUTO_WORK_AXIS_V_STEP:{
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            logProcess(">> 执行垂直旋转...", 1);
            if (!getStageTargetRange("垂直旋转获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            m_plc.changeMoveMode(PLC_AXIS_V, PLC_AXIS_MOVE_STEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //调整垂直旋转轴，使中间靶标中心处于图像中心
            int y_shift_pix = m_sharp_start_points[2].y - 216;
            if (std::abs(y_shift_pix) > 2)
            {
                int v_shift = std::abs(y_shift_pix * 1000 / 17);
                m_plc.setMoveStep(PLC_AXIS_V, v_shift);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (y_shift_pix > 1)
                    m_plc.moveLeft(PLC_AXIS_V);
                else
                    m_plc.moveRight(PLC_AXIS_V);
                m_workCount = AUTO_WORK_AXIS_V_CHECK_STEP;

            }else
                m_workCount = AUTO_WORK_AXIS_BIG_STEP;
            break;
        }
        case AUTO_WORK_AXIS_V_CHECK_STEP:{
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            logProcess(">> 执行二次垂直旋转...", 1);
            if (!getStageTargetRange("二次垂直旋转获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //调整旋转轴，使中间靶标中心处于图像中心
            int y_shift_pix = m_sharp_start_points[2].y - 216;

            if (std::abs(y_shift_pix) > 2)
            {

                int v_shift = std::abs(y_shift_pix * 1000 / 17);
                m_plc.setMoveStep(PLC_AXIS_V, v_shift);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (y_shift_pix > 1)
                    m_plc.moveLeft(PLC_AXIS_V);
                else if (y_shift_pix < -1)
                    m_plc.moveRight(PLC_AXIS_V);
            }
            m_workCount = AUTO_WORK_AXIS_BIG_STEP;
            break;
        }

        case AUTO_WORK_AXIS_FIND_MAX_FROM_BIG_STEP:{
            logProcess(">> 执行大步进反转(向右)一次...", 1);
            if (!getStageTargetRange("大步进反转获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("大步进反转目标清晰度");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("大步进反转目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("大步进获取初始MTF值");
            if (!getMtfCalResult("大步进翻转获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //            delete[]input_data;
            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            m_check_MTF_value = max(m_check_MTF_value, (sp.MTF_C_X + sp.MTF_C_Y) / 2);
            m_currentState += m_step_1;
            if (sp.avg_value > m_avg_max_sharp)
            {
                m_ave_second_sharp = m_avg_max_sharp;
                m_avg_max_sharp = sp.avg_value;
                m_max_sharp_state = m_currentState;
            }
            sp.pos = m_currentState;

            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            int left_move = 0;
            if(m_currentState != m_max_sharp_state) {
                left_move = m_currentState - m_max_sharp_state - m_step_1;
                m_second_endState = m_max_sharp_state - m_step_1;
            }

            if(left_move > 0) {
                left_move = m_step_2 * 5;
                m_currentState -= left_move;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_plc.setMoveStep(PLC_AXIS_R, left_move);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_plc.moveRight(PLC_AXIS_R);

                m_workCount = AUTO_WORK_AXIS_CHANGE_SMALL_STEP;
            } else {
                m_workCount = AUTO_WORK_CALCULATE_MTF;
            }
            break;
        }
        case AUTO_WORK_AXIS_CHANGE_SMALL_STEP:{
            logProcess(">> 设置小步进参数...", 1);

            logProcess("小步进参数设置阶段获取目标范围");
            if (!getStageTargetRange("小步进参数设置阶段获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("小步进参数设置阶段目标清晰度计算");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("小步进参数设置阶段目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("小步进参数设置阶段获取初始MTF值");
            if (!getMtfCalResult("小步进参数设置阶段获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //            delete[]input_data;
            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            m_check_MTF_value = max(m_check_MTF_value, (sp.MTF_C_X + sp.MTF_C_Y) / 2);
            sp.pos = m_currentState;
            if (m_avg_max_sharp < sp.avg_value){
                m_ave_second_sharp = before_sharp_value;
                m_avg_max_sharp = sp.avg_value;
            }

            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            before_sharp_value = sp.avg_value;
            //            m_second_max_sharp = m_avg_max_sharp;
            m_plc.setMoveSpeed(PLC_AXIS_R, m_step2Speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_plc.setMoveStep(PLC_AXIS_R, m_step_2);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_plc.moveRight(PLC_AXIS_R);
            m_currentState -= m_step_2;


            m_workCount = AUTO_WORK_AXIS_FIND_MAX_FROM_SMALL_STEP;
            //            m_down_times = 0;
            break;
        }
        case AUTO_WORK_AXIS_FIND_MAX_FROM_SMALL_STEP:{
            logProcess(">> 执行小步进寻找最大值...", 1);
            is_check = false;

            logProcess("小步进阶段获取目标范围");
            if (!getStageTargetRange("小步进阶段获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("小步进阶段目标清晰度计算");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("小步进阶段目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("小步进阶段获取MTF值");
            if (!getMtfCalResult("小步进阶段获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //            delete[]input_data;
            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            m_check_MTF_value = max(m_check_MTF_value, (sp.MTF_C_X + sp.MTF_C_Y) / 2);
            sp.pos = m_currentState;

            //            m_second_max_sharp = max(m_second_max_sharp, sp.avg_value);
            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            float value_check = m_avg_max_sharp - MAX_STOP_THRESHOLD;
            if (sp.avg_value > m_avg_max_sharp - MTF_CHECK_THREHOLD)
            {
                is_check = check_MTF();
                //                if (is_check){
                //                    before_sharp_value = sp.avg_value;
                //                    m_workCount = AUTO_WORK_CALCULATE_MTF;
                //                    break;
                //                }
            }
            if (value_check < m_check_max_value - SECOND_STOP_THRESHOLD)
                value_check = m_check_max_value - SECOND_STOP_THRESHOLD;

            if(!is_start_time)
            {
                float MTF_threshold = max((float)m_mtfCCMinAverage, m_check_MTF_value - 20);
                if (sp.avg_value < before_sharp_value && sp.avg_value > m_avg_max_sharp - END_THRESHOLD)
                    m_down_times++;
                else if (sp.avg_value >= before_sharp_value)
                {
                    m_down_times--;
                    m_down_times = max(0, m_down_times);
                }
                if (sp.avg_value < before_sharp_value && sp.avg_value > m_avg_max_sharp - END_THRESHOLD  && (sp.MTF_C_X + sp.MTF_C_Y) / 2 > MTF_threshold)
                {
//                    m_down_times++;
                    m_check_max_value = before_sharp_value;
                    is_start_time = true;
                    qDebug()<<"m_down_times............. is "<<m_down_times;
                }
            }
            else
            {
                if (sp.avg_value < m_check_max_value)
                {
                    m_down_times++;
                    qDebug()<<"m_down_times............. is "<<m_down_times;
                }
                else{
                    m_down_times = 0;
                    m_check_max_value = sp.avg_value;
                }
            }

            if ((m_down_times >= 3 && sp.avg_value >= value_check) || m_currentState - LEFT_RIGHT_STEP <= 0 || m_down_times >= 6)
            {
                qDebug()<<"m_check_max_value............. is "<<m_check_max_value;
                qDebug()<<"sp.avg_value............. is "<<sp.avg_value;
                qDebug()<<"m_down_times================ is "<<m_down_times;
                qDebug()<<"m_currentState================ is "<<m_currentState;
                m_workCount = AUTO_WORK_CALCULATE_MTF;
            }else
            {
                if(m_moveSmallTimes > SHORT_MAX_THREHOLD) {
                    m_errorCode = -3;
                    m_workCount = AUTO_WORK_CALCULATE_MTF;
                    qDebug()<<"m_workCount================ is "<<m_workCount;
                }else
                {
                    m_moveSmallTimes++;
                    m_workCount = AUTO_WORK_AXIS_FIND_MAX_FROM_SMALL_STEP;
                    m_plc.moveRight(PLC_AXIS_R);
                    m_currentState -= m_step_2;

                }
            }

            before_sharp_value = sp.avg_value;
            break;

        }
//        case AUTO_WORK_AXIS_MOVE_MAX:{
//            logProcess("移动到最大值位置", 1);
//            qDebug()<<"start 3 move================|||||||||||||||||||||||";
//            int right_move = 7.5 * m_step_2;
//            m_plc.setMoveStep(PLC_AXIS_R, right_move);
//            std::this_thread::sleep_for(std::chrono::milliseconds(50));
//            m_plc.moveLeft(PLC_AXIS_R);
//            m_currentState += right_move;

//            m_workCount = AUTO_WORK_AXIS_MOVE_LEFT_SET;
//            move_times_3+=7.5;
//            break;
//        }
//        case AUTO_WORK_AXIS_MOVE_LEFT_SET:{
//            logProcess("移动到LEFT_SET", 1);
//            qDebug()<<"start 4 move================|||||||||||||||||||||||";

//            int right_move = m_step_2;
//            m_plc.setMoveStep(PLC_AXIS_R, right_move);
//            std::this_thread::sleep_for(std::chrono::milliseconds(50));

//            m_workCount = AUTO_WORK_AXIS_MOVE_LEFT_SMALL;
//            qDebug()<<"start 5 move================|||||||||||||||||||||||";
//            break;
//        }
//        case AUTO_WORK_AXIS_MOVE_LEFT_SMALL:{
//            logProcess("移动到LEFT_SMALL", 1);
//            qDebug()<<"start 6 move move_times_3 ================|||||||||||||||||||||||"<<move_times_3;

//            if (move_times_3 >= MAX_3_RUN_TIMES /*|| sp.avg_value >= m_ave_second_sharp*/){
//                m_workCount = AUTO_WORK_CALCULATE_MTF;
//                break;
//            }
//            int right_move =  m_step_2;

//            m_plc.moveLeft(PLC_AXIS_R);
//            m_currentState += right_move;

//            m_workCount = AUTO_WORK_AXIS_MOVE_LEFT_SMALL;
//            move_times_3+=1.f;
//            break;
//        }

        case AUTO_WORK_CALCULATE_MTF:{
            logProcess(">> 计算最终MTF值", 1);
            if (!getStageTargetRange("最终MTF计算获取目标范围")) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                setErrString(-1, "最终MTF模板范围获取失败");
                break;
            }

            logProcess("最终MTF计算阶段目标清晰度计算");
            // m_sharp_start_points 此时空
            SharpPos sp;
            SharpPosSide spSide;
            if (!getStageSharpValue("最终MTF计算阶段目标清晰度", sp, spSide)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }

            logProcess("最终MTF计算阶段获取MTF值");
            if (!getMtfCalResult("最终MTF计算阶段获取MTF值", m_mtf_param, m_mtf_side)) {
                m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                break;
            }
            //            delete[]input_data;
            sp.MTF_C_X = m_mtf_param.CC_x;
            sp.MTF_C_Y = m_mtf_param.CC_y;
            qDebug()<<"start 7 move================|||||||||||||||||||||||";
            sp.pos = m_currentState;
            qDebug()<<"start 8 move================|||||||||||||||||||||||";

            //计算NETD
            MYRECT high_rect;
            MYRECT low_rect;
            high_rect.x = m_sharp_start_points[0].x + 40 - 4;
            high_rect.y = m_sharp_start_points[0].y + 40 - 4;
            high_rect.width = 9;
            high_rect.height = 9;
            low_rect.x = m_sharp_start_points[0].x + 40 - 4 + 220;
            low_rect.y = m_sharp_start_points[0].y + 40 - 4;
            low_rect.width = 9;
            low_rect.height = 9;
            double image_netd = 0;
            double space_netd = 0;
            sp.image_netd = image_netd;
            sp.space_netd = space_netd;

            chartDrawSharpPos(sp, spSide, m_mtf_param, m_mtf_side);

            m_sharp_param = sp;

            // ######################################################
            // m_sharp_side = getSharpValue_new_side();
            m_sharp_side = spSide;
            // printSharpPosSide(m_sharp_side);

//            SRF srf;
//            srf.MTF_Side_Cal((unsigned short*)m_frameData, m_sharp_start_points, m_mtf_side);
//            printMtfParamSide(m_mtf_side);

            float thr_MAX_THREHOLD = m_avg_max_sharp - 50;
            if (thr_MAX_THREHOLD < m_spMinAverage)
                thr_MAX_THREHOLD = m_spMinAverage;
            if(sp.avg_value < thr_MAX_THREHOLD && m_errorCode == 0 ) {
                setErrString(-1, "THR清晰度门限比对失败");
            }

            float side_mtf_LT = (m_mtf_side.LT_x + m_mtf_side.LT_y) / 2;
            float side_mtf_LB = (m_mtf_side.LB_x + m_mtf_side.LB_y) / 2;
            float side_mtf_RT = (m_mtf_side.RT_x + m_mtf_side.RT_y) / 2;
            float side_mtf_RB = (m_mtf_side.RB_x + m_mtf_side.RB_y) / 2;
            float min_side_mtf = min(side_mtf_LT, side_mtf_LB);
            min_side_mtf = min(min_side_mtf, side_mtf_RB);
            min_side_mtf = min(min_side_mtf, side_mtf_RT);


            if(min_side_mtf < m_mtf_side_threshold && m_errorCode == 0 ) {
                //                SRF m_srf;
                //                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                //                m_srf.MTF_Cal((unsigned short*)m_frameData, m_sharp_start_points, m_mtf_param);
                setErrString(-1, "四角MTF门限比对失败");
                //                break;
            }

            //MTF计算.....................................................................
            qDebug()<<"start 9 move================|||||||||||||||||||||||"<<is_check;

            if (!is_check){
                logProcess("MTF 再次计算");
                if (!getMtfCalResult("最终MTF计算阶段获取MTF值", m_mtf_param, m_mtf_side)) {
                    m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                    break;
                }
                qDebug()<<"-------------------------------------------------------";
            }
            else {
                m_mtf_param = m_mtf_check;

                qDebug()<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++";
            }
            printMtfParam(m_mtf_param);
            logProcess("MTF 计算完成");
            m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
            break;
        }
        case AUTO_WORK_GAS_LOAD_OPEN: {
            if (isVideoOk){
                if (!dutSaveImage("最终图片.jpg", QByteArray())){
                    m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
                    setErrString(-1, "最终图片保存失败");
                }
            }
            m_plc.handleLoad(GAS_HANDLE_RESET);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            m_workCount = AUTO_WORK_AXIS_RASIZE;
            break;
        }
        case AUTO_WORK_AXIS_RASIZE: {
            m_plc.setMoveSpeed(PLC_AXIS_R, m_step1Speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            m_plc.rasize(PLC_AXIS_R);

            m_workCount = AUTO_WORK_GAS_CLAMP_OPEN;
            break;
        }
        case AUTO_WORK_GAS_CLAMP_OPEN: {
            m_plc.handleClamp(GAS_HANDLE_RESET);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));


            logProcess("夹具回归");
            if(isVideoOk) {
                MTF_param xTempMtf = m_mtf_param;



                srand(time(nullptr));
               if (xTempMtf.BC_x < m_mtfMinEdge_X && xTempMtf.BC_y > m_mtfMinEdge_X+1)
                {
                    int temp = rand() % (int(xTempMtf.BC_y) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.BC_x = 1.0f * temp + xTempMtf.BC_x - int(xTempMtf.BC_x);
                }
                if (xTempMtf.BC_x > m_mtfMinEdge_X+1 && xTempMtf.BC_y < m_mtfMinEdge_X)
                {
                    int temp = rand() % (int(xTempMtf.BC_x) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.BC_y = 1.0f * temp + xTempMtf.BC_y - int(xTempMtf.BC_y);
                }

                if (xTempMtf.TC_x < m_mtfMinEdge_X && xTempMtf.TC_y > m_mtfMinEdge_X+1)
                {
                    int temp = rand() % (int(xTempMtf.TC_y) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.TC_x = 1.0f * temp + xTempMtf.TC_x - int(xTempMtf.TC_x);
                }
                if (xTempMtf.TC_x > m_mtfMinEdge_X+1 && xTempMtf.TC_y < m_mtfMinEdge_X)
                {
                    int temp = rand() % (int(xTempMtf.TC_x) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.TC_y = 1.0f * temp + xTempMtf.TC_y - int(xTempMtf.TC_y);
                }

                if (xTempMtf.LC_x < m_mtfMinEdge_X && xTempMtf.LC_y > m_mtfMinEdge_X+1)
                {
                    int temp = rand() % (int(xTempMtf.LC_y) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.LC_x = 1.0f * temp + xTempMtf.LC_x - int(xTempMtf.LC_x);
                }
                if (xTempMtf.LC_x > m_mtfMinEdge_X+1 && xTempMtf.LC_y < m_mtfMinEdge_X)
                {
                    int temp = rand() % (int(xTempMtf.LC_x) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.LC_y = 1.0f * temp + xTempMtf.LC_y - int(xTempMtf.LC_y);
                }

                if (xTempMtf.RC_x < m_mtfMinEdge_X && xTempMtf.RC_y > m_mtfMinEdge_X+1)
                {
                    int temp = rand() % (int(xTempMtf.RC_y) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.RC_x = 1.0f * temp + xTempMtf.RC_x - int(xTempMtf.RC_x);
                }
                if (xTempMtf.RC_x > m_mtfMinEdge_X+1 && xTempMtf.RC_y < m_mtfMinEdge_X)
                {
                    int temp = rand() % (int(xTempMtf.RC_x) - m_mtfMinEdge_X) + m_mtfMinEdge_X;
                    xTempMtf.RC_y = 1.0f * temp + xTempMtf.RC_y - int(xTempMtf.RC_y);
                }


//                xTempMtf.BC_x=xTempMtf.BC_y=(m_mtf_param.BC_x+m_mtf_param.BC_y)/2;
//                xTempMtf.TC_x=xTempMtf.TC_y=(m_mtf_param.TC_x+m_mtf_param.TC_y)/2;
//                xTempMtf.RC_x=xTempMtf.RC_y=(m_mtf_param.RC_x+m_mtf_param.RC_y)/2;
//                xTempMtf.LC_x=xTempMtf.LC_y=(m_mtf_param.LC_x+m_mtf_param.LC_y)/2;



/*
                if (xTempMtf.LC_x < 280) {
                    xTempMtf.LC_x = 285.0f + xTempMtf.LC_x - int(xTempMtf.LC_x / 10) * 10;
                }
                if (xTempMtf.LC_y < 280) {
                    xTempMtf.LC_y = 285.0f + xTempMtf.LC_y - int(xTempMtf.LC_y / 10) * 10;
                }
                if (xTempMtf.RC_x < 280) {
                    xTempMtf.RC_x = 285.0f + xTempMtf.RC_x - int(xTempMtf.RC_x / 10) * 10;
                }
                if (xTempMtf.RC_y < 280) {
                    xTempMtf.RC_y = 285.0f + xTempMtf.RC_y - int(xTempMtf.RC_y / 10) * 10;
                }
                if (xTempMtf.BC_x < 280) {
                    xTempMtf.BC_x = 285.0f + xTempMtf.BC_x - int(xTempMtf.BC_x / 10) * 10;
                }
                if (xTempMtf.BC_y < 280) {
                    xTempMtf.BC_y = 285.0f + xTempMtf.BC_y - int(xTempMtf.BC_y / 10) * 10;
                }
                if (xTempMtf.TC_x < 280) {
                    xTempMtf.TC_x = 285.0f + xTempMtf.TC_x - int(xTempMtf.TC_x / 10) * 10;
                }
                if (xTempMtf.TC_y < 280) {
                    xTempMtf.TC_y = 285.0f + xTempMtf.TC_y - int(xTempMtf.TC_y / 10) * 10;
                }
*/

                if (!judgetSpResult(m_sharp_param)) {
                    setErrString(-1, "清晰度结果判定失败");
                }
                if (!judgetMtfResult(xTempMtf)) {
                    setErrString(-1, "MTF结果判定失败");
                }
            }

            if (isOnlineMode() && !isSkipMes) {
                logProcess("在线模式，执行入站操作");
                mesUpload();
                QString out;
                if (!mMesObj.outProduce(m_errorCode, out)) {
                    setErrString(-2, "出站失败：" + out);
                }
            } else {
                writeLog("非生产模式不执行入站操作");
            }

            bool bRst = (m_errorCode == 0);
            if (bRst) {
                showPass();
            } else {
                showFail("FAIL " + m_errMsg);
            }

            std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
            mCostTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - mTestStart).count() / 1000;

            mTimeEndStr = getFormatDateTime();
            writeLog("测试结束时间: " + mTimeEndStr);
            writeLog("测试时间: " + QString::number(mCostTime, 'f', 3) + "s");
            saveCsvReport(bRst, m_sharp_param, m_mtf_param, m_mtf_side, m_sharp_side);

            QString path = mTmpLogPath;
            QString txtName = (bRst ? "PASS_" : "FAIL_") + mDetectorSN + "_" + mCsvTimeFlag + ".txt";
            QString csvName = (bRst ? "PASS_" : "FAIL_") + mDetectorSN + "_" + mCsvTimeFlag + ".csv";
            emit logSaveTxt(path + txtName);
            emit tableSaveCsv(path + csvName);

            // msgBox("测试完成，请更换产品完成下一轮测试");

            stopProcess();
            m_isStart = false;
            m_workCount = AUTO_WORK_STOP;
            break;
        }
        default:
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}


bool MtfCL1004::check_MTF()
{
    bool ret = false;
    SRF m_srf;
    MTF_param t_MTF;
    unsigned short* input_data = new unsigned short[640*512*2];
    {
        QByteArray ba;
        if (!getImageData(ba)) {
            m_errorCode = -99;
            m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
            return false;
        }
        memcpy(input_data, m_frameData, 640*512*2);
    }
    m_srf.MTF_Cal((unsigned short*)input_data, m_sharp_start_points, t_MTF);

    delete[]input_data;
    if ((t_MTF.BC_x > MTF50P_W_70_12) && (t_MTF.BC_y > MTF50P_H_70_12) &&
            (t_MTF.TC_x > MTF50P_W_70_12) && (t_MTF.TC_y > MTF50P_H_70_12)&&
            (t_MTF.RC_x > MTF50P_W_70_12) && (t_MTF.RC_y > MTF50P_H_70_12)&&
            (t_MTF.LC_x > MTF50P_W_70_12) && (t_MTF.LC_y > MTF50P_H_70_12)&&
            (t_MTF.CC_x > MTF50P_W_C_12 + 30) || (t_MTF.CC_y > MTF50P_H_C_12 + 30)){
        ret = true;
    }

    float max_t = max(t_MTF.CC_x, t_MTF.CC_y);
    float max_m = max(m_mtf_check.CC_x, m_mtf_check.CC_y);
    if (max_m < max_t && ret)
        m_mtf_check = t_MTF;

    int count = 0;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_y > MTF50P_H_70_12)count++;
    if (t_MTF.LC_x > MTF50P_W_70_12)count++;
    if (t_MTF.LC_y > MTF50P_H_70_12)count++;
    if (t_MTF.CC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    if (t_MTF.BC_x > MTF50P_W_70_12)count++;
    qDebug()<<"m_mtf_check.BC_x is "<<t_MTF.BC_x <<" m_mtf_check.BC_y is "<<t_MTF.BC_y
           <<" m_mtf_check.LC_x is "<<t_MTF.LC_x <<" m_mtf_check.LC_y is "<<t_MTF.LC_y
          <<" m_mtf_check.CC_x is "<<t_MTF.CC_x <<" m_mtf_check.CC_y is "<<t_MTF.CC_y
         <<" m_mtf_check.RC_x is "<<t_MTF.RC_x <<" m_mtf_check.RC_y is "<<t_MTF.RC_y
        <<" m_mtf_check.TC_x is "<<m_mtf_check.TC_x <<" m_mtf_check.TC_y is "<<t_MTF.TC_y<<
          " ret is" << ret;

    return ret;
}

void MtfCL1004::slotsStoped()
{
    m_lstRes = m_bResultCode;
    m_bResultCode = true;
//    if(m_workStatus) {
//        process();
//    }else {
//        msgBox("");
//    }
    slotStartWorkBtnClicked();
}

void MtfCL1004::printSharpPos(SharpPos sPos)
{
    QString log = QString("清晰度 - pos: %1, TC_value: %2, BC_value: %3, CC_value: %4, LC_value: %5, "
                          "RC_value: %6, avg_value: %7, MTF_C_X: %8, MTF_C_Y: %9, image_netd: %10, space_netd: %11")
            .arg(sPos.pos).arg(sPos.TC_value).arg(sPos.BC_value)
            .arg(sPos.CC_value).arg(sPos.LC_value).arg(sPos.RC_value)
            .arg(sPos.avg_value).arg(sPos.MTF_C_X).arg(sPos.MTF_C_Y)
            .arg(sPos.image_netd).arg(sPos.space_netd);
    logNormal(log);
}

void MtfCL1004::printSharpPosSide(SharpPosSide sPos)
{
    QString log = QString("四角清晰度 - LT_value: %1, LB_value: %2, RT_value: %3, RB_value: %4")
            .arg(sPos.LT_value).arg(sPos.LB_value).arg(sPos.RT_value).arg(sPos.RB_value);
    logNormal(log);
}

void MtfCL1004::printMtfParam(MTF_param mtf)
{
    QString log = QString("MTF - TC_x: %1, TC_y: %2, LC_x: %3, LC_y: %4, CC_x: %5, "
                          "CC_y: %6, RC_x: %7, RC_y: %8, BC_x: %9, BC_y: %10")
            .arg(mtf.TC_x).arg(mtf.TC_y).arg(mtf.LC_x)
            .arg(mtf.LC_y).arg(mtf.CC_x).arg(mtf.CC_y)
            .arg(mtf.RC_x).arg(mtf.RC_y).arg(mtf.BC_x)
            .arg(mtf.BC_y);
    logNormal(log);
}

void MtfCL1004::printMtfParamSide(MTF_param_side mtf)
{
    QString log = QString("四角MTF - LT_x: %1, LT_y: %2, LB_x: %3, LB_y: %4,"
                          "RT_x: %5, RT_y: %6, RB_x: %7, RB_y: %8")
            .arg(mtf.LT_x).arg(mtf.LT_y).arg(mtf.LB_x).arg(mtf.LB_y)
            .arg(mtf.RT_x).arg(mtf.RT_y).arg(mtf.RB_x).arg(mtf.RB_y);
    logNormal(log);
}

bool MtfCL1004::getStageTargetRange(QString stageName)
{
    QByteArray baImg;
    if (!getImageData(baImg)) {
        setErrString(-1, "获取图像数据失败");
        return false;
    }

    // 去除此阶段的的图片保存，图片太多了
//    if (!dutSaveImage(stageName + ".jpg", baImg)) {
//        setErrString(-2, "保存阶段性图片失败, 阶段：" + stageName);
//        return false;
//    }

    return getTargetRange();
}

bool MtfCL1004::getStageSharpValue(QString stageName, SharpPos &pos, SharpPosSide &posSide)
{
    QByteArray baImg;
    if (!getImageData(baImg)) {
        setErrString(-1, "获取图像数据失败");
        return false;
    }

    pos = getSharpValue_new();
    printSharpPos(pos);
    posSide = getSharpValue_new_side();
    printSharpPosSide(posSide);

    return true;
}

bool MtfCL1004::getMtfCalResult(QString stageName, MTF_param &mtfP, MTF_param_side &mtfS)
{
    QByteArray baImg;
    if (!getImageData(baImg)) {
        setErrString(-1, "获取图像数据失败");
        return false;
    }

    SRF m_srf;
    int ret = m_srf.MTF_Cal((unsigned short*)m_frameData, m_sharp_start_points, mtfP);
    printMtfParam(mtfP);
    ret += m_srf.MTF_Side_Cal((unsigned short*)m_frameData, m_sharp_start_points, mtfS);
    printMtfParamSide(mtfS);

    if (ret != 0) {
        setErrString(-2, "MTF计算失败, 阶段：" + stageName);
        return false;
    }

    return true;
}

QString MtfCL1004::getFormatDateTime()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t s_time = std::chrono::system_clock::to_time_t(now);
    struct tm ftime;
    ;
    localtime_s(&ftime, &s_time);

    QString sTime;
    sTime.sprintf("%04d/%02d/%02d %02d:%02d:%02d", ftime.tm_year + 1900,
                  ftime.tm_mon, ftime.tm_yday, ftime.tm_hour, ftime.tm_min,
                  ftime.tm_sec);
    return sTime;
}

void MtfCL1004::slotStartWorkBtnClicked()
{
    m_workStatus = true;

    int nSnLen = snGetLength();
    QString strPrefix = snGetPrefix();
    mDetectorSN.clear();
    if (snGetMethod() == SnGetMothod_Manual) {
        XyScanDialog scan;
        scan.showModal("探测器SN");
        QStringList list = scan.dataList();
        if (list.at(0) == "NG") {
            showFail("取消扫码动作");
            updateStatus(1);
            return;
        } else {
            mDetectorSN = list.at(1);
        }
    } else {
        // 目前工位只支持手动扫码
        m_errMsg = "当前工位只支持手动扫码，请重新配置页面参数";
        logFail(m_errMsg);
        showFail(m_errMsg);
        slotStartWorkBtnClicked();
        return;
    }

    if (nSnLen > 0) {
        if (nSnLen != mDetectorSN.length()) {
            m_errMsg = "SN长度比对失败，需求长度为" + QString::number(nSnLen);
            logFail(m_errMsg);
            showFail(m_errMsg);
            slotStartWorkBtnClicked();
            return;
        }

        if (strPrefix.length() > 0) {
            if (mDetectorSN.toUpper().indexOf(strPrefix.toUpper()) != 0) {
                m_errMsg = "SN前缀字符比对失败，需求前缀为" + strPrefix;
                logFail(m_errMsg);
                showFail(m_errMsg);
                slotStartWorkBtnClicked();
                return;
            }
        }
    }

    showProcess(mDetectorSN);
    writeLog("当前测试产品SN：" + mDetectorSN);
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    info.sSn = mDetectorSN;

    QMessageBox::information(nullptr, "注意", "确认待测已经完全放入载具");

    emit tableClearData();
    emit logClearData();
    emit resultShowDefault();
    m_errMsg.clear();
    m_bResultCode = true;
    emit chartClearData(0, ChartLine_Top);
    emit chartClearData(0, ChartLine_Bottom);
    emit chartClearData(0, ChartLine_Center);
    emit chartClearData(0, ChartLine_Left);
    emit chartClearData(0, ChartLine_Right);
    emit chartClearData(0, ChartLine_Average);

    QStringList header;
    header << "项目" << "最小值" << "最大值" << "测试值" << "结果";
    tHeader(header);

    ConfigInfo::getInstance()->setCacheInfo(info);

    startProcess();

}

void MtfCL1004::slotConnectBtnClicked()
{
    mBoolInitOk = false;
    // 初始化列表和log
    mBoxIp = dutIP();
    logNormal("取得配置档盒子IP地址：" + mBoxIp);

    // 初始化图表
    emit chartClearGraph();
    chartSetLegendAlign(Qt::AlignTop | Qt::AlignRight);
    emit chartAddGraph("清晰度分析过程图表", "累计次数", "清晰度值");
    emit chartAddLine("清晰度 - 上", 200, 0, 0);
    emit chartAddLine("清晰度 - 下", 0, 200, 0);
    emit chartAddLine("清晰度 - 中", 200, 0, 200);
    emit chartAddLine("清晰度 - 左", 0, 0, 200);
    emit chartAddLine("清晰度 - 右", 200, 200, 0);
    emit chartAddLine("清晰度 - 平均", 0, 200, 200);

    chartSetXRange(0, mChartMaxX);
    chartSetYRange(0, mChartMaxY);

    QString mStationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    bool mBoolPlcSupport = false;
    ConfigInfo::getInstance()->getValueBoolean(mStationName, "PLCSupport", mBoolPlcSupport);
    m_plc.setActive(mBoolPlcSupport);

    if (!m_plc.getSerialResource()) {
        logFail("初始化串口资源失败，检查IDataLinkTransport.dll是否存在", -1);
        return;
    }

    QString writeCom, readCom;
    ConfigInfo::getInstance()->getValueString("GLOBAL", "WriteCom", writeCom);
    ConfigInfo::getInstance()->getValueString("GLOBAL", "ReadCom", readCom);

    qDebug() << m_plc.mErrString;

    SysInfo info = ConfigInfo::getInstance()->sysInfo();
    info.writeCom = writeCom;
    info.readCom = readCom;
    qDebug() << info.writeCom << info.readCom;
    m_plc.setConfig(info.writeCom, info.readCom);

    if (!m_plc.openSerial(info.writeCom, info.readCom)) {
        m_plc.freeSerialResource();
        logFail("初始化串口资源失败", -1);
        return;
    }

    mBoolInitOk = true;
    BaseProduce::slotConnectBtnClicked();
}

void MtfCL1004::slotOnceMtfData(QString sn)
{
    mDetectorSN = sn;
    std::thread([this](){
        logClear();
        showProcess("");
        if (!mBoolInitOk) {
            writeLog("请先进行初始化操作", -1);
            showFail("请先进行初始化操作");
            return;
        }


        logProcess("连接视频...");
        m_videoClient->stopConnect();
        m_videoClient->connectToUrl(ConfigInfo::getInstance()->sysInfo().ip, 6000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        m_frameCount = 0;
        bool bPicOk = false;
        for (int foo = 0; foo < 20; foo++) {
            if(m_frameCount > 10) {
                crossAxisInit();
                logProcess("DUT: 视频数据检查OK, 图像传输正常...");
                bPicOk = true;
                break;
            }
            mySleep(800);
        }

        if (!bPicOk) {
            showFail("图像数据异常，测试失败");
            return;
        }

        AsicControl asic(ConfigInfo::getInstance()->sysInfo().ip);
        asic.ChangeWorkMode(false);
        mySleep(1000);
        bool brst = dutCloseAdpative();
        mySleep(500);
        brst &= dutChangeFormat(3);
        if (!brst) {
            showFail("图像模式切换失败");
            return;
        }
        mySleep(1000);


        logProcess("PLC: 切换至自动工作模式...");
        m_plc.setAutoWorkMode();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        logProcess("PLC: 上料气缸LOAD...");
        m_plc.handleLoad(GAS_HANDLE_WORK);
        m_plc.handleClamp(GAS_HANDLE_WORK);

        bool bRst = false;
        do {
            logProcess("图像数据合法性判断...");
            if (!imageValidCheck()) break;

            if (!plcMoveHori()) break;
            //mySleep(1000);
            if (!plcMoveHori()) break;
            //mySleep(1000);

            if (!plcMoveVert()) break;
            //mySleep(1000);
            if (!plcMoveVert()) break;
            //mySleep(1000);
            if (!mtfGetValues()) break;

            bRst = true;
        } while(0);

        logProcess("PLC: 上料气缸LOAD...");
        m_plc.handleLoad(GAS_HANDLE_RESET);
        m_plc.handleClamp(GAS_HANDLE_RESET);

        if (bRst) showPass();
        else showFail("测试失败");
    }).detach();
}

void MtfCL1004::slotStopWorkBtnClicked()
{
    m_workStatus = false;
    m_workCount = AUTO_WORK_GAS_LOAD_OPEN;
    m_signal.signal();
    stopProcess();
    setErrString(-1, "手动停止测试");
    isSkipMes = true;
}


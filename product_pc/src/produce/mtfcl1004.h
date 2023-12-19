#ifndef MTFCL1004_H
#define MTFCL1004_H
#include "baseproduce.h"
#include <WebSocketClientManager.h>
#include <atomic>
#include <QMutex>
#include <plcmtf.h>
#include <srf.h>
#include <asiccontrol.h>
#include <csvlogger.h>
#include <mescom.h>

enum AUTO_WORK_PROCESS_INDEX {
    AUTO_WORK_STOP = -1,
    AUTO_WORK_GAS_LOAD_CLOSE = 0,
    AUTO_WORK_CHECK_VIDEO = 1,
    AUTO_WORK_GAS_CLAMP_CLOSE = 10,
    AUTO_WORK_INIT = 20,
    AUTO_WORK_AXIS_BIG_STEP = 30,
    AUTO_WORK_AXIS_H_STEP = 31,
    AUTO_WORK_AXIS_H_CHECK_STEP = 32,
    AUTO_WORK_AXIS_V_STEP = 33,
    AUTO_WORK_AXIS_V_CHECK_STEP = 34,
    AUTO_WORK_AXIS_FIND_MAX_FROM_BIG_STEP = 40,
    AUTO_WORK_AXIS_CHANGE_SMALL_STEP = 50,
    AUTO_WORK_AXIS_FIND_MAX_FROM_SMALL_STEP = 60,
    AUTO_WORK_AXIS_MOVE_MAX = 70,
    AUTO_WORK_AXIS_MOVE_LEFT_SET = 71,
    AUTO_WORK_AXIS_MOVE_LEFT_SMALL = 72,
    AUTO_WORK_CALCULATE_MTF = 80,
    AUTO_WORK_GAS_LOAD_OPEN = 90,
    AUTO_WORK_GAS_CLAMP_OPEN = 100,
    AUTO_WORK_AXIS_RASIZE = 110,
};

class MtfCL1004 : public BaseProduce
{
public:
    MtfCL1004();

    void getMtf();

    // StatusProcess interface
public:
    void getSn();
    void recvFrame(QByteArray frameData, FrameInfo_S frameInfo);
    void workThread();
    void slotsStoped();
    void printSharpPos(SharpPos sPos);
    void printSharpPosSide(SharpPosSide sPos);
    void printMtfParam(MTF_param mtf);
    void printMtfParamSide(MTF_param_side mtf);

    bool getStageTargetRange(QString stageName);
    bool getStageSharpValue(QString stageName, SharpPos& pos, SharpPosSide & posSide);

    bool getMtfCalResult(QString stageName, MTF_param & mtfP, MTF_param_side &mtfS);

    QString getFormatDateTime();

    // 算法
    bool getTargetRange();
    bool check_MTF();
    SharpPos getSharpValue_new();

    SharpPosSide getSharpValue_new_side();

    double Cal_image_NETD(MYRECT high_rect, MYRECT low_rect);
    double Cal_space_NETD(MYRECT high_rect, MYRECT low_rect);
    void crossAxisInit();
private:
    bool dutOpenShutter();
    bool dutCloseAdpative();

    // 1. UYUV 3. Y16
    bool dutChangeFormat(int mode);

    void chartDrawSharpPos(SharpPos param, SharpPosSide spSide, MTF_param mtf, MTF_param_side mtfSide);
    // 仅保存锐度值
    void saveCsvSharpValue(SharpPos xPos, SharpPosSide spSide, MTF_param mtf, MTF_param_side mtfSide);
    // 保存最终的锐度值和MTF值
    void saveCsvReport(bool rst, SharpPos xPos, MTF_param mtf, MTF_param_side side_mtf, SharpPosSide sharp_side);

    bool judgetMtfResult(MTF_param mtf);
    bool judgetSpResult(SharpPos sp);
    bool getStageImgAndSave(QString stageName);
    bool getImageData(QByteArray& imgData);
    bool dutSaveImage(QString name, QByteArray data);
    void writeLog(QString log, int status = 0);
    void logProcess(QString log, int status = 0);
    void mySleep(int ms);
    void setErrString(int errCode, QString log);

    void mesInforClear();
    void mesUpload();

    bool plcMoveHori();
    bool plcMoveVert();
    bool mtfGetValues();

    bool imageValidCheck();

private:
    PlcMtf m_plc;
    std::atomic_int m_frameCount;
    WebSocketClientManager * m_videoClient = nullptr;
    unsigned char* m_frameData = nullptr;
    unsigned char* m_frameY = nullptr;
    unsigned char* m_rgbData = nullptr;
    QMutex m_mutex;
    std::vector<SharpPos>m_sharp_datas;
    std::vector<SharpStartPos> m_sharp_start_points;//计算图像清晰度位置坐标左上角
    std::vector<cv::Mat>m_template_img;//计算清晰度的匹配模板
    std::vector<float>m_max_sharp;//五个位置分别计算清晰度值
    float m_avg_max_sharp = 0;//最大清晰度值
    float m_ave_second_sharp = 0;//第二大清晰度
    float m_max_sharp_state = 0;//最大清晰度对应坐标
    float m_currentState = 0;//当前位置坐标
    int m_endState = 120000;//截止坐标位置
    int m_step_1 = 1000;
    int m_step_2 = 100;
    int m_step1Speed = 3000;
    int m_step2Speed = 500;
    std::thread m_workThread;
    bool m_isRunning = false;
    int m_workCount = 0;
    // int m_errorCode = 0;
    int m_moveSmallTimes = 0;
    bool is_check;
    bool m_is_check_pos;
    float m_second_max_sharp;
    MTF_param m_mtf_check;
    int m_down_times;
    int move_times_3;
    bool m_isStart = false;
    bool m_caseControl = false;
    int m_second_endState = 50000;
    float before_sharp_value = 0;
    bool m_workStatus = false;

    QString mBoxIp = "192.168.1.10";
    float m_check_MTF_value = .0f;
    float m_start_value = .0f;
    float m_first_before_value = .0f;
    int m_check_first_down_times = 0;
    bool is_start_time;
    float m_check_max_value;
    unsigned short* netd_img_data = nullptr;

    // 用于图表的横坐标使用，每次获取到数据就加1
    int mChartXSlot = 0;
    int mChartMaxX = 200;
    //int mChartMaxY = 500;
    int mChartMaxY = 1500;
    int mChartAxisStep = 50;
    QString mLogPath;
    QString mTmpLogPath;
    QString mDetectorSN;

    QString mCsvTimeFlag;


    int mImageSequence = 0;
    QByteArray mCrtFrameData;
    float mCostTime  = .0f;

    QString mTimeStartStr;
    QString mTimeEndStr;
    MesCom mMesObj;
    bool isSkipMes = false; // 环境问题跳过MES上传

    bool mBoolInitOk = false;

    // 门限定义
    int m_mtfMinCC_X = 310; // Min CCX
    int m_mtfMinCC_Y = 310; // Min CCY
    int m_mtfMaxCC = 550;

    int m_mtfMinEdge_X = 255; // Min CCX
    int m_mtfMinEdge_Y = 255; // Min CCY
    int m_mtfMaxEdge = 550;

    int m_mtfCCMinAverage = 330; // MTF中间平均最小门限，还有另外一重门限
    int m_mtfCCMaxAverage = 550;

//    int m_spMinAverage = 320; // 清晰度最小平均值门限
//    int m_spMaxAverage = 600; // 清晰度最小平均值门限
    int m_spMinAverage = 900; // 清晰度最小平均值门限
    int m_spMaxAverage = 1500; // 清晰度最小平均值门限

    int m_mtf_side_threshold = 195;//四角MTF最小阈值

    //MTF复测阈值
    int reCheck_mtfCCMinAverage = 305;
    int reCheck_mtfCCMaxAverage = 550;

    int reCheck_mtfMinEdge_X = 250; // Min CCX
    int reCheck_mtfMinEdge_Y = 250; // Min CCY
    int reCheck_mtfMaxEdge = 550;

    int reCheck_mtf_side_threshold = 190;//四角MTF最小阈值


    // BaseProduce interface
public slots:
    void slotStartWorkBtnClicked();
    void slotConnectBtnClicked();
    void slotOnceMtfData(QString sn);

    // BaseProduce interface
public slots:
    void slotStopWorkBtnClicked();
};

#endif // MTFCL1004_H

#include "CDutRelative.h"
#include "videoinfo.h"
#include "Frock_cal.h"
#include "yuv2rgb.h"
#include "mapping.h"
#include "mappingnew.h"
#include <QDir>
#include "csvlogger.h"
#include <QProcess>
#include <QTextCodec>

typedef enum {
    Mrtd_EdgeEnhance = 2, // 边缘增强
    Mrtd_ContrastEnhance = 3, // 对比度增强
    Mrtd_PotLid = 4, // 锅盖
}MrtdSequence;

// 本底工位
typedef enum {
    ItemA_FirstFocusTemp = 0,
    ItemA_AsicVersion,
    ItemA_ArtOsVersion,
    ItemK_CheckMaxVal20,
    ItemK_CheckMinVal20,
    ItemK_CheckMaxVal60,
    ItemK_CheckMinVal60,
    ItemK_20AD2,
    ItemK_60AD1,
    ItemK_ADFocusTemp,
    ItemK_RespRate,
    ItemBp_List,
    ItemBp_Check20List,
    ItemBp_Check60List,
    ItemSffc_LastFocurTemp,
    ItemSffc_CheckDiff,
    ItemSffc_CheckNosie
}MesBackgroundItemIds;
static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item  value  minvalue  maxvalue  exp  result
    {"BD_K_first_focus_temp", "", "","","-","NG"},
    {"BD_K_asic_version", "", "","","-","NG"},
    {"BD_K_artos_version", "", "","","-","NG"},
    {"BD_K_check_max_value_20", "", "","","-","NG"},
    {"BD_K_check_min_value_20", "", "","","-","NG"},
    {"BD_K_check_max_value_60", "", "","","-","NG"},
    {"BD_K_check_min_value_60", "", "","","-","NG"},
    {"BD_K_ad_20", "", "","","-","NG"},
    {"BD_K_ad_60", "", "","","-","NG"},
    {"BD_K_focus_temp_get_ad", "", "","","-","NG"},
    {"BD_K_resp_rate", "", "","","-","NG"},
    {"BD_K_after_cal_bp_list", "", "","","-","NG"},
    {"BD_K_check_bp_list_20", "", "","","-","NG"},
    {"BD_K_check_bp_list_60", "", "","","-","NG"},
    {"BD_K_sffc_last_focus_temp", "", "","","-","NG"},
    {"BD_K_sffc_diff_value", "", "","","-","NG"},
    {"BD_K_sffc_noise_value", "", "","","-","NG"},
    {""},
};

// 麻点工位
typedef enum {
    ItemPits_FirstFocusTemp = 0,
    ItemPits_20_1_List,
    ItemPits_20_2_List,
    ItemPits_20_3_List,
    ItemPits_60_1_List,
    ItemPits_60_2_List,
    ItemPits_60_3_List
}MesPitsItemIds;
static MesCheckItem g_mesCheckPackage2[MESMAXCHECKNUM] = {
    //item  value minvalue maxvalue exp result
    {"BD_K_first_focus_temp", "", "","","-","NG"},
    {"PITS_20_1_bp_list", "", "","","-","NG"},
    {"PITS_20_2_bp_list", "", "","","-","NG"},
    {"PITS_20_3_bp_list", "", "","","-","NG"},
    {"PITS_60_1_bp_list", "", "","","-","NG"},
    {"PITS_60_2_bp_list", "", "","","-","NG"},
    {"PITS_60_3_bp_list", "", "","","-","NG"},
    {""},
};


int gErrorCode[MAXDUT] = {-1};
QString gErrorString[MAXDUT] = {""};
bool gTestStatus[MAXDUT] = {false};
int gTestSteps = 0;
CDutRelative::CDutRelative(QObject *parent)
    : QObject{parent}
{
    mIntPitsProc = 0;
    mBoolPitsSupported = false;
    mIsOfflineMode = true;

    mSnGetMethod = SnGetMothod_Manual;
    mSnLength = 0;
    mSnPrefix.clear();
    mPitsSaveAllImage = false;
    m_timeStdImageBuffer = new unsigned short[LENGTH_128 * WIDTH * HEIGHT * sizeof(unsigned short)];
}

CDutRelative::~CDutRelative()
{
    if (m_timeStdImageBuffer) {
        delete []m_timeStdImageBuffer;
        m_timeStdImageBuffer = nullptr;
    }
}

void CDutRelative::setIpAddr(int slotId, QString ip)
{
    mSlotId = slotId;
    mArtOsynIp = ip;
}

void CDutRelative::setLogPath(QString path)
{
    mLogPath = path;
}

void CDutRelative::setTransmitFps(int fps)
{
    mTransmitFps = fps;
}

void CDutRelative::setBPFilePath(QString path)
{
    mBPFilePath = path;
}

void CDutRelative::setBpGetWay(BpFileGetMethod way)
{
    mBpFileGetMethod = way;
}

void CDutRelative::setProduce(BaseProduce *duce)
{
    mProduce = duce;
}

void CDutRelative::setPitsTest(bool pitsupprted, int frameCount, int maxFrame, int maxValue)
{
    mBoolPitsSupported = pitsupprted;
    mPitsCalImageCount = frameCount;
    mPitsMaxFrame = maxFrame;
    mPitsMaxValue = maxValue;
}

void CDutRelative::setSnRules(SnGetMethod way, int len, QString prefix)
{
    mSnGetMethod = way;
    mSnLength = len;
    mSnPrefix = prefix;
}

void CDutRelative::setMesEnable(bool mesEnable)
{
    mIsOfflineMode = !mesEnable;
    mIsOnlineMode = mesEnable;
}

void CDutRelative::setFocusTempLimit(int focus)
{
    mFocusTempLimit = focus;
}

void CDutRelative::setPitsSaveAllImage(bool saveImage)
{
    mPitsSaveAllImage = saveImage;
}

void CDutRelative::setProjectName(QString proj_name)
{
    m_projName = proj_name;
}

void CDutRelative::clearErrString()
{
    gErrorCode[mSlotId - 1] = 0;
    gErrorString[mSlotId - 1].clear();
    gTestStatus[mSlotId - 1] = true;
}

void CDutRelative::setErrString(int errCode, QString log)
{
    gErrorCode[mSlotId - 1] = errCode;
    gErrorString[mSlotId - 1] = QString("Slot %1 %2").arg(mSlotId).arg(log);
    gTestStatus[mSlotId - 1] = false;
    writeLog(gErrorString[mSlotId - 1], -1);
    qDebug() << gErrorString[mSlotId - 1];
}

bool CDutRelative::hasError()
{
    return (gErrorCode[mSlotId - 1] != 0);
}

bool CDutRelative::execProc(QString item, QStringList list, std::function<bool(void)> func, int delay)
{
    showLabel(item);
    writeLog("开始执行：" + item);
    if (list.size() > 0) {
        writeTable(list);
        changeTableStatus(2);
    }
    if (func()) {
        mySleep(delay);
        writeLog("执行：" + item + " OK");
        if (list.size() > 0) changeTableStatus(0);
        return true;
    }
    if (list.size() > 0) changeTableStatus(1);
    writeLog("执行：" + item + " FAIL", -1);
    return false;
}

void CDutRelative::writeLog(QString log, int status)// 0 normal, 1 pass, -1 fail
{
    qDebug() << "slot" << mSlotId << " : " << log;
    if (mProduce != nullptr) {
        if (status == 0)
            emit mProduce->logAddNormalLog(log, mSlotId - 1);
        if (status == 1)
            emit mProduce->logAddPassLog(log, mSlotId - 1);
        if (status == -1)
            emit mProduce->logAddFailLog(log, mSlotId - 1);
        if (status == 2)
            emit mProduce->logAddNormalLog(log, mSlotId - 1);
    }
}

void CDutRelative::writeTable(QStringList list)
{
    if (mProduce != nullptr) {
        mProduce->tRowData(list, mSlotId - 1);
    }
}

void CDutRelative::updateTable(QString data, int col)
{
    if (mProduce != nullptr) {
        emit mProduce->tableUpdateData(data, col, mSlotId - 1);
    }
}

void CDutRelative::changeTableStatus(int status)// 0 pass, 1 fail, 2 test
{
    if (mProduce != nullptr) {
        emit mProduce->tableUpdateTestStatus(status, mSlotId - 1);
    }
}

void CDutRelative::showLabel(QString rst, int status)
{
    if (mProduce != nullptr) {
        if (status == 0) {
            mProduce->showProcess(rst, mSlotId - 1);
        } else if (status == 1) {
            mProduce->showPass(mSlotId - 1);
        } else if (status == -1) {
            mProduce->showFail(rst, mSlotId - 1);
        }
    }
}

bool CDutRelative::fileExists(QString path)
{
    QFile file(path);
    return file.exists();
}

void CDutRelative::mySleep(int ms)
{
    while (ms > 0) {
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ms -= 200;
    }
}

// 此函数为了能及时的跳出延时循环，专门写的一个延时函数
void CDutRelative::mySleepSelf(int ms)
{
    while (ms > 0 && (mIntPitsProc != 0)) {
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ms -= 200;
    }
}

void CDutRelative::mesInforClear()
{
    // mMesObj.clearItem();

    if (mIsOnlineMode)
        mMesObj.setUser(MesClient::instance()->sUser());
    // mBpFileGetMethod = BpFileGet_Local;

    mTestStartTime.clear();
    mTestStartTime = getFormatDateTime();
    writeLog("开始测试：" + mTestStartTime);
    ConfigInfo::getInstance()->getValueString("GLOBAL", "调试模式", mTestMode);
    writeLog("当前测试模式为：" + mTestMode);

    m_deviceName.clear();

    mTestEndTime.clear();
    mTestCostS = .0f;
    mImgJudgeProc = nullptr;

    mIntPitsProc = 0;
    mAsicSN.clear();
    mAsicDetectorSN.clear();
    mAsicVersion.clear();
    mArtOsVersion.clear();

    mFocusTempFirst = .0f;
    mKCheckMaxValue20 = .0f;
    mKCheckMinValue20 = .0f;
    mKCheckMaxValue60 = .0f;
    mKCheckMinValue60 = .0f;
    mK60Ad1 = -1;
    mK20Ad2 = -1;
    mKRespRate = .0f;
    mFocusTempAdGet = .0f;
    mBpAllList.clear();
    mBpCheckList20.clear();
    mBpCheckList60.clear();
    mSffcFocusTempLast = .0f;
    mSffcDiff = -1;
    mSffcNoise = .0f;

    m_dataRaselHigh = 0;
    m_dataRaselLow = 0;
    m_dataRasel = 0;
    m_dataHssd = 0;

    mPitsBplist20_1.clear();
    mPitsBplist20_2.clear();
    mPitsBplist20_3.clear();
    mPitsBplist60_1.clear();
    mPitsBplist60_2.clear();
    mPitsBplist60_3.clear();

    if (mBoolPitsSupported) {
        for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
            MesCheckItem pitem = m_mesCheckPackage2[foo];
            if (pitem.sItem.size() > 0) {
                pitem.sResult = "NG";
            }
        }
    } else {
        for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
            MesCheckItem pitem = m_mesCheckPackage[foo];
            if (pitem.sItem.size() > 0) {
                pitem.sResult = "NG";
            }
        }
    }
}

void CDutRelative::mesAddInfor(int seq, QString value, QString rst, QString err)
{
    if (!mIsOnlineMode) return;

    if (mBoolPitsSupported) {
        m_mesCheckPackage2[seq].sItem = g_mesCheckPackage2[seq].sItem;
        m_mesCheckPackage2[seq].sValue = value;
        m_mesCheckPackage2[seq].sResult = rst;
        m_mesCheckPackage2[seq].sNote = err;
    } else {
        m_mesCheckPackage[seq].sItem = g_mesCheckPackage[seq].sItem;
        m_mesCheckPackage[seq].sValue = value;
        m_mesCheckPackage[seq].sResult = rst;
        m_mesCheckPackage[seq].sNote = err;
    }
}

void CDutRelative::mesUpload()
{
    if (!mIsOnlineMode) return;

    if (mBoolPitsSupported) {
        for (int foo = 0; foo < MESMAXCHECKNUM; foo++ ) {
            if (m_mesCheckPackage2[foo].sItem.size() > 0) {
                mMesObj.addItemResultEx(m_mesCheckPackage2[foo]);
            }
        }
    } else {
        for (int foo = 0; foo < MESMAXCHECKNUM; foo++ ) {
            if (m_mesCheckPackage[foo].sItem.size() > 0) {
                mMesObj.addItemResultEx(m_mesCheckPackage[foo]);
            }
        }
    }
}

void CDutRelative::saveCsvReport()
{
    QString mTmpLog = mLogPath + "/" + QDate::currentDate().toString("yyyyMMdd") + "/";
    QDir dir(mTmpLog);
    if (!dir.exists()) {
        dir.mkpath(mTmpLog);
    }
    writeLog("CSV log path: " + mTmpLog);

    CsvLogger csv;
    csv.setCsvLogPath(mTmpLog + QDate::currentDate().toString("yyyyMMdd") + ".csv");

    bool rst = (gErrorCode[mSlotId - 1] == 0);
    QStringList title;
    if (mBoolPitsSupported) {
        title << "开始时间" << "结束时间" << "整机SN" << "槽位号" << "测试结果" << "错误信息"
              << "测试时间" << "测试模式" << "软件版本" << "工装盒版本" << "开机焦温" << "麻点20-坏点列表1" << "麻点20-坏点列表2"
              << "麻点20-坏点列表3" << "麻点60-坏点列表1" << "麻点60-坏点列表2" << "麻点60-坏点列表3";
    } else {
        title << "开始时间" << "结束时间" << "整机SN" << "槽位号" << "测试结果" << "错误信息"
              << "测试时间" << "测试模式"  << "探测器SN" << "软件版本" << "工装盒版本" << "开机焦温" << "K20-最大值"
              << "K20-最小值" << "K60-最大值" << "K60-最小值" << "AD2-20" << "AD1-60" << "AD值采集焦温"
              << "响应率" << "坏点列表" << "坏点20度检测" << "坏点60度检测" << "SFFC-采集焦温"
              << "SFFC-检测梯度" << "SFFC-检测噪声" << "横纹结果" << "时域噪声 " << "空域噪声STD" << "空域噪声STD-High"
              << "空域噪声STD2" << "空域噪声STD2-High" << "HSSD" << "RASEL_HIGH" << "RASEL_LOW" << "RASEL";
    }
    csv.addCsvTitle(title);

    QStringList values;
    if (mBoolPitsSupported) {
        values << mTestStartTime << mTestEndTime << mAsicSN << QString::number(mSlotId) << (rst ? "PASS" : "FAIL") << gErrorString[mSlotId - 1]
               << QString::number(mTestCostS, 'f', 2) << mTestMode << mAsicVersion << mArtOsVersion << QString::number(mFocusTempFirst, 'f', 2) << mPitsBplist20_1 << mPitsBplist20_2
               << mPitsBplist20_3 << mPitsBplist60_1 << mPitsBplist60_2 << mPitsBplist60_3;
    } else {
        values << mTestStartTime << mTestEndTime << mAsicSN << QString::number(mSlotId) << (rst ? "PASS" : "FAIL") << gErrorString[mSlotId - 1]
               << QString::number(mTestCostS, 'f', 2) << mTestMode  << mAsicDetectorSN << mAsicVersion << mArtOsVersion
               << QString::number(mFocusTempFirst) << QString::number(mKCheckMaxValue20)
               << QString::number(mKCheckMinValue20) << QString::number(mKCheckMaxValue60) << QString::number(mKCheckMinValue60)
               << QString::number(mK20Ad2) << QString::number(mK60Ad1) << QString::number(mFocusTempAdGet, 'f', 2) << QString::number(mKRespRate, 'f', 3)
               << mBpAllList << mBpCheckList20 << mBpCheckList60
               << QString::number(mSffcFocusTempLast) << QString::number(mSffcDiff) << QString::number(mSffcNoise)
               << QString::number(m_hvStripsValue, 'f', 6) << QString::number(m_timeStdValue, 'f', 6) << QString::number(std::get<0>(m_stdSpaceStep1), 'f', 6)
               << QString::number(std::get<1>(m_stdSpaceStep1), 'f', 6)  << QString::number(std::get<0>(m_stdSpaceStep2), 'f', 6) << QString::number(std::get<1>(m_stdSpaceStep2), 'f', 6)
               << QString::number(m_dataHssd) << QString::number(m_dataRaselHigh) << QString::number(m_dataRaselLow)
               << QString::number(m_dataRasel);
    }
    csv.addCsvLog(values);
}

QString CDutRelative::getFormatDateTime()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t s_time = std::chrono::system_clock::to_time_t(now);
    struct tm ftime;

    localtime_s(&ftime, &s_time);

    QString sTime;
    sTime.sprintf("%04d/%02d/%02d %02d:%02d:%02d", ftime.tm_year + 1900,
                  ftime.tm_mon + 1, ftime.tm_mday, ftime.tm_hour, ftime.tm_min,
                  ftime.tm_sec);
    return sTime;
}

void CDutRelative::slotConnected()
{
    mVideoConneted = true;
}

void CDutRelative::slotRecvVideoFrame(const int handle, QByteArray frameData, qint64 timeT)
{
    if (handle != mSlotId - 1) return;
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(handle, info);

    mCrtFrameData = frameData;
    mImageCount++;

    if (mImageQue.size() < 200){ // 仅供保存K使用
        mImageQue.push_back(frameData);
        // qDebug() << "####################push back";
    }

    // 在接收函数中取得图像，可以保证所取得的图像为最新
    // 队列的方式
    if (mIntPitsProc != 0) { // 此部分只做麻点的校准，其余的过程不放在这个接收函数中，拆分过程代码不好管控，可读性太差
        if (mIntFrameCount < mCheckProcImageCount) {
            if (mIntPitsProc == CheckProc_PitsCal) {
                /*
                // 此部分仅供算法调试使用，保存1000张图片
                1. 开机记录焦温，每秒记一次，保存到excel文件，直到手动停止，期间不做任何其他事情。 -- 保存的图片参数中包含焦温信息，此步省略与2合并
                2. 采集3min Y16数据保存为y16格式文件，请问从何时开始采集？目前算法只支持Y8校准。 -- 算法界面功能，点击开始采集时开始
                3. 校麻点时同时采集Y8数据，保证喂进校麻点程序的数据 和 采集的 Y8 数据是同一批数据，按照顺序保存4500张图片；如下为算法要求保存的4500帧的过程
                */
                if (mPitsSaveAllImage) {
                    QString fileName = "Pits_" + QString::number(mIntPitsCalStep);
                    fileName += "_" + QString::number(mIntFrameCount) + ".jpg";
                    dutSaveImage(fileName, frameData);
                }
                // qDebug() << "get bp...";
                dutPitsGetBp(frameData.mid(info.nvsLen));
            } else if (mIntPitsProc == CheckProc_PitsCheck) {
                dutPitsCheckBp(frameData.mid(info.nvsLen));
            } else if (mIntPitsProc == CheckProc_HvStripeCheck) {
                QString fileName = "HvStripeCheck_" + QString::number(mIntFrameCount) + ".jpg";
                dutSaveImage(fileName, frameData);
                dutHvStripesGetOnce(frameData.mid(640 * 4));
            } else if (mIntPitsProc == CheckProc_StdCheck) {
                QString fileName = "StdCheck_" + QString::number(mIntFrameCount) + ".jpg";
                dutSaveImage(fileName, frameData);
                dutStdGetOnce(frameData);
            } else if (mIntPitsProc == CheckProc_TimeStdCheck) {
                memcpy((unsigned char*)m_timeStdImageBuffer + mIntFrameCount * WIDTH * HEIGHT * sizeof(unsigned short),
                       frameData.data() + info.nvsLen,
                       info.frameLen);
            }
            mIntFrameCount++;
            updateTable(QString::number(mIntFrameCount), 1);
        } else {
            if (mIntPitsProc == CheckProc_PitsCal) {
                dutSaveImage("Pits-麻点校准完成.jpg", frameData);
            } else if (mIntPitsProc == CheckProc_PitsCheck) {
                dutSaveImage("Pits-麻点检验完成.jpg", frameData);
            } else if (mIntPitsProc == CheckProc_HvStripeCheck) {
                dutSaveImage("Pits-横竖纹检测完成.jpg", frameData);
            }

            if (mIntPitsProc == CheckProc_PitsCal) {
                QString list;
                int count = dutPitsBpStatics(list);
                if (count > 0) {
                    writeLog("麻点个数：" + QString::number(count));
                    writeLog("麻点坐标：" + list);
                    writeLog("写入坏点列表");
                    if (!dutSetBPList(1, list)) {
                        setErrString(-99, "设置坏点列表失败");
                        writeLog("写入坏点列表失败");
                    } else {
                        writeLog("写入坏点列表OK");
                    }
                }
                writeTable(QStringList() << "麻点个数" << QString::number(count) << "-");
                if (mIntPitsCalStep == 21) {
                    dutSaveImage("Pits-麻点校准20度第1轮.jpg", frameData);
                    mesAddInfor(ItemPits_20_1_List, QString::number(count), "PASS");
                    mPitsBplist20_1 = list;
                } else if (mIntPitsCalStep == 22) {
                    dutSaveImage("Pits-麻点校准20度第2轮.jpg", frameData);
                    mesAddInfor(ItemPits_20_2_List, QString::number(count), "PASS");
                    mPitsBplist20_2 = list;
                } else if (mIntPitsCalStep == 23) {
                    dutSaveImage("Pits-麻点校准20度第3轮.jpg", frameData);
                    mesAddInfor(ItemPits_20_3_List, QString::number(count), "PASS");
                    mPitsBplist20_3 = list;
                } else if (mIntPitsCalStep == 61) {
                    dutSaveImage("Pits-麻点校准60度第1轮.jpg", frameData);
                    mesAddInfor(ItemPits_60_1_List, QString::number(count), "PASS");
                    mPitsBplist60_1 = list;
                } else if (mIntPitsCalStep == 62) {
                    dutSaveImage("Pits-麻点校准60度第2轮.jpg", frameData);
                    mesAddInfor(ItemPits_60_2_List, QString::number(count), "PASS");
                    mPitsBplist60_2 = list;
                } else if (mIntPitsCalStep == 63) {
                    dutSaveImage("Pits-麻点校准60度第3轮.jpg", frameData);
                    if (count <= 2) {
                        mesAddInfor(ItemPits_60_3_List, QString::number(count), "PASS");
                    } else {
                        mesAddInfor(ItemPits_60_3_List, QString::number(count), "NG");
                        setErrString(-5, "麻点60度第三轮测试麻点数大于2，失败：" + QString::number(count));
                    }
                    mPitsBplist60_3 = list;
                }
            } else if (mIntPitsProc == CheckProc_HvStripeCheck) {
                m_hvStripsValue = dutHvStripsGetResult();
                QString log = QString("横纹检测结果：%1, Value: %2, ExpectMin: %3, ExpectMax: %4")
                        .arg(QString::number(m_hvStripsValue, 'f', 2))
                        .arg(QString::number(m_hvValueStatics, 'f', 2))
                        .arg(QString::number(m_hvValueExpectMin, 'f', 2))
                        .arg(QString::number(m_hvValueExpectMax, 'f', 2));
                writeLog(log);
            } else if (mIntPitsProc == CheckProc_StdCheck) {
                dutStdGetResult();
                writeLog("空域噪声计算, 阶段：" + QString::number(m_spaceStdStep));
                writeLog("全图平均方差std为：" + QString::number(m_stdSpace, 'f', 6));
                writeLog("全图高频平均方差std为：" + QString::number(m_stdSpaceHigh, 'f', 6));
                if (m_spaceStdStep == 1) {
                    m_stdSpaceStep1 = std::make_tuple(m_stdSpace, m_stdSpaceHigh);
                } else if (m_spaceStdStep == 2) {
                    m_stdSpaceStep2 = std::make_tuple(m_stdSpace, m_stdSpaceHigh);
                }
            } else if (mIntPitsProc == CheckProc_TimeStdCheck) {
                writeLog("时域噪声计算");
                imagecheck imgCheck;
                float timeStd = imgCheck.TimeStd(m_timeStdImageBuffer, mIntFrameCount, mIntFrameCount);
                if (m_timeStdStep == 1) {
                    m_timeStdValue = timeStd;
                    writeLog("全图时域平均方差std为：" + QString::number(m_timeStdValue, 'f', 6));
                } else if (m_timeStdStep == 2) {
                    m_timeStdValue2 = timeStd;
                    writeLog("全图时域平均方差std为：" + QString::number(m_timeStdValue2, 'f', 6));
                }
                if (!dutSaveTimeStdBuffer()) {
                    setErrString(-9, "时域视频数据保存失败");
                }
            }

            // 结束图像未输出图像判定线程
            // 置为0前需要备份，后面还会用到这个值...
            // 或者将线程销毁放到后面去也行
            // int oriProcTemp = mIntPitsProc;
            mIntPitsProc = 0;
            writeLog("#####等待图像判定线程完成");
            mImgJudgeProc->join();
            writeLog("#####图像判定线程完成");
            delete mImgJudgeProc;
            mImgJudgeProc = nullptr;

            gTestSteps++;
        }
    }
}

void CDutRelative::slotStartTest(QStringList sn)
{
    mImageSequence = 0;
    mBoolIngoreMes = false;
    mStartPoint = std::chrono::system_clock::now();
    mBdList.clear();
    mesInforClear();
    gErrorCode[mSlotId - 1] = 0;
    gErrorString[mSlotId - 1].clear();
    mBoolCalKOk = false;
    mBoolCalSffcOk = false;

    do {
        if (!dutPing()) {
            setErrString(-1, "Ping 待测工装失败");
            mBoolIngoreMes = true;
            break;
        }

        if (mSnGetMethod == SnGetMothod_Manual) {
            mAsicSN = sn.at(mSlotId - 1);
            mAsicDetectorSN = mAsicSN;
        } else if (mSnGetMethod = SnGetMothod_Production) {
            mAsicSN.clear();
            if (!execProc("读ASIC SN",
                          QStringList() << "ASIC SN" << "-" << "-",
                          std::bind(&CDutRelative::dutGetAsicSn, this)))
            {
                setErrString(-1, "SN获取失败");
                mBoolIngoreMes = true;
                break;
            }
            writeLog("读取ASIC的SN: " + mAsicSN);

            updateTable(mAsicSN, 1);
            if (mAsicSN.size() == 0) {
                setErrString(-1, "读取的SN为空");
                mBoolIngoreMes = true;
                break;
            }

            // 只有配置的SN长度大于0才激活SN的比对机制
            if (mSnLength > 0) {
                if (mAsicSN.size() != mSnLength) {
                    setErrString(-1, "读取的SN长度不对，需要长度: " + QString::number(mSnLength));
                    mBoolIngoreMes = true;
                    break;
                }

                if (!mSnPrefix.isEmpty()) {
                    if (mAsicSN.indexOf(mSnPrefix) != 0) {
                        setErrString(-1, "读取的SN前缀不对，需要前缀: " + mSnPrefix);
                        mBoolIngoreMes = true;
                        break;
                    }
                }
            }

        }

        // 根据 SN 创建log路径
        mLogPath.clear();
        ConfigInfo::getInstance()->getValueString("GLOBAL", "LogPath", mLogPath);
        if (mLogPath.size() <= 0) {
            mLogPath = QCoreApplication::applicationDirPath();
        }

        QString dirName = mAsicSN;
        dirName = dirName.replace("*", "_");
        mTmpLogPath = mLogPath + "/" + QDate::currentDate().toString("yyyyMMdd");
        mTmpLogPath += "/slot" + QString::number(mSlotId) + "/";
        mTmpLogPath += dirName + "_" + QTime::currentTime().toString("hhmmss") + "/";
        QDir dir(mTmpLogPath);
        if (!dir.exists()) {
            dir.mkpath(mTmpLogPath);
        }

        if (!dutGetDeviceName()) {
            setErrString(-1, "获取产品型号失败");
            mBoolIngoreMes = true;
            break;
        }
        writeLog("获取当前产品型号为: " +  m_deviceName);

        if (mIsOnlineMode) {
            QString out;
            writeLog("当前为在线模式，为SN执行入站操作：" + mAsicSN);
            if (!mMesObj.enterProduce(mAsicSN, out)) {
                setErrString(-1, "入站失败：" + out);
                mBoolIngoreMes = true;
                break;
            }

            if (!mMesObj.getSensorSN(mAsicSN, mAsicDetectorSN, "sensor_complete_sn")) {
                writeLog("获取探测器SN失败");
                if (mBpFileGetMethod == BpFileGet_Mes) {
                    setErrString(-1, "MES获取探测器SN失败!!!" + out);
                    mBoolIngoreMes = true;
                    break;
                }
            }
            writeLog("获取探测器SN为：" + mAsicDetectorSN);
        } else {
            mAsicDetectorSN = "";
            writeLog("当前为离线模式，跳过入站操作!");
        }

        if (!mBoolPitsSupported) {
            if (mBpFileGetMethod == BpFileGet_Local) {
                QString bdFilePath = mBPFilePath + "/" + mAsicDetectorSN + ".txt";
                writeLog("当前从本地文件获取坏点列表文件!");
                writeLog("坏点文件路径: " + bdFilePath);
                if (!fileExists(bdFilePath)) {
                    writeLog("坏点文件不存在：" + bdFilePath, -1);
                    setErrString(-1, "坏点文件不存在!");
                    mBoolIngoreMes = true;
                    break;
                }
            } else if (mBpFileGetMethod == BpFileGet_Mes) {
                QStringList list;
                QString sSn;
                if (mMesObj.getBadPointInfo(mAsicSN, list, sSn)) {
                    mBdList = list.join(";");
                    writeLog("当前从MES获取坏点列表: " + mBdList);
                    writeLog("当前从MES获取探测器SN: " + sSn);
                    mAsicDetectorSN = sSn;
                } else {
                    writeLog("MES获取探测器列表和探测器SN失败", -1);
                    setErrString(-1, "MES获取探测器列表和探测器SN失败");
                    mBoolIngoreMes = true;
                    break;
                }
            } else {
                writeLog("当前无坏点文件写入!");
            }
        }
        // 帧率传输要放在视频连接之前
        writeLog("开始设置传输帧率：" + QString::number(mTransmitFps));
        if (!execProc("设置帧率",
                      QStringList() << "传输帧率" << QString::number(mTransmitFps) << "-",
                      std::bind(&CDutRelative::dutSetFps, this)))
        {
            setErrString(-1, "帧率设置失败");
            break;
        }

        if (!execProc("连接视频", QStringList() << "连接视频" << "-" << "-",
                      std::bind(&CDutRelative::dutPlayVideo, this)))
        {
            setErrString(-1, "连接视频失败!");
            break;
        }

        float tmp;
        if (!dutGetFocusTemp(tmp)) {
            setErrString(-1, "读取焦温失败");
            break;
        } else {
            mFocusTempFirst = tmp;
            writeLog("当前焦温为：" + QString::number(mFocusTempFirst, 'f', 2));
            mesAddInfor(ItemA_FirstFocusTemp, QString::number(mFocusTempFirst, 'f', 2), "PASS");
            mesAddInfor(ItemPits_FirstFocusTemp, QString::number(mFocusTempFirst, 'f', 2), "PASS");
            writeLog("执行开机焦温比对");
            if (mBoolPitsSupported) {
                if ((mFocusTempFirst > mFocusTempLimit) || (mFocusTempFirst <= 0)) {
                    writeLog("执行焦温比对失败，焦温需要小于" + QString::number(mFocusTempLimit) + "度，大于0度");
                    setErrString(-1, "执行焦温比对失败，焦温需要小于" + QString::number(mFocusTempLimit) + "度，大于0度");
                    break;
                }
            } else {
                // 2023年4月11日 xiangh 工装盒异常会造成读取的焦温异常，在此处加入温度上限限制
                if ((mFocusTempFirst < mFocusTempLimit) || (mFocusTempFirst > 80)) {
                    writeLog("执行焦温比对失败，焦温需要大于" + QString::number(mFocusTempLimit) + "度");
                    setErrString(-1, "执行焦温比对失败，焦温需要大于" + QString::number(mFocusTempLimit) + "度");
                    break;
                }
            }
        }

        if (!execProc("开启快门",
                      QStringList(),
                      std::bind(&CDutRelative::dutOpenShutter, this)))
        {
            setErrString(-1, "开快门失败");
            break;
        }

        // 0 白热
        if (!execProc("设置白热模式",
                      QStringList() ,
                      std::bind(&CDutRelative::dutSetPallte, this, 0)))
        {
            setErrString(-1, "白热模式设置失败");
            break;
        }

        // 确定是否出图OK
        writeLog("检测是否出图正常");
//        bool brst = false;
//        for (int foo = 0; foo < 10; foo++) {
//            if (!dutSaveImage("开机图片.jpg")) {
//                QThread::msleep(500);
//                continue;
//            }
//            brst = true;
//            break;
//        }
        if (!videoJudgeNormally()) {
            setErrString(-1, "视频出图失败");
            break;
        }

        QString err;
        // 读取和比较Asic, ArtOs版本
        if (!dutReadyForTest(err)) {
            break;
        }
    } while (0);
    gTestSteps++;
}

void CDutRelative::slotOverTest()
{
    dutOpenAdaptive();

    slotSaveTemplate();

    mesUpload();

    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    mTestCostS = std::chrono::duration_cast<std::chrono::milliseconds>(end-mStartPoint).count() / 1000;
    writeLog("测试时间: " + QString::number(mTestCostS, 'f', 2));

    saveCsvReport();


    mTestEndTime = getFormatDateTime();
    writeLog("结束测试：" + mTestEndTime);

    int rstI = (gErrorCode[mSlotId - 1] == 0) ? 0 : 1;
    QString out;
    if (!mBoolIngoreMes && mIsOnlineMode ) {
        if (!mMesObj.outProduce(rstI, out)) {
            writeLog("出站失败：" + out, -1);
            setErrString(-5, "出站失败：" + out);
        } else {
            writeLog("出站OK: " + mAsicSN);
        }
    } else {
        if(mBoolIngoreMes) {
            writeLog("跳过MES过站操作，环境问题失败");
        } else {
            writeLog("当前为离线模式，不执行MES出站站操作");
        }
    }

    QString logName = "";
    if (gErrorCode[mSlotId - 1] == 0) {
        writeLog("测试PASS", 1);
        showLabel("PASS", 1);
        logName = "PASS_";
    } else {
        writeLog("测试FAIL, Error MSG: " + gErrorString[mSlotId - 1], -1);
        showLabel("FAIL:" + gErrorString[mSlotId - 1], -1);
        logName = "FAIL_";
    }
    QString dirName = mAsicSN;
    dirName = dirName.replace("*", "_");
    logName += dirName + "_" + QTime::currentTime().toString("hhmmss");

    QString pathCsv = mTmpLogPath + logName + ".csv";
    QString pathTxt = mTmpLogPath + logName + ".txt";
    writeLog("测试log保存至: " + pathTxt);

    emit mProduce->logTxt(pathTxt, mSlotId - 1);
    emit mProduce->tCsv(pathCsv, mSlotId - 1);

    gTestSteps++;
}

void CDutRelative::slotKPreCal()
{
    do {
        if (hasError()) break;

        // 2 UYVY 3 Y16 1 YUYV
        if (!execProc("设置视频格式Y16",
                      QStringList() << "视频格式" << "Y16" << "-",
                      std::bind(&CDutRelative::dutSetVideoFormat, this, 3)))
        {
            setErrString(-1, "图像格式Y16设置失败");
            break;
        }
    }while(0);

    gTestSteps++;
}

bool CDutRelative::dutPing()
{
    int iContinue = 0;
    writeLog("开始 ping ip " + mArtOsynIp);
    for (int foo = 0; foo < 20; foo++) {
        if (CPingTest::pingTest(nullptr, mArtOsynIp.toLocal8Bit().data())) {
            writeLog("ping ip " + mArtOsynIp + " ok!");
            iContinue++;
            if (iContinue >= 3) {
                return true;
            }
        } else {
            writeLog("ping ip " + mArtOsynIp + " failed!");
            iContinue = 0;
            QProcess process;
            QStringList arg;
            arg << "/C" << "arp -d";
            process.start("cmd.exe", arg);
            process.waitForStarted();
            process.waitForFinished();
//            QByteArray ba = process.readAllStandardOutput();
//            QTextCodec* pTextCodec = QTextCodec::codecForName("GBK");
//            QString str = pTextCodec->toUnicode(ba);
//            writeLog("OUT: " + str);
            QByteArray ba = process.readAllStandardError();
            if (ba.size() > 0) {
                QTextCodec* pTextCodecX = QTextCodec::codecForName("GBK");
                QString str = pTextCodecX->toUnicode(ba);
                writeLog(str);
            }
        }
        mySleep(500);
    }

    return false;
}

bool CDutRelative::dutReadyForTest(QString &err)
{
    mAsicVersion.clear();
    if (!execProc("获取ASIC版本",
                  QStringList() << "ASIC版本" << "-" << "-",
                  std::bind(&CDutRelative::dutGetAsicVersion, this)))
    {
        mesAddInfor(ItemA_AsicVersion, "", "NG");
        setErrString(-1, "ASIC版本获取失败");
        return false;
    }
    writeLog("获取ASIC版本信息：" + mAsicVersion);
    updateTable(mAsicVersion, 1);
    mesAddInfor(ItemA_AsicVersion, mAsicVersion, "PASS");

    mArtOsVersion.clear();
    if (!execProc("获取控制盒版本",
                  QStringList() << "控制盒版本" << "-" << "-",
                  std::bind(&CDutRelative::dutGetArtOsVersion, this)))
    {
        mesAddInfor(ItemA_ArtOsVersion, "", "NG");
        setErrString(-1, "ARM版本获取失败");
        return false;
    }
    writeLog("获取控制盒版本信息：" + mArtOsVersion);
    updateTable(mArtOsVersion, 1);
    mesAddInfor(ItemA_ArtOsVersion, mArtOsVersion, "PASS");

    return true;
}

bool CDutRelative::dutGetAsicSn()
{
    for (int foo = 0; foo < 6; foo++) {
        AsicControl asic(mArtOsynIp);
        QString sn = asic.GetSn();
        if (sn.size() > 0) {
            mAsicSN = sn;
            return true;
        }

        mySleep(1000);
    }

    return false;
}

bool CDutRelative::dutGetDeviceName()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        QString name;
        if (asic.GetDeviceName(name)) {
            if (name.size() > 0) {
                m_deviceName = name.toUpper();
                return true;
            }
        }

        mySleep(100);
    }

    return false;
}

bool CDutRelative::dutGetArtOsVersion()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.GetArtOsVersion(mArtOsVersion);
        if (ret) return true;

        QThread::msleep(500);
    }

    return false;
}

bool CDutRelative::dutGetAsicVersion()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.GetAsicVersion(mAsicVersion);
        if (ret) return true;

        QThread::msleep(500);
    }

    return false;
}

bool CDutRelative::dutOpenShutter()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.ChangeWorkMode(0);
        if (ret) return true;

        QThread::msleep(500);
    }

    return false;
}

bool CDutRelative::dutSetFps()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.setTransmitFps(mTransmitFps);
        if (ret) return true;

        QThread::msleep(500);
    }

    return false;
}

bool CDutRelative::dutSetVideoFormat(int fmt)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.changeSensorFormat(fmt);
        if (ret) {
            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < 5000) {
                QCoreApplication::processEvents();
                VideoFormatInfo info;
                VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, info);
                switch (fmt) {
                case 1:
                    if (info.format == VIDEO_FORMAT_UYUV) {
                        return true;
                    }
                case 3:
                    if (info.format == VIDEO_FORMAT_Y16) {
                        return true;
                    }
                }
                QThread::msleep(1000);
                asic.changeSensorFormat(fmt);
            }
            return false;
        }
        QThread::msleep(500);
    }
    return false;
}

// 0, 白热
bool CDutRelative::dutSetPallte(int mode)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        bool ret = asic.SetPattle(mode);
        if (ret) return true;

        QThread::msleep(200);
    }

    return false;
}

bool CDutRelative::dutCloseShutter()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.ChangeWorkMode(1)) {
            return true;
        }

        QThread::msleep(200);
    }

    return false;
}

bool CDutRelative::dutPlayVideo()
{
    emit sigPlayVideo(mSlotId - 1);
    mySleep(1000);
    return true;
}

void CDutRelative::slotKCal20()
{
    bool bRst = false;
    do {
        if (hasError()) break;

        if (!execProc("关自适应补偿", QStringList() << "关自适应补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutCloseAdaptive, this), 1000)) {
            setErrString(-4, "自适应关闭失败");
            break;
        }
        dutSaveImage("校K-自适应关.jpg");

        if (!execProc("场景NUC", QStringList() << "场景NUC" << "-" << "-",
                      std::bind(&CDutRelative::dutSceneNUC, this), 3000)) {
            setErrString(-4, "场景NUC失败");
            break;
        }

        if (!execProc("BL补偿", QStringList() << "BL补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutBLCompensation, this), 5000)) {
            setErrString(-4, "BL补偿失败");
            break;
        }
        dutSaveImage("校K-BL图像.jpg");

        mK20Ad2 = -1;
        if (!execProc("获取20度平均AD值", QStringList() << "20度AD" << "-" << "-",
                      std::bind(&CDutRelative::dutGetAdValue, this, std::ref(mK20Ad2)))) {
            setErrString(-4, "取20度平均AD值失败: ad is " + QString::number(mK20Ad2));
            mesAddInfor(ItemK_20AD2, QString::number(mK20Ad2), "NG");
            break;
        }
        emit mProduce->tableUpdateData(QString::number(mK20Ad2), 1, mSlotId - 1);
        writeLog("获取20度AD值：" + QString::number(mK20Ad2));
        mesAddInfor(ItemK_20AD2, QString::number(mK20Ad2), "PASS");

    } while(0);
    gTestSteps++;
}

void CDutRelative::slotKCal60()
{
    do {
        if (hasError()) break;

        if (!execProc("BH补偿", QStringList() << "BH补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutBHCompensation, this), 5000)) {
            setErrString(-4, "BH补偿失败");
            break;
        }
        dutSaveImage("校K-BH图像.jpg");

        mK60Ad1 = -1;
        if (!execProc("获取60度平均AD值", QStringList() << "60度AD" << "-" << "-",
                      std::bind(&CDutRelative::dutGetAdValue, this, std::ref(mK60Ad1)))) {
            setErrString(-4, "取60度平均AD值失败: ad is " + QString::number(mK60Ad1));
            mesAddInfor(ItemK_60AD1, QString::number(mK60Ad1), "NG");
            break;
        }
        emit mProduce->tableUpdateData(QString::number(mK60Ad1), 1, mSlotId - 1);
        writeLog("获取60度AD值："  + QString::number(mK60Ad1));
        mesAddInfor(ItemK_60AD1, QString::number(mK60Ad1), "PASS");

        if (!execProc("获取AD获取时焦温", QStringList(),
                      std::bind(&CDutRelative::dutGetFocusTemp, this, std::ref(mFocusTempAdGet)))) {
            setErrString(-4, "获取AD获取时焦温失败: " + QString::number(mFocusTempAdGet));
            mesAddInfor(ItemK_ADFocusTemp, QString::number(mFocusTempAdGet, 'f', 2), "NG");
            break;
        }
        writeLog("获取AD值时焦温为：" + QString::number(mFocusTempAdGet, 'f', 2));
        mesAddInfor(ItemK_ADFocusTemp, QString::number(mFocusTempAdGet, 'f', 2), "PASS");

        mKRespRate = float(mK60Ad1 - mK20Ad2) * 2000.0f / 16384.0f / float(60 - 20);
        writeLog("获取响应率为：" + QString::number(mKRespRate, 'f', 2));
        if (mKRespRate < 6) {
            setErrString(-4, "响应率门限比对失败:( >=6 ) ：" + QString::number(mKRespRate));
            mesAddInfor(ItemK_RespRate, QString::number(mKRespRate, 'f', 2), "NG");
            break;
        } else {
            mesAddInfor(ItemK_RespRate, QString::number(ItemK_RespRate, 'f', 2), "PASS");
        }


        if (m_deviceName.contains("XD01A")) {
            writeLog("目前XD01A还不支持寄存器读取(此处不读取RASEL&HSSD值)", 2);
        } else {
            if (!execProc("获取HSSD&RASEL寄存器数据", QStringList(),
                          std::bind(&CDutRelative::recordRegisterData_i2c, this))) {
                setErrString(-4, "获取HSSD&RASEL寄存器数据失败");
                break;
            }
        }

        if (!execProc("计算K矩阵", QStringList() << "计算K矩阵" << "-" << "-",
                      std::bind(&CDutRelative::dutKCoffCalculate, this), 2000)) {
            setErrString(-4, "计算K矩阵失败");
            break;
        }

        if (!execProc("保存K矩阵", QStringList() << "保存K矩阵" << "-" << "-",
                      std::bind(&CDutRelative::dutKCoffSave, this), 6000)) {
            setErrString(-4, "保存K矩阵失败");
            break;
        }

        if (!execProc("开自适应补偿", QStringList() << "开自适应补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutOpenAdaptive, this), 2000)) {
            setErrString(-4, "开自适应补偿失败");
            break;
        }

        mBoolCalKOk = true;
        dutSaveImage("校K-自适应开.jpg");
    } while(0);
    gTestSteps++;
}

void CDutRelative::slotKCheck20()
{
    do {
        if (hasError()) break;

        if (!execProc("20度Check K", QStringList() << "20度CheckK" << "-" << "-",
                      std::bind(&CDutRelative::dutKCheck, this, 20), 1000)) {
            setErrString(-4, "20度check K失败");
            break;
        }

    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutKCheck(int kStep)
{
    QByteArray imgData;
    if (!getImageData(imgData)) {
        setErrString(-4, "获取图片数据失败");
        return false;
    }

    VideoFormatInfo fmt;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, fmt);

    unsigned char* src_data = new unsigned char[WIDTH * HEIGHT * 2];
    memcpy(src_data, imgData.data() + fmt.nvsLen, 640 * 512 * 2);
    //    getValueY(imgData, src_data);

    Frock_cal cal;
    int high_value=0,low_value=0;
    bool ret = cal.Check_uniform((unsigned short*)src_data, WIDTH, HEIGHT, high_value, low_value);
    delete[]src_data;

    QString log = QString("校K检测结果 high_vale: %1, low_value: %2").arg(high_value).arg(low_value);
    writeLog(log);
    log = QString("本地采集校K结果检验结束: %1").arg(ret ? "PASS" : "FAIL" );
    writeLog(log);
    QString retStr = (ret ? "PASS" : "NG");
    if (kStep == 20) {
        mKCheckMaxValue20 = high_value;
        mKCheckMinValue20 = low_value;
        mesAddInfor(ItemK_CheckMaxVal20, QString::number(high_value), retStr);
        mesAddInfor(ItemK_CheckMinVal20, QString::number(low_value), retStr);
        dutSaveImage("校K-BL校验.jpg", imgData);
    } else {
        mKCheckMaxValue60 = high_value;
        mKCheckMinValue60 = low_value;
        mesAddInfor(ItemK_CheckMaxVal60, QString::number(high_value), retStr);
        mesAddInfor(ItemK_CheckMinVal60, QString::number(low_value), retStr);
        dutSaveImage("校K-BH校验.jpg", imgData);
    }
    return ret;
}

void CDutRelative::slotKCheck60()
{
    do {
        if (hasError()) break;
        if (!execProc("60度Check K", QStringList() << "60度CheckK" << "-" << "-",
                      std::bind(&CDutRelative::dutKCheck, this, 60), 1000)) {
            setErrString(-4, "60度check K失败");
            break;
        }
    } while(0);
    gTestSteps++;
}

void CDutRelative::slotBpPreCal()
{
    do {
        if (hasError()) break;

        // 2 UYVY 3 Y16 1 YUYV
        if (!execProc("设置视频格式Y16",
                      QStringList() << "视频格式" << "Y16" << "-",
                      std::bind(&CDutRelative::dutSetVideoFormat, this, 3)))
        {
            setErrString(-1, "图像格式Y16设置失败");
            break;
        }


        if (!execProc("清坏点列表", QStringList(),
                      std::bind(&CDutRelative::dutClearBPList, this), 500)) {
            setErrString(-4, "清除探测器坏点列表失败");
            break;
        }

        int bpCount = 0;
        if (dutGetBpCount(bpCount)) {
            writeLog("获取坏点列表个数为：" + QString::number(bpCount));
        }

        QString list;
        if (mBpFileGetMethod == BpFileGet_Local) {
            QString mPath = mBPFilePath;
            writeLog(QString("从文件获取坏点列表, 路径：%1, 文件名称：%2.txt").arg(mPath).arg(mAsicDetectorSN) );
            if (!execProc("读坏点列表文件", QStringList(),
                          std::bind(&CDutRelative::dutGetBPListFromFile, this, mPath, mAsicDetectorSN, std::ref(list)))) {
                setErrString(-4, "从文件读取坏点列表失败");
                break;
            }

            // 最小坏点列表数为3个字节：1,1，小于3个字节的文件会被判定为空，不用写入坏点文件列表
            // 空的坏点文件里面写的是"无"
            // if (list.size() < 3) {

            // 如上更改为寻找","符号，如果找不到说明为空坏点列表文件
            if (list.indexOf(",") == -1) {
                writeLog("文件中坏点列表为空，不执行坏点列表写入动作");
            } else {
                writeLog(QString("从文件获取到坏点列表：%1").arg(list));
                if (!execProc("写坏点列表文件", QStringList(),
                              std::bind(&CDutRelative::dutSetBPList, this, 0, list), 2000)) {
                    setErrString(-4, "写入坏点列表失败");
                    break;
                }
            }
        } else if (mBpFileGetMethod == BpFileGet_Mes) {
            writeLog("从MES获取坏点列表写入");
            if (!execProc("写坏点列表文件", QStringList(),
                          std::bind(&CDutRelative::dutSetBPList, this, 0, mBdList), 2000)) {
                setErrString(-4, "写入坏点列表失败");
                break;
            }
        } else {

        }

        dutSaveImage("校BP-导入坏点列表.jpg");

        writeLog("K值校坏点门限：" + QString::number(mKCalBPMaxLimit));
        if (!execProc("K值一键校坏点", QStringList() << "K-校坏点" << "-" << "-",
                      std::bind(&CDutRelative::dutBPKCal, this), 3000)) {
            setErrString(-4, "K值一键校坏点失败");
            break;
        }
        dutSaveImage("校BP-K值校坏点.jpg");
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutClearBPList()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.DeleteBadPoint()) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutGetBpCount(int &count)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.GetBadPointsCount(count)) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutOpenAdaptive()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.AutoAdapteComp(1)) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutCloseAdaptive()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.AutoAdapteComp(0)) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutSceneNUC()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SceneNuc()) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutSceneCompensation()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SceneComp()) {
            return true;
        }
    }

    return false;
}

void CDutRelative::slotSffcRaiseTemp()
{
    do {
        if (hasError()) break;

        if (!dutSffcEnable(mTempRaiseDiff)) {
            break;
        }
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutGetAdValue(int &ad)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.GetAd(ad)) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutBLCompensation()
{
    float temp = 0.0f;
    dutGetFocusTemp(temp);
    m_tempFocus_20_60 = std::make_pair(temp, 0.0f);
    writeLog("Get 20 BL temprature: " + QString::number(temp));
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.BlComp()) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutBHCompensation()
{
    float temp = 0.0f;
    dutGetFocusTemp(temp);
    m_tempFocus_20_60 = std::make_pair(m_tempFocus_20_60.first, temp);
// 2023年10月16日：取消0.16的温差比对
//    if (fabs(temp - m_tempFocus_20_60.first) > 0.16) {
//        setErrString(-19, "20°与60°补偿前后温差大于0.16，失败!");
//        return false;
//    }
    writeLog(QString("20°与60°补偿前后温差为%1，在校准温升接受范围内!").arg(fabs(temp - m_tempFocus_20_60.first)));

    writeLog("Get 60 BH temprature: " + QString::number(temp));
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.BhComp()) {
            return true;
        }
    }

    return false;
}

bool CDutRelative::dutKCoffCalculate()
{
    AsicControl asic(mArtOsynIp);
    return asic.CaluteK();
}

bool CDutRelative::dutKCoffSave()
{
    AsicControl asic(mArtOsynIp);
    return asic.SaveK();
}

// 2023年4月26日 xiangh 加入K文件保存XD01A的部分
bool CDutRelative::dutSendGetKCmds(int addr)
{
    bool bRst = false;
    AsicControl asic(mArtOsynIp);
    if (m_deviceName.contains("XD01A", Qt::CaseInsensitive)) {
        // 55 AA 07 A0 01 12 00 00 00 04
        QString sCmd = "55 AA 07 A0 01 12 ";
        if (addr == 0x04) {
            sCmd += "00 00 00 04";
        } else if (addr == 0x05) {
            sCmd += "00 00 00 05";
        }
        bRst = asic.SendCustomUart(sCmd);
    } else {
        bRst = asic.SendCustom(0x1e1, addr);
    }

    return bRst;
}

bool CDutRelative::dutSaveKFile(int addr, QString fileName)
{
    writeLog("发送模板文件提取指令");

    mImageQue.clear();
    if(!dutSendGetKCmds(addr)) {
        setErrString(-4, "发送模板文件提取指令失败");
        return false;
    }

    int iframe = 0;
    int iframLimit = 100;


    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, info);

    unsigned char * pDstData = new unsigned char[WIDTH * HEIGHT * 2];
    memset(pDstData, 0 , WIDTH * HEIGHT * 2);

    bool bRst = false;
    bool b1 = false, b2 = false;
    QElapsedTimer timer;
    timer.start();
    while ( !bRst && (timer.elapsed() < 10000)) {
        QCoreApplication::processEvents();

        bRst = b1 & b2;
        if (bRst) {
            break;
        }

        if (mImageQue.size() > 0) {
            QByteArray ba = mImageQue.at(0);
            mImageQue.pop_front();

            if (!b1) {
                b1 = dutGetK(ba, 1);
                if (b1) {
                    dutSaveImage("type1.jpg", ba);
                    for(int i = 0; i < WIDTH * HEIGHT; i++) {
                        pDstData[i] = ba.at(info.nvsLen + 2 * i + 1);
                    }
                    continue;
                }
            }

            if (!b2) {
                b2 = dutGetK(ba, 2);
                if (b2) {
                    dutSaveImage("type2.jpg", ba);
                    for(int i = 0; i < WIDTH * HEIGHT; i++) {
                        pDstData[WIDTH * HEIGHT + i] = ba.at(info.nvsLen + 2 * i + 1);
                    }
                    continue;
                }
            }

            if (iframe++ > iframLimit) {
                iframe = 0;
                mImageQue.clear();
                if(!dutSendGetKCmds(addr)) {
                    // setErrString(-4, "发送模板文件提取指令失败");
                    continue;
                }
            }
            qDebug() << ">>>> Frames " << iframe;
        }
        ///mySleep(30); 延时多余，还会造成数据帧丢失
    }

    if (!bRst) {
        writeLog("模板文件提取失败");
        setErrString(-2, "模板数据提取失败");
        return false;
    }


    bRst = false;
    QString path = mTmpLogPath + fileName;
    QFile videoFile(path);
    if(videoFile.open(QIODevice::WriteOnly)) {
        videoFile.write((char *)pDstData, WIDTH * HEIGHT * 2);
        videoFile.flush();
        videoFile.close();
        writeLog("模板文件保存OK: " + path);
        bRst = true;
    } else {
        writeLog("模板文件保存FAIL: " + path, -1);
    }

    delete []pDstData;

    return bRst;
}


bool CDutRelative::dutGetK(QByteArray k1, int page)
{
    unsigned char* pPreData = new unsigned char[WIDTH * HEIGHT * 2];
    memset(pPreData, 0, WIDTH * HEIGHT * 2);
    memcpy(pPreData, k1.data() + WIDTH * 4, WIDTH * HEIGHT * 2);

    bool bGetOk = false;
    int iDe = 0;
    for(iDe = 640 * 2 * 10; iDe > 0; iDe -= 2) {
        if(pPreData[iDe] != page) {
            break;
        }

        if(iDe <= 10) {
            bGetOk = true;
            writeLog("get type "+ QString::number(page) +" success!", 1);
            break;
        }
    }

    delete []pPreData;
    pPreData = nullptr;

    return bGetOk;
}

bool CDutRelative::dutGetBPListFromFile(QString sPath, QString detectorSN, QString &list)
{
    QString path = sPath + "/";
    path += detectorSN + ".txt";
    QFile file(path);

    list.clear();
    if(file.open(QIODevice::ReadOnly)) {
        QString fileData(file.readAll());
        QStringList badpointList = fileData.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        for(int i=0; i<badpointList.size(); i++) {
            QString point = badpointList[i];
            list.append(point);
            if(i != badpointList.size()-1) {
                list.append(";");
            }
        }
        file.close();
        return true;
    }

    return false;
}

bool CDutRelative::dutGetBPListFromDUT(QString &list)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.GetBadPointsList(list)) {
            return true;
        }
        QThread::msleep(100);
    }
    return false;
}

void CDutRelative::slotBpCheck20()
{
    do {
        if (hasError()) break;
        if (!execProc("20度Check BP", QStringList() << "20度CheckBP" << "-" << "-",
                      std::bind(&CDutRelative::dutBpCheck, this, 20), 1000)) {
            setErrString(-4, "20度check BP失败");
            break;
        }
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutBpCheck(int kStep)
{
    QByteArray imgData;

    if (!getImageData(imgData)) {
        setErrString(-4, "获取图片数据失败");
        return false;
    }

    unsigned char* src_data = new unsigned char[WIDTH * HEIGHT * 2];

    //Y16检坏点
    memcpy(src_data, imgData.data() + 640 * 4, WIDTH * HEIGHT * 2);
    Frock_cal cal;
    std::vector<Points> badPoints;
    int bad_count = cal.Check_badpoint((unsigned short*)src_data, badPoints, WIDTH, HEIGHT);

    //Y8检坏点
    //    getValueY(imgData, src_data);
    //    int bad_count = cal.Check_badpoint(src_data, badPoints, WIDTH, HEIGHT);

    delete[]src_data;
    src_data = nullptr;
    writeLog("坏点结果检验结束");
    writeLog("检测的换点个数: " + QString::number(bad_count));

    QString bdList;
    mBpCheckList20 = "";
    mBpCheckList60 = "";
    if (bad_count > 0) {
        foreach(Points p, badPoints) {
            bdList.append(QString("%1,%2;").arg(p.x).arg(p.y));
        }
        writeLog("Check BP list: " + bdList);
        if (kStep == 20) {
            mBpCheckList20 = bdList;
            //            if (bdList.size() >= 64) bdList = bdList.left(63);
            dutSaveImage("校BP-坏点校验20.jpg", imgData);
            mesAddInfor(ItemBp_Check20List, QString::number(bad_count), "NG");
        } else {
            mBpCheckList60 = bdList;
            //            if (bdList.size() >= 64) bdList = bdList.left(63);
            dutSaveImage("校BP-坏点校验60.jpg", imgData);
            mesAddInfor(ItemBp_Check60List, QString::number(bad_count), "NG");
        }
        return false;
    }

    if (kStep == 20) {
        dutSaveImage("校BP-坏点校验20.jpg", imgData);
        mesAddInfor(ItemBp_Check20List, "0", "PASS");
    } else {
        dutSaveImage("校BP-坏点校验60.jpg", imgData);
        mesAddInfor(ItemBp_Check60List, "0", "PASS");
    }
    return true;
}

void CDutRelative::slotBpCheck60()
{
    do {
        if (hasError()) break;
        if (!execProc("60度Check BP", QStringList() << "60度CheckBP" << "-" << "-",
                      std::bind(&CDutRelative::dutBpCheck, this, 60), 1000)) {
            setErrString(-4, "60度check BP失败");
            break;
        }
    } while(0);
    gTestSteps++;
}

bool CDutRelative::getImageData(QByteArray& imgData)
{
    for (int foo = 0; foo < 3; foo++)
    {
        QCoreApplication::processEvents();
        {
            //            QMutexLocker locker(&mMutexFrameData);
            imgData = mCrtFrameData;
            mCrtFrameData = QByteArray();
        }
        if (imgData.size() > 0)
            return true;

        mySleep(500);
    }

    writeLog("get image failed", -1);
    return false;
}

bool CDutRelative::dutSetBPList(const int mode, QString list)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SetBadPointList(mode, list)) {
            return true;
        }
        QThread::msleep(100);
    }
    return false;
}

bool CDutRelative::dutBPKCal()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SetKBadPoint(mKCalBPMaxLimit)) {
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

void CDutRelative::slotBpCal20()
{
    do {
        if (hasError()) break;

        if (!execProc("20度黑体位置快门补偿", QStringList() << "20度-快门补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutShutterComp, this), 1000)) {
            setErrString(-4, "20度快门补偿失败");
            dutSaveImage("校BP-快门补偿20-FAIL.jpg");
            break;
        }

        if (!execProc("20度黑体位置Y16一键校坏点", QStringList() << "Y16-校坏点" << "-" << "-",
                      std::bind(&CDutRelative::dutBPY16Cal, this), 3000)) {
            setErrString(-4, "20度Y16一键校坏点失败");
            dutSaveImage("校BP-Y16校坏点20-FAIL.jpg");
            break;
        }
        dutSaveImage("校BP-Y16校坏点20.jpg");
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutShutterComp()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.ShutterComp()) {
            return true;
        }
        QThread::msleep(100);
    }

    return false;
}

bool CDutRelative::dutBPY16Cal()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SetY16BadPoint(mY16CalBPMaxLimit)) {
            return true;
        }
        QThread::msleep(100);
    }

    return false;
}

void CDutRelative::slotBpCal60()
{
    do {
        if (hasError()) break;
        if (!execProc("60度黑体位置快门补偿", QStringList() << "60度-快门补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutShutterComp, this), 1000)) {
            setErrString(-4, "60度快门补偿失败");
            dutSaveImage("校BP-快门补偿60-FAIL.jpg");
            break;
        }

        if (!execProc("60度黑体位置Y16一键校坏点", QStringList() << "Y16-校坏点" << "-" << "-",
                      std::bind(&CDutRelative::dutBPY16Cal, this), 3000)) {
            setErrString(-4, "60度Y16一键校坏点失败");
            dutSaveImage("校BP-Y16校坏点60-FAIL.jpg");
            break;
        }
        dutSaveImage("校BP-Y16校坏点60.jpg");

        if (!execProc("坏点保存", QStringList() << "坏点保存" << "-" << "-",
                      std::bind(&CDutRelative::dutBPSave, this), 5000)) {
            setErrString(-4, "保存坏点失败");
            break;
        }


        int bpCount = 0;
        if (dutGetBpCount(bpCount)) {
            writeLog("获取坏点列表个数为：" + QString::number(bpCount));
        }

        // 2023年4月11日 xiangh 此处因工装盒问题，加入了100坏点限制，工装盒读取坏点列表数太长会造成i2c挂掉，需重启工装盒
        if (bpCount < 100) {
            QString listx;
            if (!execProc("回读坏点列表", QStringList(),
                          std::bind(&CDutRelative::dutGetBPListFromDUT, this, std::ref(listx)))) {
                setErrString(-4, "回读坏点列表失败");
                mBpAllList = listx;
                int count = listx.split(";").size();
                mesAddInfor(ItemBp_List, QString::number(count), "NG");
                break;
            }
            mBpAllList = listx;
            writeLog(QString("从产品获得坏点列表：%1").arg(mBpAllList));
        }
        mesAddInfor(ItemBp_List, QString::number(bpCount), "PASS");
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutBPSave()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SaveBadPoint()) {
            return true;
        }
        QThread::msleep(100);
    }

    return false;
}


bool CDutRelative::dutGetFocusTemp(float &temp)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.GetFocusTemp(temp)) {
            return true;
        }
        QThread::msleep(100);
    }

    return false;
}

void CDutRelative::slotSffcPreCal()
{
    do {
        if (hasError()) break;
        // 2 UYVY 3 Y16 1 YUYV
        if (!execProc("设置视频格式Y16",
                      QStringList() << "视频格式" << "Y16" << "-",
                      std::bind(&CDutRelative::dutSetVideoFormat, this, 3)))
        {
            setErrString(-1, "图像格式Y16设置失败");
            break;
        }

        if (!execProc("关自适应补偿", QStringList() << "关自适应补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutCloseAdaptive, this), 3000)) {
            setErrString(-4, "关自适应补偿失败");
            break;
        }

        if (!execProc("场景NUC", QStringList() << "场景NUC" << "-" << "-",
                      std::bind(&CDutRelative::dutSceneNUC, this), 3000)) {
            setErrString(-4, "场景NUC失败");
            break;
        }

        if (!execProc("场景补偿", QStringList() << "场景补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutSceneCompensation, this), 3000)) {
            setErrString(-4, "场景补偿失败");
            break;
        }
        dutSaveImage("sffc-升焦温前.jpg");
    } while(0);
    gTestSteps++;
}

void CDutRelative::slotSffcCal()
{
    do {
        if (hasError()) break;
        dutSaveImage("sffc-升焦温后.jpg");

        if (!execProc("SFFC采集", QStringList() << "SFFC采集" << "-" << "-",
                      std::bind(&CDutRelative::dutSffcCollect, this), 3000)) {
            setErrString(-4, "SFFC采集失败");
            break;
        }
        dutSaveImage("sffc-SFFC采集.jpg");

        if (!execProc("SFFC保存", QStringList() << "SFFC保存" << "-" << "-",
                      std::bind(&CDutRelative::dutSffcSave, this), 8000)) {
            setErrString(-4, "SFFC保存失败");
            break;
        }
        dutSaveImage("sffc-SFFC保存.jpg");
        mBoolCalSffcOk = true;

    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutSffcEnable(float diff)
{
    float firstTemp = 0.0f;
    if (!execProc("第一次获取焦温值", QStringList() << "初次焦温" << "-" << "-",
                  std::bind(&CDutRelative::dutGetFocusTemp, this, std::ref(firstTemp)))) {
        setErrString(-4, "获取初次焦温失败");
        return false;
    }
    updateTable(QString::number(firstTemp, 'f', 2), 1);

    // 2023年4月11日 xiangh 工装盒异常会造成读取的焦温异常，在此处加入温度上限限制
    if (firstTemp > 80) {
        setErrString(-5, "焦温获取异常，需重启工装盒后再次执行测试");
        return false;
    }

    writeLog("初次焦温: " + QString::number(firstTemp, 'f', 2));
    writeLog("焦温需要升至: " + QString::number(firstTemp + diff, 'f', 2));

    if (diff < 2.99f) {
        mTempRaiseTOS = 4 * 60; // 5分钟超时时间
    } else {
        mTempRaiseTOS = 15 * 60;
    }

    QElapsedTimer timer;
    timer.start();
    writeTable(QStringList() << "实时焦温" << "-" << "-");
    while (timer.elapsed() < mTempRaiseTOS * 1000) {
        QCoreApplication::processEvents();

        float temp = 0.0f;
        if (!dutGetFocusTemp(temp)) {
            mySleep(1000);
            continue;
        }


        updateTable(QString::number(temp, 'f', 2), 1);
        if (temp - firstTemp > diff) {
            mSffcFocusTempLast = temp;
            if (diff >= 2.99f) { // 目前暂定SFFC才需要升温3°
                mesAddInfor(ItemSffc_LastFocurTemp, QString::number(mSffcFocusTempLast, 'f', 2), "PASS");
            }
            changeTableStatus(0); // pass
            return true;
        }
        // 2023年4月11日 xiangh 工装盒异常会造成读取的焦温异常，在此处加入温度上限限制
        if (temp > 80) {
            setErrString(-5, "焦温获取异常，需重启工装盒后再次执行测试");
            return false;
        }

        mySleep(3000);
    }

    mesAddInfor(ItemSffc_LastFocurTemp, QString::number(mSffcFocusTempLast, 'f', 2), "NG");
    setErrString(-2, "焦温升温失败(超时)");
    return false;
}

bool CDutRelative::dutSffcCollect()
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.GetSffcValue()) {
            return true;
        }
        QThread::msleep(100);
    }

    return false;
}

bool CDutRelative::dutSffcSave()
{
    AsicControl asic(mArtOsynIp);
    return asic.SaveSffcValue();
}

void CDutRelative::slotSffcCheck()
{
    do {
        if (hasError()) break;
        if (!execProc("Check Sffc", QStringList() << "CheckSffc" << "-" << "-",
                      std::bind(&CDutRelative::dutSffcCheck, this), 1000)) {
            setErrString(-4, "check sffc失败");
            break;
        }

        if (!execProc("开自适应补偿", QStringList() << "开自适应补偿" << "-" << "-",
                      std::bind(&CDutRelative::dutOpenAdaptive, this), 3000)) {
            setErrString(-4, "开自适应补偿失败");
            break;
        }
        dutSaveImage("sffc-SFFC自适应补偿开.jpg");
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutSffcCheck()
{
    dutSaveImage("SFFC Check前.jpg");
    QByteArray imgData;
    if (!dutMRTDAlgorithm(Mrtd_PotLid, 0)) {
        setErrString(-4, "关锅盖失败");
        return false;
    }

    mySleep(2000);
    if (!getImageData(imgData)) {
        setErrString(-4, "获取图片数据失败");
        return false;
    }

    dutSaveImage("sffc-锅盖算法关.jpg", imgData);
    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, videoInfo);
    unsigned char* pImage1 = new unsigned char[WIDTH * HEIGHT * 4];
    memcpy(pImage1, imgData.mid(videoInfo.nvsLen, videoInfo.frameLen), videoInfo.frameLen);
    //Y8修改
    //    getValueY(imgData, pImage1);

    //Y16修改


    if (!dutMRTDAlgorithm(Mrtd_PotLid, 1)) {
        setErrString(-4, "开锅盖失败");
        delete []pImage1;
        return false;
    }
    mySleep(2000);
    QByteArray imgData2;

    if (!getImageData(imgData2)) {
        setErrString(-4, "获取图片数据失败");
        delete []pImage1;
        return false;
    }

    dutSaveImage("sffc-锅盖算法开.jpg", imgData2);
    unsigned char* pImage2 = new unsigned char[WIDTH * HEIGHT * 4];
    //Y8修改
    //    getValueY(imgData, pImage2);

    //Y16修改
    memcpy(pImage2, imgData2.mid(videoInfo.nvsLen, videoInfo.frameLen), videoInfo.frameLen);

    Frock_cal cal;
    mSffcDiff = -1;
    mSffcNoise = 0.0f;
    //    //Y8修改
    //    bool ret = cal.Check_SFFC(pImage1, pImage2, WIDTH, HEIGHT, avg_value);
    //    ret &= cal.Check_pic_grad(pImage2, WIDTH, HEIGHT, avg_grad);

    //Y16修改
    bool ret = cal.Check_SFFCEx((unsigned short*)pImage1, (unsigned short*)pImage2, WIDTH, HEIGHT, mSffcDiff);
    ret &= cal.Check_pic_grad((unsigned short*)pImage1, WIDTH, HEIGHT, mSffcNoise);

    QString log = QString("检测结果 avg_value: %1, avg_grad: %2").arg(mSffcDiff).arg(mSffcNoise);
    writeLog(log);
    if (ret) {
        mesAddInfor(ItemSffc_CheckDiff, QString::number(mSffcDiff, 'f', 2), "PASS");
        mesAddInfor(ItemSffc_CheckNosie, QString::number(mSffcNoise, 'f', 2), "PASS");
    } else {
        mesAddInfor(ItemSffc_CheckDiff, QString::number(mSffcDiff, 'f', 2), "NG");
        mesAddInfor(ItemSffc_CheckNosie, QString::number(mSffcNoise, 'f', 2), "NG");
    }

    delete []pImage1;
    delete []pImage2;
    return ret;
}

void CDutRelative::slotPitsPreCal()
{
    mIntPitsProc = 0;
    mBoolPitsSupported = true;
    do {
        if (hasError()) break;

        if (!execProc("设置视频格式UYUV",
                      QStringList() << "视频格式" << "Y16" << "-",
                      std::bind(&CDutRelative::dutSetVideoFormat, this, 3)))
        {
            setErrString(-1, "图像格式YUYUV设置失败");
            break;
        }

        if (!execProc("关闭边缘增强", QStringList() << "关边缘增强" << "-" << "-",
                      std::bind(&CDutRelative::dutMRTDAlgorithm, this, Mrtd_EdgeEnhance, 0))) {
            setErrString(-4, "关闭边缘增强失败");
            break;
        }

        if (!execProc("关闭对比度增强", QStringList() << "关闭对比度增强" << "-" << "-",
                      std::bind(&CDutRelative::dutMRTDAlgorithm, this, Mrtd_ContrastEnhance, 0))) {
            setErrString(-4, "关闭对比度增强失败");
            break;
        }
    } while(0);
    gTestSteps++;
}

bool CDutRelative::dutMRTDAlgorithm(int id, int value)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.ChangeMrtdSwitch(id, value)) {
            return true;
        }

        QThread::msleep(500);
    }

    return false;
}

void CDutRelative::imgJudgeProc()
{
    while(mIntPitsProc != 0) {
        mImageCount = 0;
        mySleepSelf(5000);

        // 对出图的判断需要根据当前帧率来选择，相当于1/5帧率图片数量(延时5s)，就判定为出图正常
        // 1/5 随意取，主要考虑槽函数在处理图片时的消费，处理阶段会随机丢掉一部分图片
        int fps = mTransmitFps;

        // 2023年4月19日 xiangh BUG
        // mIntPitsProc != 0 此步判读很关键，Sleep时可能mIntPitsProc已经置为0了
        // 接收图片的槽函数会join()等待这个线程结束(join()前会主动将mIntPitsProc置0)，此时接收槽函数时阻塞的，图片数据处于停止接收状态
        if (mIntPitsProc != 0 && mImageCount <= fps) {
            setErrString(-2, "测试过程中图片数据异常");
            mIntPitsProc = 0;
            gTestSteps++;//结束执行
            return;
        }
    }
}

// 判断视频接收槽函数是否有图片出来，而且出图的门限判断为配置所设置的帧率数
// 10000ms的超时时间大约为设备的启动时间
bool CDutRelative::videoJudgeNormally()
{
    QElapsedTimer timer;
    timer.start();
    mImageCount = 0;
    while (timer.elapsed() < 70000) {
        QCoreApplication::processEvents();
        QThread::msleep(500);
        if (mImageCount > mTransmitFps)
            return true;
    }

    return false;
}

void CDutRelative::slotPitsCal(int step)
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    writeLog(QString("麻点校准标准，(%1, %2, %3)").arg(mPitsCalImageCount).arg(mPitsMaxFrame).arg(mPitsMaxValue));

    memset(m_szY, 0, WIDTH * HEIGHT);
    memset(m_szK, 0, WIDTH * HEIGHT * 2);
    writeTable(QStringList() << "图像帧数" << "-" << "-");

    mIntFrameCount = 0;
    mIntPitsCalStep = step;
    mIntPitsProc = CheckProc_PitsCal;
    mCheckProcImageCount = mPitsCalImageCount;
    mImgJudgeProc = new std::thread(&CDutRelative::imgJudgeProc, this);
}

void CDutRelative::getValueY(QByteArray data, unsigned char *&pretY)
{
    for (int foo = 0; foo < HEIGHT * WIDTH; foo++) {
        pretY[foo] = (unsigned char)data.at(2 * foo + 1);
    }
}

void CDutRelative::getValueY_short(QByteArray data, unsigned short * &pretY)
{
    for (int foo = 0; foo < HEIGHT * WIDTH; foo++) {
        pretY[foo] = data.at(2 * foo + 1);
    }
}

void CDutRelative::dutPitsGetBp(QByteArray data)
{
    // qDebug() << "find bp y8...";
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, info);
    if (info.format == VIDEO_FORMAT_UYUV) {
        // writeLog("UYUV格式查找坏点");
        memset (m_szY, 0, WIDTH * HEIGHT * sizeof(unsigned char));
        unsigned char* pY = m_szY;
        getValueY(data, pY);
        FindBPY8(m_szY, WIDTH, HEIGHT, m_szK, mPitsMaxValue, 1);
    } else if (info.format == VIDEO_FORMAT_Y16) {
        // writeLog("Y16格式查找坏点");
        memset (m_szY16, 0, WIDTH * HEIGHT * sizeof(unsigned short));
        memcpy((char*)m_szY16, data.data(), WIDTH * HEIGHT * sizeof (unsigned short));
        FindBPY16(m_szY16, WIDTH, HEIGHT, m_szK, mPitsMaxValue, 1);
    }
    // qDebug() << "find bp y8 over...";
}

int CDutRelative::dutPitsBpStatics(QString &list)
{
    POINTX szBPPointList[BP_NUM_MAX] = {0};
    QString mBpList;

    int bpCount = 0;
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, info);
    if (info.format == VIDEO_FORMAT_UYUV) {
        writeLog("UYUV格式获取坏点列表");
        bpCount = GetBPlist(szBPPointList, m_szK, WIDTH, HEIGHT, mPitsMaxFrame);
    } else if (info.format == VIDEO_FORMAT_Y16) {
        writeLog("Y16格式获取坏点列表");
        bpCount = GetBPlistY16(szBPPointList, m_szK, WIDTH, HEIGHT, mPitsMaxFrame);
    }
    qDebug() << "current bp count is : " << bpCount;
    writeLog("current bp count is : " + QString::number(bpCount));
    for (int foo = 0; foo < bpCount; foo++) {
        QString log = QString("BP_x: %1, BP_y: %2").arg(szBPPointList[foo].x).arg(szBPPointList[foo].y);
        writeLog(log);
        mBpList.append(QString("%1,%2").arg(szBPPointList[foo].x).arg(szBPPointList[foo].y));
        if(foo != bpCount-1) {
            mBpList.append(";");
        }
    }
    list = mBpList;
    return bpCount;
}

void CDutRelative::slotPitsAfterCal()
{
    do {
        if (hasError()) break;
        if (!execProc("保存坏点", QStringList() << "保存坏点" << "-" << "-",
                      std::bind(&CDutRelative::dutBPSave, this))) {
            setErrString(-4, "保存坏点失败");
            break;
        }

        int bpCount = 0;
        if (dutGetBpCount(bpCount)) {
            writeLog("获取坏点列表个数为：" + QString::number(bpCount));
        }

        if (bpCount < 100) {
            QString list;
            if (!execProc("读取坏点列表", QStringList(),
                          std::bind(&CDutRelative::dutGetBPListFromDUT, this, std::ref(list)))) {
                setErrString(-4, "读取坏点列表失败");
                break;
            }
            writeLog("读取坏点列表：" + list);
        }

        if (!execProc("开启对比度增强", QStringList() << "开启对比度增强" << "-" << "-",
                      std::bind(&CDutRelative::dutMRTDAlgorithm, this, Mrtd_ContrastEnhance, 1))) {
            setErrString(-4, "开启对比度增强失败");
            break;
        }

        if (!execProc("开启边缘增强", QStringList() << "开边缘增强" << "-" << "-",
                      std::bind(&CDutRelative::dutMRTDAlgorithm, this, Mrtd_EdgeEnhance, 1))) {
            setErrString(-4, "开启边缘增强失败");
            break;
        }
    } while(0);
    gTestSteps++;
}

void CDutRelative::slotPitsCheck()
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    memset(m_szY, 0, WIDTH * HEIGHT);
    memset(m_szK, 0, WIDTH * HEIGHT);
    writeTable(QStringList() << "图像帧数" << "-" << "-");
    mIntFrameCount = 0;
    mIntPitsProc = CheckProc_PitsCheck;

}

void CDutRelative::dutPitsCheckBp(QByteArray data)
{
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, info);
    if (info.format == VIDEO_FORMAT_UYUV) {
        unsigned char* pY = m_szY;
        getValueY(data, pY);
        CheckBPY8(m_szY, WIDTH, HEIGHT, m_szK, 50, 1);
    } else if (info.format == VIDEO_FORMAT_Y16) {
        CheckBPY16(m_szY16, WIDTH, HEIGHT, m_szK, 50, 1);
    }
}

bool CDutRelative::dutSaveImage(QString name, QByteArray data)
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
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, vinfo);

    unsigned char* m_frameData = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_rgbData = new unsigned char[WIDTH * HEIGHT * 4];
    unsigned char* m_y8Data = new unsigned char[WIDTH * HEIGHT * 4];
    memset (m_frameData, 0, WIDTH * HEIGHT * 4);
    memset (m_rgbData, 0, WIDTH * HEIGHT * 4);
    memset (m_y8Data, 0, WIDTH * HEIGHT * 4);

    VideoFormatInfo videoInfo;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, videoInfo);
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
        if (image.save(path, "jpg")) {

            QFile file(path + ".raw");
            if (file.open(QIODevice::WriteOnly)) {
                file.write(btArray.mid(640 * 4, 640 * 516 * 2));
                file.close();

                delete []m_rgbData;
                delete []m_frameData;
                delete []m_y8Data;
                return true;
            } else {
                writeLog("保存原始图像数据失败 " + path, -1);
            }
        }
    }
    writeLog("image save to " + path + " fail");
    delete []m_rgbData;
    delete []m_frameData;
    delete []m_y8Data;
    return false;
}

bool CDutRelative::dutSaveTimeStdBuffer()
{
    QString imgDir = mAsicSN;
    imgDir = imgDir.replace("*", "_");
    QString path = mTmpLogPath + imgDir + ".time_std" + QString::number(m_timeStdStep);
    QFile file(path);
    writeLog("时域图像数据保存至: " + path);
    if (file.open(QIODevice::WriteOnly)) {
        int size = file.write((char*)m_timeStdImageBuffer, LENGTH_128 * WIDTH * HEIGHT * sizeof(unsigned short));
        qDebug() << "has write video data size: " << size;
        file.close();
        writeLog("时域图像数据保存OK");
        return true;
    } else {
        writeLog("保存原始图像数据失败 " + path, -1);
    }
    return false;
}

bool CDutRelative::dutCloseAllAlg(bool bClose)
{
    bool bRst = false;
    for (int id = 1; id < 10; id++) {
        bRst |= dutMRTDAlgorithm(id, bClose ? 0 : 1);
    }

    return bRst;
}

void CDutRelative::dutHvStripesGetOnce(QByteArray data)
{
    memset (m_szY16, 0, WIDTH * HEIGHT * sizeof(unsigned short));
    unsigned short* input_data_y8 = (unsigned short*)m_szY16;

    getValueY_short(data, input_data_y8);

    double value1[1] = {0.0};
    double value2[1] = {0.0};
    double value3[1] = {0.0};
    imagecheck Imagecheck;
    Imagecheck.Evaluate_HorStripe(value1, value2, value3, input_data_y8, WIDTH, HEIGHT);
    m_hvValueStatics += value1[0];
    m_hvValueExpectMax += value2[0];
    m_hvValueExpectMin += value3[0];
    QString log = QString("value1: %1, value2: %2, value3: %3")
            .arg(QString::number(value1[0], 'f', 6))
            .arg(QString::number(value2[0], 'f', 6))
            .arg(QString::number(value3[0], 'f', 6));
    writeLog(log);
}

double CDutRelative::dutHvStripsGetResult()
{
    double reasultValue = m_hvValueStatics / m_hvCheckImgCount;
    double expectValueMin = m_hvValueExpectMin / m_hvCheckImgCount;
    double expectValueMax = m_hvValueExpectMax / m_hvCheckImgCount;
    double value = reasultValue * 0.2 + (abs(expectValueMin) + abs(expectValueMax)) * 0.8 / 2;
    return value;
}

void CDutRelative::slotHvStripsCheck(int step)
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    writeLog("横竖纹检测");

    if (!execProc("设置UYVY传输", QStringList(),
                  std::bind(&CDutRelative::dutSetVideoFormat, this, 1), 1000)) {
        setErrString(-4, "设置UVUY传输失败");
        gTestSteps++;
        return;
    }
     if (!execProc("一键关算法", QStringList(),
                   std::bind(&CDutRelative::dutCloseAllAlg, this, true), 1000)) {
         setErrString(-4, "一键关算法失败");
         gTestSteps++;
         return;
     }

     if (!execProc("关自适应补偿", QStringList() << "关自适应补偿" << "-" << "-",
                   std::bind(&CDutRelative::dutCloseAdaptive, this), 1000)) {
         setErrString(-4, "自适应关闭失败");
         gTestSteps++;
         return;
     }

    if (!execProc("快门补偿", QStringList(),
                  std::bind(&CDutRelative::dutShutterComp, this), 1000)) {
        setErrString(-4, "快门补偿失败");
        gTestSteps++;
        return;
    }

    m_hvValueStatics  = .0f;
    m_hvValueExpectMax = .0f;
    m_hvValueExpectMin = .0f;
    m_hvStripsValue = .0f;

    writeTable(QStringList() << "横竖纹检测" << "-" << "-");

    mIntFrameCount = 0;
    mCheckProcImageCount = m_hvCheckImgCount;
    // mIntPitsCalStep = step; 目前只需要再60度检测即可
    mIntPitsProc = CheckProc_HvStripeCheck;
    mImgJudgeProc = new std::thread(&CDutRelative::imgJudgeProc, this);
}

void CDutRelative::slotStd(int step)
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    if (!execProc("设置Y16传输", QStringList(),
                  std::bind(&CDutRelative::dutSetVideoFormat, this, 3), 1000)) {
        setErrString(-4, "设置Y16传输失败");
        gTestSteps++;
        return;
    }

    // 取得高斯算子
    imagecheck imgCheck;
    double sigma = 50.0f;
    memset(m_stdGausKernel , 0, sizeof(double) * GAUSSIAN_SIZE * GAUSSIAN_SIZE);
    double* gausKernel = (double*)m_stdGausKernel;
    if (0 != imgCheck.Gaussian(gausKernel, sigma)) {
        setErrString(-9, "高斯算子获取失败");
        gTestSteps++;
        return;
    }

    writeLog("STD检测");
    writeTable(QStringList() << "STD检测" << "-" << "-");

    if (step == 1) { // 不要清掉之前的数据
        m_stdSpaceStep1 = std::make_tuple(.0f, .0f);
    } else if (step == 2) {
        m_stdSpaceStep2 = std::make_tuple(.0f, .0f);
    }
    m_spaceStdStep = step;

    mIntFrameCount = 0;
    mCheckProcImageCount = m_spaceStdCheckImgCount;
    // mIntPitsCalStep = step; 目前只需要再60度检测即可
    mIntPitsProc = CheckProc_StdCheck;
    mImgJudgeProc = new std::thread(&CDutRelative::imgJudgeProc, this);
}

bool CDutRelative::dutStdGetOnce(QByteArray data)
{
    VideoFormatInfo fmt;
    VideoInfo::getInstance()->getVideoInfo(mSlotId - 1, fmt);

    // 原始数据
    unsigned short* src_data = new unsigned short[WIDTH * HEIGHT]; // 原始帧
    unsigned short* low_data = new unsigned short[WIDTH * HEIGHT];//低频帧
    float* high_data = new float[WIDTH * HEIGHT];//低频帧
    float* src_data_bak = new float[WIDTH * HEIGHT];//原始帧存储
    memset(src_data, 0, WIDTH * HEIGHT * sizeof(unsigned short));
    memset(low_data, 0, WIDTH * HEIGHT * sizeof(unsigned short));
    memset(high_data, 0, WIDTH * HEIGHT * sizeof(float));
    memset(src_data_bak, 0, WIDTH * HEIGHT * sizeof(float));

    memcpy((unsigned char*)src_data, data.data() + fmt.nvsLen, WIDTH * HEIGHT * sizeof(unsigned short));

    bool bRst = false;
    imagecheck imgCheck;
    if (imgCheck.Filter(m_stdGausKernel, src_data, low_data, high_data, src_data_bak) != 0) {
        setErrString(-9, "STD filter 失败");
    } else {
        m_stdSpace += imgCheck.Std2(src_data_bak); //窗口统计
        m_stdSpaceHigh += imgCheck.Std2(high_data);  //去低频后的高频窗口统计
        bRst = true;
    }


    delete []src_data;
    delete []low_data;
    delete []high_data;
    delete []src_data_bak;

    return bRst;
}

void CDutRelative::dutStdGetResult()
{
    m_stdSpace /= m_spaceStdCheckImgCount;
    m_stdSpaceHigh /= m_spaceStdCheckImgCount;
}

void CDutRelative::slotTimeStdRaiseTemp()
{
    do {
        if (hasError()) break;

        if (!dutSffcEnable(0.5)) {
            break;
        }
    } while(0);
    gTestSteps++;
}

void CDutRelative::slotTimeStd(int step)
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    if (!execProc("设置Y16传输", QStringList(),
                  std::bind(&CDutRelative::dutSetVideoFormat, this, 3), 1000)) {
        setErrString(-4, "设置Y16传输失败");
        gTestSteps++;
        return;
    }


    writeLog("Time STD检测");
    writeTable(QStringList() << "TimeSTD检测" << "-" << "-");

    if (step == 1) { // 不要清掉之前的数据
        m_timeStdValue = .0f;
    } else if (step == 2) {
        m_timeStdValue2 = .0f;
    }
    m_timeStdStep = step;

    memset(m_timeStdImageBuffer, 0, LENGTH_128 * WIDTH * HEIGHT * sizeof(unsigned short));

    mIntFrameCount = 0;
    mCheckProcImageCount = m_timeStdCheckImgCount;
    mIntPitsProc = CheckProc_TimeStdCheck;
    mImgJudgeProc = new std::thread(&CDutRelative::imgJudgeProc, this);
}

void CDutRelative::slotRestart()
{
    if (hasError()) {
        gTestSteps++;
        return;
    }

    do {
        if (!dutPing()) {
            setErrString(-1, "Ping 待测工装失败");
            mBoolIngoreMes = true;
            break;
        }

        mySleep(15000);

        // 帧率传输要放在视频连接之前
        writeLog("开始设置传输帧率：" + QString::number(mTransmitFps));
        if (!execProc("设置帧率",
                      QStringList() << "传输帧率" << QString::number(mTransmitFps) << "-",
                      std::bind(&CDutRelative::dutSetFps, this), 5000))
        {
            setErrString(-1, "帧率设置失败");
            break;
        }

        if (!execProc("连接视频", QStringList() << "连接视频" << "-" << "-",
                      std::bind(&CDutRelative::dutPlayVideo, this)))
        {
            setErrString(-1, "连接视频失败!");
            break;
        }
        mySleep(6000);

        float tmp;
        if (!dutGetFocusTemp(tmp)) {
            setErrString(-1, "读取焦温失败");
            break;
        } else {
            mFocusTempFirst = tmp;
            writeLog("当前焦温为：" + QString::number(mFocusTempFirst, 'f', 2));
        }

        if (!execProc("开启快门",
                      QStringList(),
                      std::bind(&CDutRelative::dutOpenShutter, this), 1000))
        {
            setErrString(-1, "开快门失败");
            break;
        }

        // 0 白热
        if (!execProc("设置白热模式",
                      QStringList() ,
                      std::bind(&CDutRelative::dutSetPallte, this, 0), 1000))
        {
            setErrString(-1, "白热模式设置失败");
            break;
        }

        // 确定是否出图OK
        bool brst = false;
        writeLog("检测是否出图正常");
//        for (int foo = 0; foo < 20; foo++) {
//            if (!dutSaveImage("开机图片.jpg")) {
//                QThread::msleep(500);
//                continue;
//            }
//            brst = true;
//            break;
//        }

        if (!videoJudgeNormally()) {
            setErrString(-1, "视频出图失败");
            break;
        }
    } while (0);
    gTestSteps++;
}

bool CDutRelative::dutSendCustom(int addr, int value)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.SendCustom(addr, value)) {
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

bool CDutRelative::dutReadCustom(int addr, int &value)
{
    for (int foo = 0; foo < 3; foo++) {
        AsicControl asic(mArtOsynIp);
        if (asic.ReadCustom(addr, value)) {
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

bool CDutRelative::dutGetRegister_i2c(int page, int addr, int &data) {
    if (!dutSendCustom(REGISTER_WRITE_6F0, page)) {
        return false;
    }
    mySleep(50);
    if (!dutSendCustom(REGISTER_WRITE_6F1, addr)) {
        return false;
    }
    mySleep(50);

    if (!dutReadCustom(REGISTER_READ_6F2, data)) {
        return false;
    }

    return true;
}

bool CDutRelative::dutGetRegister_com(QString &ret)
{
    bool bRst = false;
    AsicControl asic(mArtOsynIp);
    // 获取探测器配置页
    QString sCmd = "55 AA 07 A0 00 80 00 00 00 00";
    bRst = asic.SendCustomUart(sCmd);

    return bRst;
}

bool CDutRelative::recordRegisterData_i2c()
{
    int data = 0;
    if (!dutGetRegister_i2c(REGISTER_PAGE_3, REGISTER_ADDR_9, data)) {
        setErrString(-1, "读取HSSD数据失败");
        return false;
    }
    m_dataHssd = data;
    writeLog("读取到HSSD数据：" + QString::number(data));


    int data_h = 0, data_l = 0;
    if (!dutGetRegister_i2c(REGISTER_PAGE_3, REGISTER_ADDR_32, data_l)) {
        setErrString(-1, "读取HSSD_LOW数据失败");
        return false;
    }
    writeLog("读取到RASEL低位数据：" + QString::number(data_l), 1);
    m_dataRaselLow = data_l;

    if (!dutGetRegister_i2c(REGISTER_PAGE_3, REGISTER_ADDR_33, data_h)) {
        setErrString(-1, "读取HSSD_HIGH数据失败");
        return false;
    }
    writeLog("读取到RASEL高位数据：" + QString::number(data_h), 1);
    m_dataRaselHigh = data_h;

    int dataValue = (data_h << 8) & 0xFF00 | (data_l & 0x00FF);
    writeLog("转换RASEL高低位记录为：" + QString::number(dataValue), 1);

    QString dataStr = QString::number(dataValue, 2);
    writeLog("转换RASEL二进制值为：" + dataStr);
    int count_1 = 0, count_0 = 0;
    for (int foo = 0; foo < dataStr.size(); foo++) {
        if (dataStr.at(foo) == '1') {
            count_1++;
        } else if (dataStr.at(foo) == '0') {
            count_0++;
        }
    }
    m_dataRasel = count_1;
    writeLog("转换RASEL值为：" + QString::number(m_dataRasel));
    if (count_0) {
        writeLog("读取数据存在非连续1，RASEL读取失败");
        setErrString(-1, "读取数据存在非连续1，RASEL读取失败");
        return false;
    }

    return true;
}

void CDutRelative::slotSaveTemplate()
{
    // 2 UYVY 3 Y16 1 YUYV
    // 2023年4月11日 xiangh 因保存K和SFFC只能在YUV传输模式下面，而校K的动作在Y16下面，避免频繁切换，将保存的动作放在了测试结束进行
    if (mBoolCalKOk || mBoolCalSffcOk) {
        writeLog("设置UYUV");
        if (!dutSetVideoFormat(1)) {
            setErrString(-1, "图像格式UYUV失败, 放弃K文件保存");
        } else {
            QThread::sleep(2);
            if (mBoolCalKOk) {
                QString dirName = mAsicSN;
                dirName = dirName.replace("*", "_");
                QString fileName = dirName + "_k.raw";

                writeLog("保存K文件到" + fileName);
                if (!dutSaveKFile(0x04, fileName)) {
                    writeLog("保存K文件失败, 调试阶段：忽略", -1);
                    setErrString(-5, "K模板保存失败，请确认当前测试产品型号选择是否正确：" + m_projName);
                }
            }

            // 延时用来避免K文件传输未完成，被后面SFFC误接收
            if (mBoolCalKOk && mBoolCalSffcOk) {
                QThread::sleep(3);
            }

            if (mBoolCalSffcOk) {
                QString dirName = mAsicSN;
                dirName = dirName.replace("*", "_");
                QString fileName = dirName + "_sffc.raw\0";
                writeLog("保存SFFC模板文件到：" + fileName);
                if (!dutSaveKFile(0x05, fileName)) {
                    writeLog("保存SFFC文件失败, 调试阶段：忽略", -1);
                    setErrString(-5, "SFFC模板保存失败，请确认当前测试产品型号选择是否正确：" + m_projName);
                }
            }
        }
    }
}

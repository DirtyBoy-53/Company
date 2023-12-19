#include "NonUniformDE1008.h"
#include <functional>
#include <QMessageBox>
#include <inputmessagebox.h>
#include <QElapsedTimer>
#include "XyScanDialog.h"
#include <QCoreApplication>
#include "csvlogger.h"
#include <QDateTime>
#include <QDir>
#include <videoinfo.h>
#include <StaticSignals.h>

// mes: http://192.168.110.56:31453/mes-web/work/BasicMaterialInformation/snRetroSpect/index
// mes: http://192.168.110.23:32380/mes-web/work/BasicMaterialInformation/snRetroSpect/index

#define FALSE_RETURN(x) do{ \
    if (!(x)) { \
        return; \
    } \
}while(0);

#define INVALID_VALUE -99999

NonUniform::NonUniform()
{
    QObject::connect(StaticSignals::getInstance(), &StaticSignals::kStartCollectY16, this, &NonUniform::slotRecordFTData);
    QObject::connect(StaticSignals::getInstance(), &StaticSignals::kStopCollectY16, this, &NonUniform::slotStopRecordFTData);

    mTecDevice = new CTecRelative;
    mPlcDevice = new CPlcRelative;
    mScanDlg = new XyScanDialog;
    for (int foo = 0; foo < MAXDUT; foo++) {
        mVideoSocket[foo] = new WebSocketClientManager;
        if (foo == 0) QObject::connect(mVideoSocket[foo], &WebSocketClientManager::signalRecvVideoFrame,
                         this, &NonUniform::slotVideoSlot1);
        if (foo == 1) QObject::connect(mVideoSocket[foo], &WebSocketClientManager::signalRecvVideoFrame,
                         this, &NonUniform::slotVideoSlot2);
        if (foo == 2) QObject::connect(mVideoSocket[foo], &WebSocketClientManager::signalRecvVideoFrame,
                         this, &NonUniform::slotVideoSlot3);
        if (foo == 3) QObject::connect(mVideoSocket[foo], &WebSocketClientManager::signalRecvVideoFrame,
                         this, &NonUniform::slotVideoSlot4);
        m_thread[foo] = nullptr;
        mDutPtrArray[foo] = nullptr;
    }

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
}

void NonUniform::getParameters()
{
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "HvSupport", mBoolHvSupport);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "TECSupport", mBoolTecSupported);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "PLCSupport", mBoolPlcSupported);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "KSupport", mBoolKSupport);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "BPSupport", mBoolBpSupport);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "PitsSupport", mBoolPitsSupport);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "SFFCSupport", mBoolSffcSupport);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "PitsSaveAllImage", mBoolPitsSaveAllImage);
    ConfigInfo::getInstance()->getValueString(m_StationName, "LogPath", mLogPath);
    ConfigInfo::getInstance()->getValueString(m_StationName, "TecCom", mTecSerial);
    ConfigInfo::getInstance()->getValueString(m_StationName, "ReadCom", mPlcReadSerial);
    ConfigInfo::getInstance()->getValueString(m_StationName, "WriteCom", mPlcWriteSerial);
    ConfigInfo::getInstance()->getValueString(m_StationName, "BPfilePath", mBPFilePath);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "TecTargetTemp", mTecTargetTemp);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "FramePerSecond", mIntFps);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "PitsMaxValue", mPitsMaxValue);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "焦温限制", mFocusTempLimit);
    ConfigInfo::getInstance()->getValueString(m_StationName, "DUT1IP", mAddrList[0]);
    ConfigInfo::getInstance()->getValueString(m_StationName, "DUT2IP", mAddrList[1]);
    ConfigInfo::getInstance()->getValueString(m_StationName, "DUT3IP", mAddrList[2]);
    ConfigInfo::getInstance()->getValueString(m_StationName, "DUT4IP", mAddrList[3]);
    QString value;
    ConfigInfo::getInstance()->getValueString(m_StationName, "BpFileGetMethod", value); // 0 从MES获取，1 从本底获取， 2 不写坏点列表
    if (value.indexOf("MES获取") >= 0) {
        mIntBpFileWay = BpFileGet_Mes;
    } else if (value.indexOf("本地获取") >= 0) {
        mIntBpFileWay = BpFileGet_Local;
    } else { // 无坏点列表文件
        mIntBpFileWay = BpFileGet_None;
    }
    logNormal("HvSupport: " + QString::number(mBoolHvSupport));
    logNormal("TECSupport: " + QString::number(mBoolTecSupported));
    logNormal("PLCSupport: " + QString::number(mBoolPlcSupported));
    logNormal("KSupport: " + QString::number(mBoolKSupport));
    logNormal("BPSupport: " + QString::number(mBoolBpSupport));
    logNormal("SFFCSupport: " + QString::number(mBoolSffcSupport));
    logNormal("PitsSupport: " + QString::number(mBoolPitsSupport));
    logNormal("LogPath: " + mLogPath);
    logNormal("TecCom: " + mTecSerial);
    logNormal("ReadCom: " + mPlcReadSerial);
    logNormal("WriteCom: " + mPlcWriteSerial);
    logNormal("BPfilePath: " + mBPFilePath);
    logNormal("TecTargetTemp: " + QString::number(mTecTargetTemp));
    logNormal("FramePerSecond: " + QString::number(mIntFps));
    logNormal("DUT1IP: " + mAddrList[0]);
    logNormal("DUT2IP: " + mAddrList[1]);
    logNormal("DUT3IP: " + mAddrList[2]);
    logNormal("DUT4IP: " + mAddrList[3]);
    logNormal("BpFileGetMethod: " + QString::number(mIntBpFileWay));
}

bool NonUniform::dutPing(QString local, QString ip)
{
    int iContinue = 0;
    for (int foo = 0; foo < 20; foo++) {
        QCoreApplication::processEvents();
        if (CPingTest::pingTest(nullptr, ip.toLocal8Bit().data())) {
            iContinue++;
            if (iContinue > 3) {
                return true;
            }
        } else {
            iContinue = 0;
        }
    }

    return false;
}

void NonUniform::initSlotUI(int slot)
{
    logClear(slot);
    showDefault(slot);
    tHeader(QStringList() << "测试项目" << "测试内容" << "测试结果", slot);
    tClear(slot);
}

void NonUniform::addHvProcess()
{
    if (!mBoolHvSupport) return;
    logNormal("hv supported");
    // 环境温检测
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_Hv_Stripes", std::bind(&NonUniform::testHvStrips, this));
    add(mStateSequence++, "State_STD", std::bind(&NonUniform::testTimeStd, this, 1));
    add(mStateSequence++, "State_STD", std::bind(&NonUniform::testStd, this, 1));
    add(mStateSequence++, "State_TimeSTD_RaiseTemp", std::bind(&NonUniform::testTimeStdRaiseTemp, this));
    add(mStateSequence++, "State_STD", std::bind(&NonUniform::testStd, this, 2));
    // add(mStateSequence++, "State_Restart", std::bind(&NonUniform::testRestart, this));
}

void NonUniform::addKProcess()
{
    if (!mBoolKSupport) return;
    logNormal("k supported");
    // 校 K
    // add(mStateSequence++, "State_PLC_ZERO", std::bind(&NonUniform::plcMoveToZero, this));
    add(mStateSequence++, "State_DUT_PreK", std::bind(&NonUniform::testKPreCal, this));
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_DUT_BL", std::bind(&NonUniform::testBlTest, this));
    add(mStateSequence++, "State_PLC_60", std::bind(&NonUniform::plcMoveTo60, this));
    add(mStateSequence++, "State_DUT_BH", std::bind(&NonUniform::testBhTest, this));
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_DUT_KCHECK20", std::bind(&NonUniform::testBlCheck, this));
    add(mStateSequence++, "State_PLC_60", std::bind(&NonUniform::plcMoveTo60, this));
    add(mStateSequence++, "State_DUT_KCHECK60", std::bind(&NonUniform::testBhCheck, this));
}

void NonUniform::addBpProcess()
{
    if (!mBoolBpSupport) return;
    logNormal("bp supported");
    // 校 BP
    add(mStateSequence++, "State_DUT_PreBP", std::bind(&NonUniform::testBpPreCal, this));
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_DUT_BP20", std::bind(&NonUniform::testBpCal20, this));
    add(mStateSequence++, "State_PLC_60", std::bind(&NonUniform::plcMoveTo60, this));
    add(mStateSequence++, "State_DUT_BP60", std::bind(&NonUniform::testBpCal60, this));
// 2023年10月16日： 取消坏点检测，与麻点工位重复
//    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
//    add(mStateSequence++, "State_DUT_BPCHECK20", std::bind(&NonUniform::testBpCheck20, this));
//    add(mStateSequence++, "State_PLC_60", std::bind(&NonUniform::plcMoveTo60, this));
//    add(mStateSequence++, "State_DUT_BPCHECK60", std::bind(&NonUniform::testBpCheck60, this));
}

void NonUniform::addSffcProcess()
{
    if (!mBoolSffcSupport) return;
    logNormal("sffc supported");
    // 校 Sffc
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_DUT_PreSffc", std::bind(&NonUniform::testSffcPreCal, this));
    add(mStateSequence++, "State_PLC_Down", std::bind(&NonUniform::PlcFixtureDown, this));
    add(mStateSequence++, "State_PLC_Down", std::bind(&NonUniform::plcMoveTo20T, this));
    // add(mStateSequence++, "State_TEC_START", std::bind(&NonUniform::tecStart, this));
    add(mStateSequence++, "State_DUT_RaiseTemp", std::bind(&NonUniform::testSffcRaiseTemp, this));
    add(mStateSequence++, "State_DUT_Cal", std::bind(&NonUniform::testSffcCal, this));
    add(mStateSequence++, "State_DUT_Check", std::bind(&NonUniform::testSffcCheck, this));
}

void NonUniform::addPitsProcess()
{
    if (!mBoolPitsSupport) return;
    logNormal("pits supported");
    // 校 pits
    add(mStateSequence++, "State_DUT_PrePits", std::bind(&NonUniform::testPitsPreCal, this));
    add(mStateSequence++, "State_PLC_20", std::bind(&NonUniform::plcMoveTo20, this));
    add(mStateSequence++, "State_DUT_Cal1", std::bind(&NonUniform::testPitsCal, this, 21));
    add(mStateSequence++, "State_DUT_Cal2", std::bind(&NonUniform::testPitsCal, this, 22));
    add(mStateSequence++, "State_DUT_Cal3", std::bind(&NonUniform::testPitsCal, this, 23));
    add(mStateSequence++, "State_PLC_60", std::bind(&NonUniform::plcMoveTo60, this));
    add(mStateSequence++, "State_DUT_Cal1", std::bind(&NonUniform::testPitsCal, this, 61));
    add(mStateSequence++, "State_DUT_Cal2", std::bind(&NonUniform::testPitsCal, this, 62));
    add(mStateSequence++, "State_DUT_Cal3", std::bind(&NonUniform::testPitsCal, this, 63));
    add(mStateSequence++, "State_DUT_AfterCal", std::bind(&NonUniform::testPitsAfterCal, this));
}

void NonUniform::clearTestFlag()
{
    gTestSteps = 0;
}

void NonUniform::waitTestOver()
{
    while (gTestSteps < MAXDUT) {
        QCoreApplication::processEvents();
    }
}

void NonUniform::plcRstToUi(QString log, int status)
{
    for (int foo = 0; foo < MAXDUT; foo++) {
        if (gErrorCode[foo] == 0) {
            if (status == 0) {
                showProcess(log, foo);
                emit logAddNormalLog(log, foo);
            } else {
                showFail(log, foo);
                emit logAddFailLog(log, foo);
            }
        }
    }
}

void NonUniform::setAllErrString(int errCode, QString errInfo)
{
    plcRstToUi(errInfo, errCode);
    for (int foo = 0; foo < MAXDUT; foo++) {
        gErrorCode[foo] = errCode;
        gErrorString[foo] = errInfo;
    }
}


void NonUniform::start()
{
    m_pclRaised = false;
    m_fixtureDown = false;

    setAllErrString(0, "");
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigTestStart(mSnFromManual);
    waitTestOver();
}
void NonUniform::slotsStoped()
{
    qDebug() << ">>>>>>STOPPED<<<<<<<<<<<";
    slotStartWorkBtnClicked();
}
void NonUniform::stop()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigTestOver();
    waitTestOver();


    if (!isPressureMode())
        msgBox("更换产品并按Enter执行下一次测试", 4);
    stopProcess();
}

void NonUniform::testKPreCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigKPreCal();
    waitTestOver();
}

void NonUniform::testBlTest()
{
    qDebug() << __FUNCTION__;

    clearTestFlag();
    emit sigKCal20();
    waitTestOver();
}

void NonUniform::testBhTest()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigKCal60();
    waitTestOver();
}

void NonUniform::testBlCheck()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigKCheck20();
    waitTestOver();
}

void NonUniform::testBhCheck()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigKCheck60();
    waitTestOver();
}

void NonUniform::testBpPreCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigBPPreCal();
    waitTestOver();
}

void NonUniform::testBpCal20()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigBpCal20();
    waitTestOver();
}

void NonUniform::testBpCal60()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigBpCal60();
    waitTestOver();
}


void NonUniform::testBpCheck20()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigBpCheck20();
    waitTestOver();
}

void NonUniform::testBpCheck60()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigBpCheck60();
    waitTestOver();
}

void NonUniform::testSffcPreCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigSffcPreCal();
    waitTestOver();
}

void NonUniform::testSffcRaiseTemp()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigSffcRaiseTemp();
    waitTestOver();
}

void NonUniform::testTimeStdRaiseTemp()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigTimeStdRaiseTemp();
    waitTestOver();
}

void NonUniform::testSffcCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigSffcCal();
    waitTestOver();
}

void NonUniform::testSffcCheck()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigSffcCheck();
    waitTestOver();
}

void NonUniform::testPitsPreCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigPitsPreCal();
    waitTestOver();
}

void NonUniform::testPitsCal(int step)
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigPitsCal(step);
    waitTestOver();
}

void NonUniform::testPitsAfterCal()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigPitsAfterCal();
    waitTestOver();
}

void NonUniform::testPitsCheck()
{
    qDebug() << __FUNCTION__;
    clearTestFlag();
    emit sigPitsCheck();
    waitTestOver();
}

bool NonUniform::plcInit()
{
    if (!mBoolPlcSupported) {
        logNormal("PLC设备未添加，默认返回OK");
        return true;
    }

    if (!mPlcDevice) return false;

    //mPlcDevice->deInitResource();
    mPlcDevice->freeSerialResource();
    if (!mPlcDevice->getSerialResource()) {
        mErrString = "PLC获取串口句柄失败";
        return false;
    }
    logNormal("Read: " + mPlcReadSerial + " Write: " + mPlcWriteSerial);
    if (!mPlcDevice->initResource(mPlcWriteSerial, 9600, mPlcReadSerial, 9600)) {
        mErrString = mPlcDevice->errorString();
        return false;
    }

    logNormal("执行PLC复位");
    if (!mPlcDevice->plcReset()) {
        mErrString = "PLC 复位指令失败";
        return false;
    }

    mPlcDevice->boxPowerup();
    return true;
}

bool NonUniform::plcMoveReset()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC复位");
    if (!mPlcDevice->plcReset()) {
        mErrString = "PLC 复位失败";
        plcRstToUi("PLC复位异常", -1);
        return false;
    }
    return true;
}

bool NonUniform::plcMoveStop()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;
    return true;
}


bool NonUniform::plcMoveToZero()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC到原点");
    if (!mPlcDevice->plcMoveToOrign()) {
        //mErrString = "PLC 行进上下料位失败";
        //plcRstToUi("PLC到原点异常", -1);
        return false;
    }
    return true;
}

bool NonUniform::plcMoveTo20()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC到20", 0);
    for (int foo = 0; foo < MAXDUT; foo++) {
        showProcess("PLC移动到20度黑体位置", foo);
    }
    if (!mPlcDevice->plcMoveTo20()) {
        mErrString = "PLC 行进20度黑体处失败";
        plcRstToUi("PLC到20异常", -1);
        setAllErrString(-1, "PLC到20异常");
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
}

bool NonUniform::plcMoveTo60()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;


    plcRstToUi("PLC到60", 0);
    if (!mPlcDevice->plcMoveTo60()) {
        mErrString = "PLC 行进60度黑体处失败";
        plcRstToUi("PLC到60异常", -1);
        setAllErrString(-1, mErrString);
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
}

bool NonUniform::PlcMovePlain()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;
    if (!m_pclRaised) return true;

    plcRstToUi("PLC载具放平");
    if (!mPlcDevice->plcTurnBack()) {
        mErrString = "PLC 载具放平动作失败";
        plcRstToUi("PLC载具放平", -1);
        setAllErrString(-1, mErrString);
        return false;
    }
    m_pclRaised = false;
    return true;
}

bool NonUniform::PlcMoveRaise()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC载具抬起");
    if (!mPlcDevice->plcTurnOver()) {
        mErrString = "PLC 载具抬起动作失败";
        plcRstToUi("PLC载具抬起", -1);
        setAllErrString(-1, mErrString);
        return false;
    }

    m_pclRaised = true;
    return true;
}

bool NonUniform::plcMoveTo20T()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC到20贴合位置", 0);
    for (int foo = 0; foo < MAXDUT; foo++) {
        showProcess("PLC移动到20度贴合黑体位置", foo);
    }
    if (!mPlcDevice->plcMoveTo20T()) {
        mErrString = "PLC 行进20度贴合黑体处失败";
        plcRstToUi("PLC到20贴合异常", -1);
        setAllErrString(-1, mErrString);
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
}

bool NonUniform::plcMoveTo60T()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC到60贴合位置", 0);
    for (int foo = 0; foo < MAXDUT; foo++) {
        showProcess("PLC移动到60度贴合黑体位置", foo);
    }
    if (!mPlcDevice->plcMoveTo60T()) {
        mErrString = "PLC 行进60度贴合黑体处失败";
        plcRstToUi("PLC到60贴合异常", -1);
        setAllErrString(-1, mErrString);
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
}

bool NonUniform::PlcFixtureUp()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;
    if (!m_fixtureDown) return true;

    plcRstToUi("PLC气缸抬起");
    if (!mPlcDevice->plcFixtureUp()) {
        mErrString = "PLC 气缸抬起动作失败";
        plcRstToUi("PLC气缸抬起", -1);
        setAllErrString(-1, mErrString);
        return false;
    }
    m_fixtureDown = false;
    return true;
}

bool NonUniform::PlcFixtureDown()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC气缸下降");
    if (!mPlcDevice->plcFixtureDown()) {
        mErrString = "PLC 气缸下降动作失败";
        plcRstToUi("PLC气缸下降", -1);
        setAllErrString(-1, mErrString);
        return false;
    }
    m_fixtureDown = true;

    return true;
}

bool NonUniform::plcSetWorking(bool status)
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;

    plcRstToUi("PLC设置工作状态");
    if (!mPlcDevice->plcLightSetWork(status)) {
        mErrString = "PLC 设置工作状态失败";
        plcRstToUi("PLC设置工作状态", -1);
        return false;
    }
    return true;
}

bool NonUniform::plcCheckCoverStatus()
{
    if (!mBoolPlcSupported) return true;
    if (!mPlcDevice) return false;
    bool status = false;
    plcRstToUi("PLC获取产品锁状态");
    if (!mPlcDevice->plcLockStatus(status)) {
        mErrString = "PLC 获取产品锁状态失败";
        plcRstToUi("PLC获取产品锁状态", -1);
        setAllErrString(-1, "PLC获取产品锁状态");
        return false;
    }

    if (!status) {
        mErrString = "产品锁状态失败";
        plcRstToUi("产品锁状态", -1);
        setAllErrString(-1, "产品锁状态失败");
        return false;
    }

    return true;
}

bool NonUniform::tecInit()
{
    if (!mBoolTecSupported) {
        logNormal("TEC设备未添加，默认返回OK");
        return true;
    }

    if (!mTecDevice) return false;

    // mTecDevice->deInitResource();
    mTecDevice->freeTecResource();
    if (!mTecDevice->getTecResource()) {
        mErrString = "TEC关联串口DLL加载失败";
        return false;
    }
    if (!mTecDevice->initResource(mTecSerial)) {
        mErrString = mTecDevice->errorString();
        return false;
    }
//    if (!mTecDevice->tecStopHeating()) {
//        mErrString = "TEC预加热失败";
//        return false;
//    }
    if (!tecStart()) {
        mErrString = "TEC预加热失败";
        return false;
    }
    return true;
}

bool NonUniform::tecStart()
{
    bool brst = true;
    for (int foo = 0; foo < MAXDUT; foo++) {
        brst &= (gErrorCode[foo] != 0);
    }
    if (brst) return false;
    if (!mBoolTecSupported) return true;
    if (!mTecDevice) return false;


    plcRstToUi("TEC开始加热");
    if (!mTecDevice->tecStartHeating(mTecTargetTemp)) {
        mErrString = QString("TEC加热设置FAIL(目标%1度)").arg(mTecTargetTemp);
        plcRstToUi("TEC开始加热异常", 1);
        return false;
    }
    return true;
}

bool NonUniform::tecStop()
{
    if (!mBoolTecSupported) return true;
    if (!mTecDevice) return false;


    plcRstToUi("TEC停止加热");
    if (!mTecDevice->tecStopHeating()) {
        mErrString = "TEC停止加热设置FAIL";
        plcRstToUi("TEC停止加热异常", 1);
        return false;
    }
    return true;
}

void NonUniform::testRestart()
{
    bool bRst = false;
    for (int foo = 0; foo < 4; foo++) {
        slotStopVideo(foo);
        if (gErrorCode[foo] == 0) {
            bRst |= true;
        }
    }


    if (mBoolPlcSupported && bRst) {
        mPlcDevice->boxPoweroff();
        int continueTimes = 0;
        QString ip = mAddrList.at(0);
        for (int foo = 0; foo < 20; foo++) {
            if (!CPingTest::pingTest(nullptr, ip.toLocal8Bit().data())) {
                continueTimes++;
            } else {
                continueTimes = 0;
            }
            if (continueTimes >= 3) {
                logAddWarningLog("工装盒已下电，等待再次上电");
                break;
            }
            if (foo == 19) {
                setAllErrString(-2, "工装盒下电失败");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        mPlcDevice->boxPowerup();
    }
//    if (bRst)
//        msgBox("请对工装盒进行下电上电操作后按键Enter进行确认", 4);
    clearTestFlag();
    emit sigRestart();
    waitTestOver();
}

void NonUniform::testHvStrips()
{
    clearTestFlag();
    emit sigHvTest(0);
    waitTestOver();
}

void NonUniform::testStd(int step)
{
    clearTestFlag();
    emit sigStdTest(step);
    waitTestOver();
}

void NonUniform::testTimeStd(int step)
{
    clearTestFlag();
    emit sigTimeStdTest(step);
    waitTestOver();
}

void NonUniform::testTempratureOk()
{
    double fTecTemp = .0f;
    if (!mBoolTecSupported) return;
    if (!mTecDevice) {
        setAllErrString(-2, "TEC设备未初始化");
        return;
    }

    while (true) {
        QCoreApplication::processEvents();
        fTecTemp = mTecDevice->tecGetCurrentTempture();
        logNormal("TEC温度: " + QString::number(fTecTemp, 'f', 2));
        if ((fTecTemp < 37.5) && (fTecTemp > 0)){
            break;
        }
        showProcess("TEC温度还未稳定到37.5度以下，当前温度" + QString::number(fTecTemp, 'f', 2));
        logNormal("TEC温度还未稳定到37.5度以下，当前温度" + QString::number(fTecTemp, 'f', 2));
        Sleep(1000);
        continue;
    }

    msgBox("TEC温度已降至37.5度以下，请放入产品并按Enter按键继续测试", 4);
}

void NonUniform::slotPitsFrame(int frame_cnt, int slotid)
{
    emit tableUpdateData(QString::number(frame_cnt), 1, slotid);
}

void NonUniform::slotPitsCount(int pits_cnt, QString list, int slotid)
{
    emit tableUpdateData(QString::number(pits_cnt), 1, slotid);
    logNormal(list, slotid);
}

void NonUniform::slotSffcFocusTemp(float temp, int slotid)
{
    emit tableUpdateData(QString::number(temp, 'f', 2), 1, slotid);
}


void NonUniform::slotConnectBtnClicked()
{
    mAddrList.clear();
    mAddrList << "192.168.1.12"
              << "192.168.1.13"
              << "192.168.1.13"
              << "192.168.1.14";
    mErrString.clear();
    emit updateStatus(0);
    for (int foo = 0; foo < MAXDUT; foo++) {
        initSlotUI(foo);
        gErrorCode[foo] = 0;
        gErrorString[foo].clear();
    }

//    resetProcess();
    QObject::connect(pMachine(), &QStateMachine::stopped, this, &NonUniform::slotsStoped);

//    logNormal("加载配置文件");
//    if (!configXmlOpen()) {
//        showFail("配置文件不存在");
//        return;
//    }
    getParameters();

    // add(mStateSequence++, "State_TEC_40", std::bind(&NonUniform::testTempratureOk, this));

    add(mStateSequence++, "TestStart", std::bind(&NonUniform::start, this));
    add(mStateSequence++, "Life Time", std::bind(&NonUniform::jugdeCustomLifeTime, this, true));
    add(mStateSequence++, "State_PLC_ZERO", std::bind(&NonUniform::plcMoveToZero, this));
    add(mStateSequence++, "State_PLC_CheckCover", std::bind(&NonUniform::plcCheckCoverStatus, this));
    add(mStateSequence++, "State_TEC_START", std::bind(&NonUniform::tecStart, this));
    add(mStateSequence++, "State_PLC_RAISE", std::bind(&NonUniform::PlcMoveRaise, this));
    add(mStateSequence++, "State_PLC_SETWORK", std::bind(&NonUniform::plcSetWorking, this, true));
    addKProcess();
    addBpProcess();
    addSffcProcess();
    addHvProcess(); // 移动一下到所有数据完成之后
    addPitsProcess();
    // add(mStateSequence++, "State_TEC_STOP", std::bind(&NonUniform::tecStop, this));
    add(mStateSequence++, "State_PLC_ZERO", std::bind(&NonUniform::plcMoveToZero, this));
    add(mStateSequence++, "State_PLC_SETWORK", std::bind(&NonUniform::plcSetWorking, this, false));
    add(mStateSequence++, "State_PLC_PLAIN", std::bind(&NonUniform::PlcMovePlain, this));
    add(mStateSequence++, "State_PLC_Up", std::bind(&NonUniform::PlcFixtureUp, this));
    add(mStateSequence++, "TestOver", std::bind(&NonUniform::stop, this));

    logNormal("初始化TEC设备");
    if (!tecInit()) {
        plcRstToUi("TEC初始化失败", -1);
        return;
    }

    logNormal("初始化PLC串口资源");
    if (!plcInit()) {
        plcRstToUi("PLC初始化失败", -1);
        return;
    }

    bool bPass = true;

//    for (int foo = 0; foo < MAXDUT; foo++) {
//        if (!dutPing(nullptr, mAddrList[foo])) {
//            mErrString = "Ping 待测工装盒失败 - " + mAddrList[foo];
//            showFail(mErrString);
//            logFail(mErrString);
//            bPass = false;
//        }
//    }

    for (int foo = 0; foo < MAXDUT; foo++) {
        if (mDutPtrArray[foo]) {
            delete mDutPtrArray[foo];
            mDutPtrArray[foo] = nullptr;
        }
        mDutPtrArray[foo] = new CDutRelative;
        mDutPtrArray[foo]->setIpAddr(foo + 1, mAddrList[foo]);
        mDutPtrArray[foo]->setLogPath(mLogPath);
        mDutPtrArray[foo]->setBPFilePath(mBPFilePath);
        mDutPtrArray[foo]->setBpGetWay(mIntBpFileWay);
        mDutPtrArray[foo]->setTransmitFps(mIntFps);
        mDutPtrArray[foo]->setProduce(this);
        mDutPtrArray[foo]->setFocusTempLimit(mFocusTempLimit);
        mDutPtrArray[foo]->setPitsSaveAllImage(mBoolPitsSaveAllImage);
        mDutPtrArray[foo]->setProjectName(projectName());
        if (mBoolPitsSupport) {
            mDutPtrArray[foo]->setPitsTest(true, 1500, 30, mPitsMaxValue);
        } else {
            mDutPtrArray[foo]->setPitsTest(false);
        }
        if (isOnlineMode()) {
            mDutPtrArray[foo]->setMesEnable(true);
        } else {
            mDutPtrArray[foo]->setMesEnable(false);
        }
        mDutPtrArray[foo]->setSnRules(snGetMethod(), snGetLength(), snGetPrefix());

        // stat, stop
        QObject::connect(this, &NonUniform::sigTestStart, mDutPtrArray[foo], &CDutRelative::slotStartTest);
        QObject::connect(this, &NonUniform::sigTestOver, mDutPtrArray[foo], &CDutRelative::slotOverTest);

        // hv
        QObject::connect(this, &NonUniform::sigHvTest, mDutPtrArray[foo], &CDutRelative::slotHvStripsCheck);
        QObject::connect(this, &NonUniform::sigStdTest, mDutPtrArray[foo], &CDutRelative::slotStd);
        QObject::connect(this, &NonUniform::sigTimeStdTest, mDutPtrArray[foo], &CDutRelative::slotTimeStd);
        QObject::connect(this, &NonUniform::sigRestart, mDutPtrArray[foo], &CDutRelative::slotRestart);

        // K
        QObject::connect(this, &NonUniform::sigKPreCal, mDutPtrArray[foo], &CDutRelative::slotKPreCal);
        QObject::connect(this, &NonUniform::sigKCal20, mDutPtrArray[foo], &CDutRelative::slotKCal20);
        QObject::connect(this, &NonUniform::sigKCal60, mDutPtrArray[foo], &CDutRelative::slotKCal60);
        QObject::connect(this, &NonUniform::sigKCheck20, mDutPtrArray[foo], &CDutRelative::slotKCheck20);
        QObject::connect(this, &NonUniform::sigKCheck60, mDutPtrArray[foo], &CDutRelative::slotKCheck60);

        // BP
        QObject::connect(this, &NonUniform::sigBPPreCal, mDutPtrArray[foo], &CDutRelative::slotBpPreCal);
        QObject::connect(this, &NonUniform::sigBpCal20, mDutPtrArray[foo], &CDutRelative::slotBpCal20);
        QObject::connect(this, &NonUniform::sigBpCal60, mDutPtrArray[foo], &CDutRelative::slotBpCal60);
        QObject::connect(this, &NonUniform::sigBpCheck20, mDutPtrArray[foo], &CDutRelative::slotBpCheck20);
        QObject::connect(this, &NonUniform::sigBpCheck60, mDutPtrArray[foo], &CDutRelative::slotBpCheck60);

        // SFFC
        QObject::connect(this, &NonUniform::sigSffcPreCal, mDutPtrArray[foo], &CDutRelative::slotSffcPreCal);
        QObject::connect(this, &NonUniform::sigSffcRaiseTemp, mDutPtrArray[foo], &CDutRelative::slotSffcRaiseTemp);
        QObject::connect(this, &NonUniform::sigTimeStdRaiseTemp, mDutPtrArray[foo], &CDutRelative::slotTimeStdRaiseTemp);
        QObject::connect(this, &NonUniform::sigSffcCal, mDutPtrArray[foo], &CDutRelative::slotSffcCal);
        QObject::connect(this, &NonUniform::sigSffcCheck, mDutPtrArray[foo], &CDutRelative::slotSffcCheck);

        // Pits
        QObject::connect(this, &NonUniform::sigPitsPreCal, mDutPtrArray[foo], &CDutRelative::slotPitsPreCal);
        QObject::connect(this, &NonUniform::sigPitsCal, mDutPtrArray[foo], &CDutRelative::slotPitsCal);
        QObject::connect(this, &NonUniform::sigPitsAfterCal, mDutPtrArray[foo], &CDutRelative::slotPitsAfterCal);
        QObject::connect(this, &NonUniform::sigPitsCheck, mDutPtrArray[foo], &CDutRelative::slotPitsCheck);
        QObject::connect(mDutPtrArray[foo], &CDutRelative::sigFrameNumber, this, &NonUniform::slotPitsFrame);
        QObject::connect(mDutPtrArray[foo], &CDutRelative::sigPitsNumber, this, &NonUniform::slotPitsCount);
        QObject::connect(mDutPtrArray[foo], &CDutRelative::sigSffcFocusTemp, this, &NonUniform::slotSffcFocusTemp);

        // video
        QObject::connect(this, &NonUniform::videoUpdate, mDutPtrArray[foo], &CDutRelative::slotRecvVideoFrame);
        QObject::connect(mDutPtrArray[foo], &CDutRelative::sigPlayVideo, this, &NonUniform::slotPlayVideo);
        QObject::connect(mDutPtrArray[foo], &CDutRelative::sigStopVideo, this, &NonUniform::slotStopVideo);
        if (m_thread[foo]) {
            m_thread[foo]->terminate();
            delete m_thread[foo];
            m_thread[foo] = nullptr;
        }
        m_thread[foo] = new QThread(this);
        mDutPtrArray[foo]->moveToThread(m_thread[foo]);
        m_thread[foo]->start();
    }

    BaseProduce::slotConnectBtnClicked();
}

void NonUniform::slotStartWorkBtnClicked()
{
    qDebug() << "执行开始测试";
    mStartTimePoint = std::chrono::system_clock::now();
    for (int foo = 0; foo < MAXDUT; foo++) {
        initSlotUI(foo);
        gErrorCode[foo] = 0;
        gErrorString[foo].clear();
    }

    if (snGetMethod() == SnGetMothod_Manual)
    {
        mSnFromManual.clear();
        for (int foo = 0; foo < MAXDUT; foo++) {
            showProcess("请扫描探测器编码到框内", foo);
            mScanDlg->showModal("探测器编码");
            QString sn = mScanDlg->data();
            logNormal(sn, foo);
            showProcess(sn, foo);
            if (snGetLength() > 0) {
                if (sn.size() != snGetLength()) {
                    showFail("SN长度不对，请重新输入");
                    emit updateStatus(2);
                    return;
                }
                if (!snGetPrefix().isEmpty()) {
                    if (sn.indexOf(snGetPrefix()) != 0) {
                        showFail("SN前缀不对，请重新输入");
                        emit updateStatus(2);
                        return;
                    }
                }
            }
            mSnFromManual << sn;
        }
    }

    if (!isPressureMode())
        //msgBox("确认待测已经完全放入载具", 4);
        QMessageBox::information(nullptr, "注意", "确认待测已经完全放入载具");

//    double tecTemp = mTecDevice->tecGetCurrentTempture();
//    logNormal("当前TEC温度为：" + QString::number(tecTemp, 'f', 3));
//    QStringList list;
//    list << "TEC温度" << QString::number(tecTemp, 'f', 3) << "";
//    tRowData(list);

    startProcess();
}

void NonUniform::slotDisConnectBtnClicked()
{

}

void NonUniform::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
}

void NonUniform::slotVideoSlot1(QByteArray frameData, FrameInfo_S frameInfo)
{
    videoDistribute(0, frameData, frameInfo);
}

void NonUniform::slotVideoSlot2(QByteArray frameData, FrameInfo_S frameInfo)
{
    videoDistribute(1, frameData, frameInfo);
}

void NonUniform::slotVideoSlot3(QByteArray frameData, FrameInfo_S frameInfo)
{
    videoDistribute(2, frameData, frameInfo);
}

void NonUniform::slotVideoSlot4(QByteArray frameData, FrameInfo_S frameInfo)
{
    videoDistribute(3, frameData, frameInfo);
}

void NonUniform::videoDistribute(int id, QByteArray frameData, FrameInfo_S frameInfo)
{
    mIntPicCount[id]++;
    VideoFormatInfo info;
    VideoInfo::getInstance()->getVideoInfo(id, info);
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
    VideoInfo::getInstance()->setVideoInfo(id, info);

    emit videoUpdate(id, frameData, frameInfo.uTimeTm);
}

void NonUniform::slotPlayVideo(int slot)
{
    mVideoSocket[slot]->connectToUrl(mAddrList[slot], 6000);
}

void NonUniform::slotStopVideo(int slot)
{
    mVideoSocket[slot]->stopConnect();
}

// 算法需求，记录图像数据
void NonUniform::slotRecordFTData(int timeoutS)
{
    mBoolRecordPicData = true;
    std::thread(&NonUniform::recordDataTimeout, this, 0, timeoutS).detach();
    std::thread(&NonUniform::recordDataTimeout, this, 1, timeoutS).detach();
    std::thread(&NonUniform::recordDataTimeout, this, 2, timeoutS).detach();
    std::thread(&NonUniform::recordDataTimeout, this, 3, timeoutS).detach();
}

void NonUniform::slotStopRecordFTData()
{
    mBoolRecordPicData = false;
}

void NonUniform::recordDataTimeout(int slot, int timeoutS)
{
    logClear(slot);
    showProcess("", slot);
    mIntPicCount[slot] = 0;
    slotPlayVideo(slot);

    for (int foo = 0; foo < 100; foo++) {
        QCoreApplication::processEvents();
        if (mIntPicCount[slot] > 10) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (mIntPicCount[slot] < 10) {
        logAddFailLog("出图异常 slot" + QString::number(slot), slot);
        showFail("出图异常 slot" + QString::number(slot), slot);
        return;
    }
    AsicControl asic(mAddrList[slot]);
    logAddFailLog("设置帧率 " + QString::number(mIntFps), slot);
    if (!asic.setTransmitFps(mIntFps)) {
        logAddFailLog("设置帧率 " + QString::number(mIntFps), slot);
        showFail("设置帧率 " + QString::number(mIntFps), slot);
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));


    if (!asic.changeSensorFormat(3)) {
        logAddFailLog("切换Y16图像失败 slot" + QString::number(slot), slot);
        showFail("切换Y16图像失败 slot" + QString::number(slot), slot);
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    showProcess("开始记录图像数据", slot);
    logAddNormalLog("开始记录图像", slot);
    logAddNormalLog("超时时间为：" + QString::number(timeoutS), slot);

    QString path = mLogPath + "/";
    path += "slot" + QString::number(slot+1);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    startRecord(path, slot);

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < (timeoutS * 1000)) {
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!mBoolRecordPicData) {
            stopRecord(slot);
            logAddWarningLog("手动停止测试", slot);
            break;
        }
    }

    if (mBoolRecordPicData) {
        // mBoolRecordPicData = false;
        logAddWarningLog("测试完成", slot);
        showProcess("测试完成", slot);
        stopRecord(slot);
    }
}

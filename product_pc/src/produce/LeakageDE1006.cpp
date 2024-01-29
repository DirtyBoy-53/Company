#include "LeakageDE1006.h"
#include <functional>
#include <QMessageBox>
#include <inputmessagebox.h>
#include <QElapsedTimer>
#include "XyScanDialog.h"
#include <QCoreApplication>
#include "csvlogger.h"
#include <QDateTime>
#include <QDir>

#define FALSE_RETURN(x) do{ \
    if (!(x)) { \
        return false; \
    } \
}while(0);


#define ITEM_PRESSURE 0
#define ITEM_LEAKAGE  2

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item  value minvalue maxvalue exp result
    {"Leakage_pressure", "", "","30","-","NG"},
    {"Leakage_threshold", "", "","50","-","NG"},
    {"Leakage_value", "", "","50","-","NG"},
    {""},
};

#define INVALID_VALUE -99999

LeakageDE1006::LeakageDE1006()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &LeakageDE1006::slotsStoped);

    m_handleScanner = new CSerialDirect();
    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    qDebug() << "Current Station Name: " << m_StationName;
    mMesObj.setUser(MesClient::instance()->sUser());

//    add(0, "", std::bind(&LeakageDE1006::CycleInit, this));
//    add(10, "", std::bind(&LeakageDE1006::getSnAndExchangeDUT, this));
//    add(20, "", std::bind(&LeakageDE1006::readyToTest, this));
//    add(30, "", std::bind(&LeakageDE1006::leakageTest, this));
//    add(40, "", std::bind(&LeakageDE1006::CycleExit, this));
//    return;

    add(0, "StartTest", std::bind(&LeakageDE1006::step0, this));
    add(5, "Judge", std::bind(&LeakageDE1006::jugdeCustomLifeTime, this, true));
    add(11, "test1", std::bind(&LeakageDE1006::step1, this));
    add(12, "Test", std::bind(&LeakageDE1006::step2, this));
    add(13, "CloseSerial", std::bind(&LeakageDE1006::step3, this));

    mFailLimitPressure = 30;
    mFailLimitLeakage = 50;

    mScanDlg = new XyScanDialog;
    mScanDlg->hide();
}

LeakageDE1006::~LeakageDE1006()
{
    m_leakagePlc.close();
    m_leakageM5001A.close();
    m_leakagePlc.freeHandle();
    m_leakageM5001A.freeHandle();
}





void LeakageDE1006::step0()
{
    m_errorCode = 0;
    m_errMsg.clear();

    mLeakageValue = -999;
    mPressureValue = -999;

    mBoolMesIgnore = true;
    mStartTime = QDateTime::currentDateTime();
    mTimer.start();

    if (m_isNewDevice) {
        logNormal("当前为新的气密设备");
        if (!CycleInit()) {
            m_errMsg = "开启串口失败";
            m_errorCode = -1;
            addLog(m_errMsg, false);
            return;
        }

        if (!getSnAndExchangeDUT()) {
//            m_errorCode = -2;
//            m_errMsg = "获取SN，执行产品交换失败";
            addLog(m_errMsg, false);
            return;
        }
        setCurrentBarcode(m_sn); // 显示到界面
    }

    if (isOnlineMode()) {
        QString out;
        logNormal("入站：" + m_sn);
        if (!mMesObj.enterProduce(m_sn, out)) {
            m_errMsg = "入站失败: " + out ;
            emit logAddFailLog("入站失败：" + out);
            mBoolMesIgnore = true;
            m_errorCode = -1;
            return;
        }
        logNormal("入站：" + m_sn + " ok");
        mBoolMesIgnore = false;

        mesInforClear();
    } else {
        logWarming("当前为离线模式，不进行入站操作");
    }
}

void LeakageDE1006::mesInforClear()
{
    // mMesObj.clearItem();
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++) {
        MesCheckItem pitem = g_mesCheckPackage[foo];
        if (pitem.sItem.size() > 0) {
            pitem.sResult = "NG";
        }
    }
    g_mesCheckPackage[1].sValue = QString::number(mFailLimitLeakage);
    g_mesCheckPackage[1].sResult = "PASS";
}

void LeakageDE1006::mesAddInfor(int seq, QString value, QString rst, QString err)
{
    g_mesCheckPackage[seq].sValue = value;
    g_mesCheckPackage[seq].sResult = rst;
    g_mesCheckPackage[seq].sNote = err;
}

void LeakageDE1006::mesUpload()
{
    for (int foo = 0; foo < MESMAXCHECKNUM; foo++ ) {
        if (g_mesCheckPackage[foo].sItem.size() > 0) {
            mMesObj.addItemResultEx(g_mesCheckPackage[foo]);
        }
    }
}
// 开串口
void LeakageDE1006::step1()
{
    if (m_errorCode != 0) {
        return;
    }

    if (m_isNewDevice) {
        // 新设备在这里执行产品到位检测，工装设备底盘上升，顶部气缸下压的动作
        if (!readyToTest()) {
            m_errorCode = -3;
            m_errMsg = "PLC设备到位准备FAIL";
        }
        return; // 后面都是旧设备的事情，此处返回
    }

    // 1. 初始化串口，更新step和log
    emit logAddNormalLog("初始化串口, " + mSerialName);
    emit resultShowProcess(QString("初始化串口，") + mSerialName);
    if (!mLeakageBase.airInit(mSerialName, 9600)) {
        m_errMsg = mLeakageBase.errorString();
        emit logAddFailLog(m_errMsg);
        mBoolMesIgnore = true;
        m_errorCode = -1;
        return;
    }

    // 2. 气密仪复位，更新step和log
    emit logAddNormalLog("气密仪复位");
    emit resultShowProcess("气密仪复位");
    if (!mLeakageBase.airReset()) {
        m_errMsg = "气密复位失败，" + mLeakageBase.errorString();
        mLeakageBase.airDeInit();
        emit logAddFailLog(m_errMsg);
        mBoolMesIgnore = true;
        m_errorCode = -1;
        return;
    }

    // 3. 气密仪测试开始指令，更新step和log
    emit logAddNormalLog("气密仪开启测试");
    emit resultShowProcess("气密仪开启测试");
    if (!mLeakageBase.airStart()) {
        m_errMsg = "气密开启测试失败";
        emit logAddFailLog(m_errMsg);
        mLeakageBase.airDeInit();
        mBoolMesIgnore = true;
        m_errorCode = -1;
        return;
    }
}

void LeakageDE1006::step2()
{
    if (m_errorCode != 0) return;

    if (m_isNewDevice) {
        if (!leakageTest()) {
            m_errorCode = -4;
            m_errMsg = "泄露测试失败";
        }
        return;
    }

    ParamOutAir outData;
    float iFirstPump = INVALID_VALUE;
    float iFirstPressure = INVALID_VALUE, iFirstTime = 0;
    float iFirstLeakage = INVALID_VALUE;
    emit logAddNormalLog("开始监测气压数据");
    emit resultShowProcess("测试气压数据监测");
    m_errorCode  = 0;
    int mTimeout = 30;
    QElapsedTimer timer;
    timer.start();
    while(timer.elapsed() < mTimeout * 1000) {
        QCoreApplication::processEvents();
        QThread::msleep(100);
        // emit logAddNormalLog("开始监测气压数据");

        if (!mLeakageBase.airReadRealResult(outData)) {
            m_errMsg = "Read Real Data Failed";
            emit logAddFailLog(m_errMsg);
            mLeakageBase.airReset();
            m_errorCode = -2;//
            break;
        }

        QString stepStr = "Unknown step";
        if (outData.AirTestStep == 4) {
            stepStr = "充气阶段";
            emit resultShowProcess(stepStr);

            QStringList list;
            double time = timer.elapsed() / 1000;
            if (time > 5) time = 5;
            list << "充气时间(s)" << "-" << "5" << QString::number(time, 'f', 2) << "TEST";
            if (iFirstPump == INVALID_VALUE) {
                emit tableAddRowData(list);
                emit tableUpdateTestStatus(2);
                iFirstPump = 0;
            } else {
                emit tableUpdateData(QString::number(time, 'f', 2), 3);
            }
        } else if (outData.AirTestStep == 5) {
            stepStr = "大漏测试";
            emit resultShowProcess(stepStr);
            if (fabs(iFirstPressure - INVALID_VALUE) < 1e-8) {
                emit tableUpdateTestStatus(0);

                iFirstPressure = outData.AirPressure;
                iFirstTime = timer.elapsed();
                QStringList list;
                list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(0, 'f', 2) << "TEST";
                emit tableAddRowData(list);
                emit tableUpdateTestStatus(2);
                emit chartAddData(0, 0, 0, 0);
            } else {
                qDebug() << "iFirst " << iFirstPressure;
                float iPressure = (iFirstPressure - outData.AirPressure) / 1000;
                float iTime = (timer.elapsed() - iFirstTime) / 1000 + 1;

                emit chartAddData(iTime, iPressure, 0, 0);
                emit tableUpdateData(QString::number(iPressure, 'f', 2), 3);
                mPressureValue = iPressure;
                QStringList list;
                // 判断小
                if ((mFailLimitPressure < iPressure) && fabs(iPressure - mFailLimitPressure) > 1e-8) {
                    mLeakageBase.airReset();
                    m_errMsg = "大漏测试失败";
                    list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(iPressure, 'f', 2) << "FAIL";
                    QString strLog = QString("大漏测试，时间点：%1 s, 大漏泄露值：%2 Pa(上限：%3)").arg(iTime).arg(QString::number(iPressure, 'f', 2)).arg(mFailLimitPressure);
                    emit logAddFailLog(strLog);

                    mesAddInfor(ITEM_PRESSURE, QString::number(mPressureValue, 'f', 2), "NG", m_errMsg);
                    // table fail变红背景
                    emit tableUpdateTestStatus(1);
                    m_errorCode = -2;
                    break;
                } else {
                    list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(iPressure, 'f', 2) << "PASS";
                    QString strLog = QString("大漏测试，时间点：%1 s, 大漏泄露值：%2 Pa(上限：%3) FAIL").arg(iTime).arg(QString::number(iPressure, 'f', 2)).arg(mFailLimitPressure);
                    emit logAddNormalLog(strLog);
                }
                // 更新压力值数据
            }

        } else if (outData.AirTestStep == 6) {
            stepStr = "微漏测试";
            emit resultShowProcess(stepStr);
            QStringList list;
            float iLeakage = outData.AirLeakage / 1000;
            float iTime = (timer.elapsed() - iFirstTime) / 1000 + 1;
            mLeakageValue = iLeakage;
            if (fabs(iFirstLeakage - INVALID_VALUE) < 1e-8) {
                iFirstLeakage = outData.AirLeakage;
                iFirstTime = timer.elapsed();
                emit chartAddData(0, 0, 1, 0);

                emit tableUpdateTestStatus(0);
                mesAddInfor(ITEM_PRESSURE, QString::number(mPressureValue, 'f', 2), "PASS");

                list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(iLeakage, 'f', 2) << "FAIL";
                emit tableAddRowData(list);
                emit tableUpdateTestStatus(TableViewShowTest);
            } else {
                QStringList list;
                emit chartAddData(iTime, iLeakage, 1, 0);
                emit tableUpdateData(QString::number(iLeakage, 'f', 2), 3);
                if ((mFailLimitLeakage < iLeakage) && fabs(iLeakage - mFailLimitLeakage) > 1e-8) {
                    mLeakageBase.airReset();
                    m_errMsg = "微漏测试失败";
                    list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(iLeakage, 'f', 2) << "FAIL";
                    QString strLog = QString("微漏测试，时间点：%1 s, 微漏泄露值：%2 Pa(上限：%3) FAIL").arg(iTime).arg(QString::number(iLeakage, 'f', 2)).arg(mFailLimitLeakage);
                    emit logAddFailLog(strLog);
                    // table fail变红背景
                    emit tableUpdateTestStatus(TableViewShowFail);
                    mesAddInfor(ITEM_LEAKAGE, QString::number(mLeakageValue), "NG", m_errMsg);
                    m_errorCode = -2;
                    break;
                } else {
                    list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(iLeakage, 'f', 2) << "PASS";
                    QString strLog = QString("微漏测试，时间点：%1 s, 微漏泄露值：%2 Pa(上限：%3)").arg(iTime).arg(QString::number(iLeakage, 'f', 2)).arg(mFailLimitLeakage);
                    emit logAddNormalLog(strLog);
                }
            }
        } else if (outData.AirTestStep == 7) {
            stepStr = "释放气压";
            emit resultShowProcess(stepStr);
            break;
            // 执行完了
        }

        unsigned char status = 0x00;
        unsigned char step = 0x00;
        if (!mLeakageBase.airReadCycleStatus(step, status)) {
            m_errMsg = "Read Status Data Failed";
            emit logAddFailLog(m_errMsg);
            mLeakageBase.airReset();
            break;
        }

        // emit logAddNormalLog("判断测试是否结束");
        if (testCompleteJugde(step, status)) {
            break;
        }

    }

    emit logAddNormalLog("最终测试结果判断");
    ParamOutAir mLastData;

    if (timer.elapsed() > mTimeout * 1000) {
        m_errMsg = "Test Timeout";
        emit logAddFailLog(m_errMsg);
        mLeakageBase.airReset();
    } else {
        if (m_errorCode == 0) {
            if (!mLeakageBase.airReadLastResult(mLastData)) {
                m_errMsg = "最终结果读取失败";
                m_errorCode = -2;
                mLeakageBase.airReset();
                emit logAddFailLog("Test Fail: " + m_errMsg);
            } else {
                int alarmStatus = mLastData.AirAlarmStatus;
                QString mAlarmStr = getAlarmString(alarmStatus);
                int testStatus = mLastData.AirInstStatus;
                if (mLastData.AirAlarmStatus == 0x00 && (testStatus & 0x01) == 0x01) {
                    qDebug() << "TEST PASS";
                    emit logAddNormalLog("Test Pass");
                    emit tableUpdateTestStatus(TableViewShowPass);
                    mesAddInfor(ITEM_LEAKAGE, QString::number(mLeakageValue), "PASS");
                } else {
                    if (m_errMsg.isEmpty()) {
                        m_errMsg = mAlarmStr;
                    }
                    emit logAddFailLog("Test Fail");
                    emit logAddFailLog(m_errMsg);
                    qDebug() << "TEST FAIL";
                    qDebug() << "error message: " + m_errMsg;
                    emit tableUpdateTestStatus(TableViewShowFail);
                }
            }
        } else if (m_errorCode == -2){
            emit tableUpdateTestStatus(TableViewShowFail);
            emit logAddFailLog("Test Fail");
            emit logAddFailLog(m_errMsg);
            qDebug() << "TEST FAIL";
            qDebug() << "error message: " + m_errMsg;
        }
    }
}

void LeakageDE1006::step3()
{
    QString out;
    if (!mBoolMesIgnore && isOnlineMode()) {
        mesUpload();
        emit logAddNormalLog("进行出站操作");
        if (!mMesObj.outProduce((m_errorCode == 0) ? 0 : 1, out)) {
            m_errMsg = "出站失败: " + out;
            emit logAddFailLog("出站失败：" + out);
            showFail("MES出站失败");
        } else {
            emit logAddNormalLog("出站完成：" + out);
        }
    }

    // save log...
    CsvLogger csvlog;
    QDateTime dt = QDateTime::currentDateTime();

    if (mLogPath.isEmpty()) {
        //getValueString("Z1232", mLogPath);
    }

    QString logPath = mLogPath;
    logPath.append("/");
    QDir dir(logPath);
    if (!dir.exists())
        dir.mkpath(logPath);
    logPath.append(QString("%1.csv").arg(dt.toString("yyyyMMdd")));

    float cost = mTimer.elapsed() / 1000;
    emit logAddNormalLog("测试时间：" + QString::number(cost, 'f', 2) + "s");

    csvlog.setCsvLogPath(logPath);
    csvlog.addCsvTitle(QStringList() << "SN" << "开始时间" << "结束时间" << "测试总时间" << "测试结果" << "错误信息" << "大漏测试值" << "大漏上限" << "微漏测试值" << "微漏上限");
    QStringList listLog;
    listLog << m_sn << mStartTime.toString("yyyy/MM/dd hh:mm:ss") << dt.toString("yyyy/MM/dd hh:mm:ss") << QString::number(cost);
    listLog << ((m_errorCode == 0) ? "PASS" : "FAIL") << m_errMsg << QString::number(mPressureValue)
            << QString::number(mFailLimitPressure) << QString::number(mLeakageValue) << QString::number(mFailLimitLeakage);
    csvlog.addCsvLog(listLog);

    qInfo("step 3");
    if (m_isNewDevice) {
        // 此变量用来保存这一次的测试结果，下一次测试前要用到
        // 红箱管控的地方会依赖上一次的测试结果，就是这个变量来储存
        m_lastTestResult = (m_errorCode == 0);
        CycleExit();
    } else {
        mLeakageBase.airReset();
        mLeakageBase.airDeInit();
    }
    QString logName, csvName;
    logName.append(mLogPath);
    logName.append("/");
    logName.append(dt.toString("yyyyMMdd"));
    logName.append("/");
    QDir dir1(logName);
    if (!dir1.exists()) {
        dir1.mkpath(logName);
    }

    if (m_errorCode == 0) {
        logName.append("PASS_");
        emit resultShowPass();
    } else {
        logName.append("FAIL_");
        emit resultShowFail(m_errMsg);
    }
    logName.append(m_sn.replace("*", "_"));
    logName.append(dt.toString("hhmmss"));
    csvName.append(logName);

    logName.append(".txt");
    csvName.append(".csv");

    emit tableSaveCsv(csvName);
    emit logSaveTxt(logName);

//    msgBox("请更换产品，并按Enter进入下一轮测试", 4);

    stopProcess();
}
void LeakageDE1006::step4()
{

    qInfo("step 4");

}
void LeakageDE1006::step5()
{
    qInfo("step 5");
}

void LeakageDE1006::pumpStep()
{

}

void LeakageDE1006::start()
{
    qInfo("start ");
    startProcess();
}

void LeakageDE1006::init()
{

}

bool LeakageDE1006::CycleInit()
{
    // 开启串口资源
    bool rst = false;
    rst = m_leakagePlc.open(m_plcSerialName, 115200, true);
    rst &= m_leakageM5001A.open(mSerialName, 9600, true);
    if (!rst) {
        m_leakagePlc.close();
        m_leakageM5001A.close();
        return false;
    }

    m_sn.clear();
    m_errorCode = 0;
    m_errMsg.clear();
    return true;
}

bool LeakageDE1006::getSnAndExchangeDUT()
{
    bool ret{false};
    // 此处会对上一次的测试结果做判断，如果fail就需要拉出红箱
    if (false == m_lastTestResult) {
        showProcess("请拉出红箱，将测试FAIL产品放入红箱内");
        showMsgBox("请拉出红箱，将测试FAIL产品放入红箱内", 4);
        while (!m_leakagePlc.plcNGBoxPullOut()) {
            QCoreApplication::processEvents();
            QThread::msleep(50);
        }
        hideMsgBox();
    }

    addLog("复位PLC设备，并解锁转盘");

    // 防止前面步骤异常，这里将上盖，底盘，转盘全部复位
    m_leakagePlc.plcCappingRise();
    m_leakagePlc.plcChassisFall();
    FALSE_RETURN(m_leakagePlc.plcTurntableUnlock());


//    addLog("侦测转盘是否做了旋转操作");

    showMsgBox("侦测转盘是否做了旋转操作", 4);
    // 侦测转盘是否已经旋转了，只有旋转了才能再次上锁，否则会造成产品混乱
    while (!m_leakagePlc.plcTurnTableOnPos(false)) {
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    result_clean();
    hideMsgBox();

    addLog("转盘上锁操作，旋转转盘直到锁住");
    // 侦测转盘不在位了，里面上锁，进行下一次逻辑测试
    FALSE_RETURN(m_leakagePlc.plcTurntableLock());

    addLog("转盘旋转到位侦测，动点*2侦测");
    while (!m_leakagePlc.plcTurnTableLockedAndOnPos()) {
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }

    addLog("产品在位检测与合规检测");
    ret = m_leakagePlc.plcDutOnPos();
    if(!ret){
        m_errorCode = -2;
        m_errMsg = "产品在位检测失败";
        return false;
    }

    addLog("请扫码下一台测试产品SN");
//    m_sn = msgBox("请扫码下一台测试产品SN");
    if(!getAutoBarcode(m_sn)){
        m_errorCode = -2;
        m_errMsg = "获取SN失败";
        return false;
    }
    addLog("请旋转转盘，取出箱内已测试完成产品");

    return true;
}

bool LeakageDE1006::readyToTest()
{
//    addLog("产品在位检测与合规检测");
//    FALSE_RETURN(m_leakagePlc.plcDutOnPos());

    addLog("底盘锁定");
    FALSE_RETURN(m_leakagePlc.plcChassisRise());

    addLog("上盖下压");
    FALSE_RETURN(m_leakagePlc.plcCappingFall());

    return true;
}

bool LeakageDE1006::leakageTest()
{
    addLog("通知仪表执行测试");
    FALSE_RETURN(m_leakageM5001A.startTest());

    emit chartAddData(0, 0, 0, 0);
    emit chartAddData(0, 0, 1, 0);
    addLog("执行仪表状态监控");
    float largeLeakage = INVALID_VALUE;
    float littleLeakage = INVALID_VALUE;
    bool testOver = false, testException = false;
    LeakageResult lastResult;
    while(true) {
        LeakageResult result;
        float pressure = .0f, leakage = .0f;
        m_leakageM5001A.getImmidiateStatus(result);
        pressure = result.imPressure;
        leakage  = result.imLeakage;

        switch(result.imStatus) {
        case LS_VectorPumpIn:// 8
        {
            addLog("容积充气阶段, 气压值：" + QString::number(pressure) + ", 泄露值：" + QString::number(leakage));
            break;
        }
        case LS_Verify: // 1
        {
            addLog("测试前检验, 气压值：" + QString::number(pressure) + ", 泄露值：" + QString::number(leakage));
            break;
        }
        case LS_Pump: // 2
        {
            QString log;
            log.sprintf("充气阶段，充气时间：%.4f，气压值：%.4f", result.imPumpTimer, result.imPressure);
            addLog(log);
            break;
        }
        case LS_Keep: // 3 稳压阶段，冲压
        {
            QStringList list;
            if (largeLeakage == INVALID_VALUE) {
                largeLeakage = result.imPressure;
                list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(0, 'f', 2) << "TEST";
                tableAddRowData(list);
                tableUpdateTestStatus(2);
                chartAddData(0, 0, 0, 0);
            }

            result.imLargeLeakage = fabs((largeLeakage - result.imPressure) * 1000);//修改：yyg 2023-12-19 增加绝对值，避免后续逻辑混乱
            mPressureValue = result.imLargeLeakage;

            QString log;
            log.sprintf("保压阶段，保压时间：%.4f，气压值：%.4f，大漏值：%.4f", result.imStableTimer, result.imPressure, result.imLargeLeakage);
            addLog(log);

            emit chartAddData(result.imStableTimer, result.imLargeLeakage, 0, 0);
            emit tableUpdateData(QString::number(result.imLargeLeakage, 'f', 3), 3);

            if ((mFailLimitPressure < result.imLargeLeakage) && fabs(result.imLargeLeakage - mFailLimitPressure) > 1e-8) {
                m_errorCode = -5;
                m_errMsg = "大漏测试失败";

                list.clear();
                list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(mFailLimitPressure, 'f', 3) << "FAIL";
                QString strLog = QString("大漏测试，时间点：%1s, 大漏泄露值：%2 Pa(上限：%3)")
                        .arg(result.imStableTimer).arg(QString::number(result.imLargeLeakage, 'f', 3)).arg(mFailLimitPressure);
                emit logAddFailLog(strLog);

                mesAddInfor(ITEM_PRESSURE, QString::number(mPressureValue, 'f', 2), "NG", m_errMsg);
                // table fail变红背景
                emit tableUpdateTestStatus(1);

                m_leakageM5001A.stopTestManual();
            } else {
                list << "大漏测试" << "0" << QString::number(mFailLimitPressure) << QString::number(mFailLimitPressure, 'f', 3) << "PASS";
                QString strLog = QString("大漏测试，时间点：%1s, 大漏泄露值：%2 Pa(上限：%3)")
                        .arg(result.imStableTimer).arg(QString::number(result.imLargeLeakage, 'f', 3)).arg(mFailLimitPressure);
                emit logAddNormalLog(strLog);
            }

            break;
        }
        case LS_Test: // 4
        {
            QStringList list;
            if (littleLeakage == INVALID_VALUE) {
                littleLeakage = result.imLeakage;
                emit tableUpdateTestStatus(0);
                mesAddInfor(ITEM_PRESSURE, QString::number(mPressureValue, 'f', 2), "PASS");

                list.clear();
                list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(result.imLeakage, 'f', 3) << "FAIL";
                emit tableAddRowData(list);
                emit tableUpdateTestStatus(TableViewShowTest);
                emit chartAddData(0, 0, 1, 0);
            }

            mLeakageValue = result.imLeakage;

            QString log;
            log.sprintf("测试阶段，测试时间：%.4f，气压值：%.4f，微漏值：%.4f", result.imVerifyTimer, result.imPressure, result.imLeakage);
            addLog(log);

            emit chartAddData(result.imVerifyTimer, result.imLeakage, 1, 0);
            emit tableUpdateData(QString::number(result.imLeakage, 'f', 3), 3);
            if ((mFailLimitLeakage < result.imLeakage) && fabs(result.imLeakage - mFailLimitLeakage) > 1e-8) {
                m_errorCode = -6;
                m_errMsg = "微漏测试失败";
                list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(result.imLeakage, 'f', 3) << "FAIL";
                QString strLog = QString("微漏测试，时间点：%1 s, 微漏泄露值：%2 Pa(上限：%3) FAIL")
                        .arg(result.imVerifyTimer).arg(QString::number(result.imLeakage, 'f', 2)).arg(mFailLimitLeakage);
                emit logAddFailLog(strLog);
                // table fail变红背景
                emit tableUpdateTestStatus(TableViewShowFail);
                mesAddInfor(ITEM_LEAKAGE, QString::number(mLeakageValue), "NG", m_errMsg);
                break;
            } else {
                list << "微漏测试" << "0" << QString::number(mFailLimitLeakage) << QString::number(result.imLeakage, 'f', 2) << "PASS";
                QString strLog = QString("微漏测试，时间点：%1 s, 微漏泄露值：%2 Pa(上限：%3)")
                        .arg(result.imVerifyTimer).arg(QString::number(result.imLeakage, 'f', 3)).arg(mFailLimitLeakage);
                emit logAddNormalLog(strLog);
            }

            break;
        }
        case LS_PumpOut: // 5
        {
            QString log;
            log.sprintf("排气阶段，排气时间：%.4f，气压值：%.4f", result.imPumpOutTimer, result.imPressure);
            addLog(log);
            break;
        }
        case LS_Over: // 6
        {
            QString log;
            log.sprintf("测试结束阶段，测试结果：%02d，稳压压力：%.4f，最终泄露值：%.4f，错误代码：%02d",
                        result.lastResult, result.lastPressure, result.lastLeakage, result.lastErrorCode);
            addLog(log);
            break;
        }
        case LS_NoTest: // 7
        {
            addLog("当前无测试, 气压值：" + QString::number(pressure) + ", 泄露值：" + QString::number(leakage));
            break;
        }
        case LS_PrePumpIn:// 8
        {
            addLog("预充气阶段, 气压值：" + QString::number(pressure) + ", 泄露值：" + QString::number(leakage));
            break;
        }
        }


        testOver  = m_leakageM5001A.isTestOver(testException);
        if (testOver) {
            m_leakageM5001A.getImmidiateStatus(result);
            lastResult = result;
            break;
        }
    }

    if (!testException && lastResult.lastResult == 1) {
        emit logAddNormalLog("Test Pass");
        emit tableUpdateTestStatus(TableViewShowPass);
        mesAddInfor(ITEM_LEAKAGE, QString::number(mLeakageValue), "PASS");

        QString log;
        log.sprintf("测试非异常结束，测试结果：%02d，稳压压力：%.4f，最终泄露值：%.4f，错误代码：%02d",
                    lastResult.lastResult, lastResult.lastPressure, lastResult.lastLeakage, lastResult.lastErrorCode);
        addLog(log);

        return true;
    } else {
        emit logAddFailLog("Test Fail");

        QString log;
        log.sprintf("测试非异常结束，测试结果：%02d，稳压压力：%.4f，最终泄露值：%.4f，错误代码：%02d",
                    lastResult.lastResult, lastResult.lastPressure, lastResult.lastLeakage, lastResult.lastErrorCode);
        addLog(log, false);

        if (m_errorCode == 0) {
            m_errorCode = -7;
        }
        if (m_errMsg.isEmpty()) {
            m_errMsg = "测试异常结束或者最终测试结果部位0";
        }
    }

    return false;
}

void LeakageDE1006::CycleExit()
{
    m_leakagePlc.close();
    m_leakageM5001A.close();
}

//void LeakageDE1006::CycleInit()
//{
//    bool bRst = m_leakageM5001A.getHandle();
//    bRst &= m_leakagePlc.getHandle();
//    if (!bRst) {
//        qDebug() << ">>>获取串口资源失败";
//        return;
//    }
//    bRst = m_leakageM5001A.open("COM17", 9600, true);
//    bRst &= m_leakagePlc.open("COM18", 115200, true);
//    if (!bRst) {
//        qDebug() << ">>>开启串口失败";
//        return;
//    }

//    logAddNormalLog("PLC转盘解锁");
//    if (!m_leakagePlc.plcTurntableUnlock()){
//        logAddFailLog("PLC 转盘解锁失败");
//    } else {
//        logAddNormalLog("PLC转盘解锁OK");
//    }
//    msgBox("请扫码后放入产品放入槽位，并转出测试完成产品");
//    while(m_leakagePlc.plcTurnTableOnPos()) {
//        QCoreApplication::processEvents();
//        QThread::msleep(100);
//    }
//    if (!m_leakagePlc.plcTurntableLock()){
//        logAddFailLog("PLC 转盘上锁FAIL");
//    } else {
//        logAddWarningLog("PLC 转盘上锁OK");
//    }

//    logAddWarningLog("等待PLC转盘到位");
//    while (!m_leakagePlc.plcTurnTableLockedAndOnPos()) {
//        QCoreApplication::processEvents();
//        // qDebug() << "等待PLC到位";
//        QThread::msleep(100);
//    }

//    logAddWarningLog("有无产品检测");
//    qDebug() << "有无产品检测";
//    if (!m_leakagePlc.plcDutOnPos()) {
//        qDebug() << "无产品，失败";
//        logAddFailLog("无产品");
//        QThread::msleep(500);
//        return;
//    } else {
//        logAddWarningLog("有产品");
//    }

//    qDebug() << "地盘上升";
//    if (!m_leakagePlc.plcChassisRise()) {
//        qDebug() << "地盘上升失败";
//        logAddFailLog("地盘上升失败");
//        QThread::msleep(500);
//        return;
//    } else {
//        logAddWarningLog("地盘上升OK");
//    }

//    qDebug() << "顶部下降";
//    if (!m_leakagePlc.plcCappingFall()) {
//        qDebug() << "顶部下降失败";
//        logAddFailLog("顶部下降失败");
//        QThread::msleep(100);
//        return;
//    } else {
//        logAddWarningLog("顶部下降OK");
//    }

//    logAddWarningLog("开始七米测试");
//    m_leakageM5001A.startTest();
//    QElapsedTimer timer;
//    timer.start();
//    do {

//        LeakageStatus stat;
//        float pressure;
//        //for (int foo = 0; foo < 10; foo++) {
//            if (!m_leakageM5001A.getImmidiateStatus(stat, pressure)) {
//                qDebug() << "获取其压制失败";
//                logAddFailLog("获取气压值失败");
//            }
//            QString str;
//            str.sprintf("stat: %d, pressure: %.2f", stat, pressure);
//            logAddWarningLog(str);
//        //}

//    } while(timer.elapsed() < 50 * 1000);
//    QThread::msleep(2000);
//    logAddWarningLog("结束测试");
//    m_leakageM5001A.stopTestManual();


//    qDebug() << "顶部抬起";
//    logAddWarningLog("顶部抬起");
//    if (!m_leakagePlc.plcCappingRise()) {
//        qDebug() << "顶部抬起失败";
//        QThread::msleep(500);
//        return;
//    }

//    qDebug() << "地部下降";
//    logAddWarningLog("地部下降");
//    if (!m_leakagePlc.plcChassisFall()) {
//        qDebug() << "底部下降失败";
//        QThread::msleep(500);
//        return;
//    }

//    qDebug() << "转盘解锁";
//    logAddWarningLog("转盘解锁");
//    if (!m_leakagePlc.plcTurntableUnlock()) {
//        qDebug() << "转盘解锁失败";
//        QThread::msleep(500);
//        return;
//    }

//    m_leakageM5001A.close();
//    m_leakagePlc.close();

//    m_leakageM5001A.freeHandle();
//    m_leakagePlc.freeHandle();
//}

QString LeakageDE1006::getAlarmString(int alarmCode)
{
    QString strAlarm = "";
    switch (alarmCode) {
    case 0:
        strAlarm = "No Alarm";
        break;
    case 1:
        strAlarm = ("测试气压太高");
        break;
    case 2:
        strAlarm = ("测试气压太低");
        break;
    case 3:
        strAlarm = ("测试泄露严重");
        break;
    case 4:
        strAlarm = ("参考泄露严重");
        break;
    case 7:
        strAlarm = "Sensor out of order(overrun)";
        break;
    case 8:
        strAlarm = "ART error";
        break;
    case 9:
        strAlarm = "ART drift";
        break;
    case 10:
        strAlarm = "CAL error";
        break;
    case 11:
        strAlarm = "Volume too small(sealed component)";
        break;
    case 12:
        strAlarm = "Volume too large(sealed component)";
        break;
    case 43:
        strAlarm = ("气压太高");
        break;
    case 44:
        strAlarm = ("气压太低");
        break;
    case 45:
        strAlarm = "Piezo sendor out of order";
        break;
    case 46:
        strAlarm = ("泄气Error");
        break;
    case 47:
        strAlarm = "CAL drift error";
        break;
    case 48:
        strAlarm = "Calibration check error";
        break;
    case 49:
        strAlarm = ("校验阶段泄露太高");
        break;
    case 50:
        strAlarm = ("校验阶段泄露太低");
        break;
    case 51:
        strAlarm = "sealed component learning error";
        break;
    case 73:
        strAlarm = ("测试大漏");
        break;
    default:
        strAlarm = "Unknown Alarm";
        break;
    }
    return strAlarm;
}

void LeakageDE1006::slotsStoped()
{
    qInfo("stop...");
    slotStartWorkBtnClicked();
}

bool LeakageDE1006::testCompleteJugde(unsigned char step, unsigned char status)
{
    QString stepStr = "Unknow Step";
    if (step == 4) {
        stepStr = "充气阶段";
    } else if (step == 5) {
        stepStr = "大漏测试";
    } else if (step == 6) {
        stepStr = "微漏测试";
    } else if (step == 7) {
        stepStr = "释放气压";
    }

    int bit0 = status & 0x01;
    int bit1 = (status >> 1)  & 0x01;
    int bit2 = (status >> 2)  & 0x01;
    int bit3 = (status >> 3)  & 0x01;
    int bit4 = (status >> 4)  & 0x01;
    int bit5 = (status >> 5)  & 0x01;
    QString strLog;
    strLog.sprintf("step:%s, passbit:%d, failbit:%d, reffailbit:%d, alarm:%d, preserror:%d, cycleend:%d",
                   stepStr.toLocal8Bit().data(), bit0, bit1, bit2, bit3, bit4, bit5);
    qDebug() << strLog;
    emit (strLog);

    if (bit5 == 0x01) { // 确定cycle执行完了
        int irst = bit0 + bit1 + bit2 + bit3 + bit4;
        if (step == 0xFF &&  irst >= 0x01) {
            if (bit1 == 0x01) {
                m_errMsg = ("微漏测试失败");
                m_errorCode = -2;
            }
            if (bit2 ==0x01) {
                m_errMsg = "Reference Failed";
                m_errorCode = -2;
            }
            if (bit3 ==0x01) {
                m_errMsg = "Alarm Failed";
                m_errorCode = -2;
            }
            if (bit4 ==0x01) {
                m_errMsg = ("测试大漏");
                m_errorCode = -2;
            }
        }
        if (step != 0xFF) {
            m_errMsg = "Step is Error";
            m_errorCode = -2;
        }

        if (bit0 == 0x01) {
            emit tableUpdateTestStatus(0);
        }

        return true;
    }

    return false;
}

void LeakageDE1006::slotConnectBtnClicked()
{
    emit chartClearGraph();
    emit tableClearData();
    emit logClearData();
    emit resultShowDefault();

    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "新气密设备", m_isNewDevice);
    ConfigInfo::getInstance()->getValueString(m_StationName, "PLCCom", m_plcSerialName);
    ConfigInfo::getInstance()->getValueString(m_StationName, "LeakageCom", mSerialName);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "PressureFailLimit", mFailLimitPressure);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "LeakageFailLimit", mFailLimitLeakage);
    ConfigInfo::getInstance()->getValueString(m_StationName, "LeakageScanCom", m_portScanner);
    logNormal("isNewDevice: " + QString(m_isNewDevice ? "True" : "False"));
    logNormal("PLCCom: " + m_plcSerialName);
    logNormal("LeakageCom: " + mSerialName);
    logNormal("PressureFailLimit: " + QString::number(mFailLimitPressure));
    logNormal("LeakageFailLimit: " + QString::number(mFailLimitLeakage));

    emit chartAddGraph("大漏测试实时值", "时间(s)", "Pa(大漏测试泄露值)");
    emit chartAddGraph("微漏测试实时值", "时间(s)", "Pa(微漏测试泄露值)");

    emit chartAddLine("大漏测试泄露值(Pa)", 0, 0, 255, 0);
    emit chartAddLine(QString("大漏测试上限值(%1Pa)").arg(mFailLimitPressure), 255, 0, 0, 0);

    emit chartAddLine("微漏测试泄露值(Pa)", 0, 0, 255, 1);
    emit chartAddLine(QString("微漏测试上限值(%1Pa)").arg(mFailLimitLeakage), 255, 0, 0, 1);

    emit chartSetXRange(0, 5, 0);
    emit chartSetYRange(-10, mFailLimitPressure + 5, 0);
    for (int foo = 0; foo < 50; foo++) {
        emit chartAddData(foo, mFailLimitPressure, 0, 1);
    }

    emit chartSetXRange(0, 15, 1);
    emit chartSetYRange(-10, mFailLimitLeakage + 5, 1);
    for (int foo = 0; foo < 50; foo++) {
        emit chartAddData(foo, mFailLimitLeakage, 1, 1);
    }

    emit tableAddHeader(QStringList() << "测试项目" << "测试下限" << "测试上限" << "测试值" << "测试结果");

    emit logClearData();

    bool leakageIsOk = false;
    if (!m_isNewDevice) {
        do {
            if (!mLeakageBase.getResource()) {
                m_errMsg = QString("DLL加载失败，") + DATALINKDLLNAME;
                break;
            }

            qDebug() << "Init COM " << mSerialName;
            if (!mLeakageBase.airInit(mSerialName, 9600)) {
                m_errMsg = "串口初始化失败，" + mLeakageBase.errorString();
                break;
            }

            qDebug() << "Reset Air";
            if (!mLeakageBase.airReset()) {
                m_errMsg = QString("气密仪复位失败，检查串口线连接状态!") + mLeakageBase.errorString();
                mLeakageBase.airDeInit();
                break;
            }

            mLeakageBase.airDeInit();

            leakageIsOk = true;
        } while(0);
        if (!leakageIsOk) {
            emit resultShowFail(m_errMsg);
            return;
        }
    } else {
        // 开启串口资源
        bool rst = m_leakagePlc.getHandle();
        rst &= m_leakageM5001A.getHandle();
        if (!rst) {
            m_leakagePlc.freeHandle();
            m_leakageM5001A.freeHandle();
            showFail("加载串口资源失败");
            return;
        }

        m_leakagePlc.close();
        m_leakageM5001A.close();
        rst = m_leakagePlc.open(m_plcSerialName, 115200, true);
        rst &= m_leakageM5001A.open(mSerialName, 9600, true);
        if (!rst) {
            m_leakagePlc.close();
            m_leakageM5001A.close();
            m_leakagePlc.freeHandle();
            m_leakageM5001A.freeHandle();
            showFail("开启串口失败");
            return;
        }
        m_leakagePlc.close();
        m_leakageM5001A.close();

        m_lastTestResult = true;
    }

    BaseProduce::slotConnectBtnClicked();
}
void LeakageDE1006::result_clean(){
    emit chartClearData(0, 0);
    emit chartClearData(1, 0);
    emit tableClearData();
    emit logClearData();
    emit resultShowDefault();
}
void LeakageDE1006::slotStartWorkBtnClicked()
{
    if (!m_isNewDevice) {
        emit resultShowProcess("请扫描 SN到显示的框内");
        mScanDlg->showModal("请扫码 SN ");
        QStringList list = mScanDlg->dataList();
        if (list.at(0) == "NG") {
            showFail("取消扫码动作");
            updateStatus(1);
            return;
        } else {
            m_sn = list.at(1);
        }

        int iSnLength = 0;
        ConfigInfo::getInstance()->getValueInt32(m_StationName, "SN长度", iSnLength);
        qDebug() << "get the sn length limitation: " << iSnLength;
        if (iSnLength > 0) {
            if (m_sn.size() != iSnLength) {
                resultShowFail("SN 长度比对失败：需要长度为" + QString::number(iSnLength) + "的SN");
                //slotStartWorkBtnClicked();
                return;
            }

            QString prefix;
            ConfigInfo::getInstance()->getValueString(m_StationName, "SN前缀", prefix);
            qDebug() << "get the sn prefix limitaion: " << prefix;
            if (m_sn.indexOf(prefix.toUpper()) != 0) {
                resultShowFail("SN 前缀比对失败：需要SN前缀为：" + prefix);
                //slotStartWorkBtnClicked();
                return;
            }
        }
    }

    start();
}

void LeakageDE1006::slotDisConnectBtnClicked()
{

}

void LeakageDE1006::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
}

bool LeakageDE1006::getAutoBarcode(QString &sn)
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
            qDebug() << "size: " << nLen << " code: " << sn;
            return true;
        }

        QThread::msleep(100);
    }

    return false;
}

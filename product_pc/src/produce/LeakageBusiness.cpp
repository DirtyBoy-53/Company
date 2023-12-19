#include "LeakageBusiness.h"
#include "CLeakageRelative.h"
#include <QElapsedTimer>
#include <QBitArray>
#include <QDir>
#include <QDateTime>

LeakageBusiness::LeakageBusiness(QObject *parent) : QObject(parent)
{
    mTimeout = 60;//秒
}

QString LeakageBusiness::getAlarmString(int alarmCode)
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

// signals: sigLeakageInitRst(bool)
void LeakageBusiness::slotLeakageInit(QString name, int bandrate, QString logPath)
{
    mBoolInitRst = false;
    mErrString = "";
    mSerialName = name;
    mIntBandrate = bandrate;
    mLogPath = logPath;
    do {
        if (!mLeakageBase.getResource()) {
            qDebug() << "get dll failed";
            mErrString = "Get DataLink dll failed";
            break;
        }

        if (!mLeakageBase.airInit(mSerialName, mIntBandrate)) {
            mErrString = "Init Serial Port Failed";
            qDebug() << "inti com failed";
            mLeakageBase.freeResource();
            break;
        }

        if (!mLeakageBase.airReset()) {
            mErrString = "Init Serial Port Failed";
            qDebug() << "reset failed";
            mLeakageBase.airDeInit();
            break;
        }
        mBoolInitRst = true;
    }while(0);

    mLeakageBase.airDeInit();

    QDir dir(mLogPath);
    if (!dir.exists()) {
        if (!dir.mkpath(mLogPath)) {
            mBoolInitRst = false;
            mErrString = ("创建本底LOG目录失败");
        }
    }

    emit sigLeakageInitRst(mBoolInitRst, mErrString);
}

// signals: sigSetTestProcData 发送过程数据
// signals: sigLeakageTestRst 发送测试结果
void LeakageBusiness::slotLeakageTest(QString sn)
{
    bool bRst = false;
    STRUCT4UI lastResult;
    QElapsedTimer timerCost;
    timerCost.start();
    QString tmpLog = QCoreApplication::applicationDirPath();
    do {
        // log
        //
        tmpLog.append("/tmplog.txt");
        if (!mFlowLog.open(tmpLog, 1)) {
            mErrString = "Create Log Fail";
            break;
        }

        mFlowLog.record("Current is Leakage Test Work Station!!!");
        if (!mLeakageBase.airInit(mSerialName, mIntBandrate)) {
            mErrString = "Init Serial Port Failed";
            mFlowLog.record(mErrString);
            break;
        }

//        if (!mLeakageBase.airChangeProgram(0x01)) {
//            mErrString = "Change ProgId Failed";
//            qDebug() << "Change ProgId Failed";
//            break;
//        }
        // 加一个参数的比对测试，比对测试的参数是否和设定的一致
        mFlowLog.record("Device reset...");
        if (!mLeakageBase.airReset()) {
            mErrString = "Reset Failed";
            mFlowLog.record(mErrString);
            break;
        }

        // 加一个参数的比对测试，比对测试的参数是否和设定的一致
        mFlowLog.record("Device Start test...");
        if (!mLeakageBase.airStart()) {
            mErrString = "Start Failed";
            mFlowLog.record(mErrString);
            break;;
        }

        QThread::sleep(2);

        QElapsedTimer timer;
        timer.start();
        ParamOutAir outData;
        mFlowLog.record("Device read realtime data...");
        while(timer.elapsed() < mTimeout * 1000) {
            QCoreApplication::processEvents();
            if (!mLeakageBase.airReadRealResult(outData)) {
                mErrString = "Read Real Data Failed";
                mFlowLog.record(mErrString);
                mLeakageBase.airReset();
                break;
            }

            STRUCT4UI data;
            data.timestamp = (double)timer.elapsed() / 1000;
            data.step = outData.AirTestStep;
            data.pressure = (double)outData.AirPressure / 1000;
            data.leakage = (double)outData.AirLeakage / 1000;
            QVariant variant = QVariant::fromValue(data);
            emit sigSetTestProcData(variant);

            unsigned char status = 0x00;
            unsigned char step = 0x00;
            if (!mLeakageBase.airReadCycleStatus(step, status)) {
                mErrString = "Read Status Data Failed";
                mFlowLog.record(mErrString);
                mLeakageBase.airReset();
                break;
            }

            int bit0 = status & 0x01;
            int bit1 = (status >> 1)  & 0x01;
            int bit2 = (status >> 2)  & 0x01;
            int bit3 = (status >> 3)  & 0x01;
            int bit4 = (status >> 4)  & 0x01;
            int bit5 = (status >> 5)  & 0x01;
            QString strLog;
            QString stepStr = "Unknown step";
            switch (step) {
            case 4:
                stepStr = "Fill";
                break;
            case 5:
                stepStr = "Stablish";
                break;
            case 6:
                stepStr = "Test";
                break;
            case 7:
                stepStr = "Release";
                break;
            default:
                break;
            }
            strLog.sprintf("step: %s, passbit: %d, failbit: %d, reffailbit: %d, alarm: %d, preserror: %d, cycleend: %d",
                           stepStr.toLocal8Bit().data(),
                           bit0, bit1, bit2, bit3, bit4, bit5);
            mFlowLog.record(strLog);

            if (bit5 == 0x01) { // 确定cycle执行完了
                int irst = bit0 + bit1 + bit2 + bit3 + bit4;
                if (step == 0xFF &&  irst >= 0x01) {
                    if (bit1 == 0x01) {
                        mErrString = ("微漏测试失败");
                    }
                    if (bit2 ==0x01) {
                        mErrString = "Reference Failed";
                    }
                    if (bit3 ==0x01) {
                        mErrString = "Alarm Failed";
                    }
                    if (bit4 ==0x01) {
                        mErrString = ("测试大漏");
                    }
                }
                if (step != 0xFF) {
                    mErrString = "Step is Error";
                }
                mFlowLog.record(mErrString);
                break;
            }
        }

        if (timer.elapsed() > mTimeout * 1000) {
            mErrString = "Test Timeout";
            mFlowLog.record(mErrString);
            mLeakageBase.airReset();
            break;
        }

        if (!mLeakageBase.airReadLastResult(mLastData)) {
            mErrString = "Read Last Result Failed";
            mFlowLog.record(mErrString);
            mLeakageBase.airReset();
            break;
        }

    } while(0);

    mLeakageBase.airDeInit();


    int alarmStatus = mLastData.AirAlarmStatus;
    mAlarmStr = getAlarmString(alarmStatus);
    mFlowLog.record("Alarm string: " + mAlarmStr);

    int testStatus = mLastData.AirInstStatus;
    if (mLastData.AirAlarmStatus == 0x00 && (testStatus & 0x01) == 0x01) {
        lastResult.result = true;
        lastResult.alarmstr = "";
        lastResult.errstr = "";
        mFlowLog.record("Test Pass");
    } else {
        lastResult.result = false;
        lastResult.alarmstr = mAlarmStr;
        if (mErrString.isEmpty()) {
            mErrString = mAlarmStr;
        }
        lastResult.errstr = mErrString;
        mFlowLog.record("Test Fail, error message: " + mErrString);
    }

    mFlowLog.record("Test Cost: " + QString::number((double)timerCost.elapsed() / 1000, 'f', 3) + "s");

    QDateTime date = QDateTime::currentDateTime();
    if (mLogPath.isEmpty()) {
        mLogPath = QCoreApplication::applicationDirPath();
    }
    QString logPath = mLogPath;
    QString logFormatPath = mLogPath;
    logPath += "/logfile/" + date.toString("yyyyMMdd") + "/";
    logFormatPath += "/logfile/";

    QDir dir(logPath);
    if (!dir.exists()) {
        dir.mkpath(logPath);
    }

    QString logName = (lastResult.result ? "PASS_" : "FAIL_");
    logName += sn;
    logName += "_";
    logName.append(date.toString("hhmmsszzz") + ".txt");
    logPath += logName;

    mFlowLog.close();

    CTestFlowLog::copy(tmpLog, logPath);

    QString logFormatName = date.toString("yyyyMMdd") + ".csv";
    logFormatPath += logFormatName;\
    CTestFormatLog formatLog;
    formatLog.addItem(date.toString("yyyy-MM-dd hh:mm:ss:zzz"));
    formatLog.addItem(sn);
    formatLog.addItem(lastResult.result ? "PASS" : "FAIL");
    formatLog.addItem(mErrString);
    formatLog.addItem(QString::number(mLastData.AirPressure));
    formatLog.addItem(QString::number(mLastData.AirLeakage));
    formatLog.addItem(QString::number(mLeakageLimit));
    formatLog.saveAtOnce(logFormatPath);

    QVariant variant;
    variant.setValue(lastResult);
    emit sigLeakageTestRst(variant);
}

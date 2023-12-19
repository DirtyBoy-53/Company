#ifndef LEAKAGEBUSINESS_H
#define LEAKAGEBUSINESS_H

#include <QObject>
#include <CLeakageRelative.h>

typedef struct {
    int timestamp;
    int step;
    double pressure; // MPa
    double leakage; // pa
    bool result;
    QString errstr;
    QString alarmstr;
}STRUCT4UI;
Q_DECLARE_METATYPE(STRUCT4UI)

class LeakageBusiness : public QObject
{
    Q_OBJECT
public:
    explicit LeakageBusiness(QObject *parent = 0);
    QString getAlarmString(int alarmCode);

private:
    // void loadParam();

signals:
    // 发送初始化结果
    void sigLeakageInitRst(bool rst, QString errStr);
    // 将测试结果发送到界面
    void sigLeakageTestRst(QVariant variant);
    // 将测试过程发送到界面
    void sigSetTestProcData(QVariant data);


public slots:
    // 处理初始化
    void slotLeakageInit(QString name, int bandrate, QString logPath);

    // 处理开始测试过程
    void slotLeakageTest(QString sn);

private:
    CLeakageRelative mLeakageBase;
    QString mErrString;
    QString mAlarmStr;

    ParamOutAir mLastData;

    bool mBoolInitRst;
    QString mSerialName;
    int mIntBandrate;

    QString mLogPath;

    int mLeakageLimit;

    int mTimeout;

    // 金机校验模式
    bool mBoolGolden;
};

#endif // LEAKAGEBUSINESS_H

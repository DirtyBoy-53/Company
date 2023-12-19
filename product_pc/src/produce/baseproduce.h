#ifndef BASEPRODUCE_H
#define BASEPRODUCE_H

#include <QObject>

#include <statusprocess.h>
#include <mesclient.h>
#include <configinfo.h>
#include <waitsignal.h>
#include "CPropertyXml.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <CButtonSelector.h>

typedef enum {
    SnGetMothod_Manual = 0,
    SnGetMothod_Mes,
    SnGetMothod_Production,
    SnGetMothod_Auto
}SnGetMethod;

typedef enum {
    TestMode_Production = 0,
    TestMode_Golden,
    TestMode_Debug,
    TestMode_Offline
}TestMode;

typedef enum {
    TableViewShowPass = 0,
    TableViewShowFail = 1,
    TableViewShowTest = 2
} TableViewResult;

typedef enum {
    MSGBOX_INPUT = 0,
    MSGBOX_SELECT = 2,
    MSGBOX_TIPS = 3,
    MSGBOX_TIPS2 = 4
} MsgBoxTips;

//static const QMap<int, QString> g_errorCodeMap = {
//    {-1, "供电异常"},
//    {-2, "ARM版本错误"},
//    {-3, "整体功率异常"},
//    {-4, "魔毯输出异常"},
//    {-5, "IVI输出异常"},
//    {-6, "CAN通信异常"},
//    {-7, "RS232通信异常"},
//    {-8, "打印机模板加载失败"},
//    {-9, "打印标贴对比失败"},
//    {-10, "摄像头版本错误"},
//    {-11, "窗口加热功率异常"},
//    {-12, "快门检测异常"},
//    {-13, "图像检测异常"},
//    {-14, "功率检测异常"},
//    {-15, "出站失败"},
//    {-16, "进站失败"},
//    {-17, "宽电压检测异常"},
//    {-18, "获取SN失败"},
//    {-19, "工单编号已经使用完"},
//    {-20, "打螺丝失败"},
//    {-21, "批头选择错误"},
//    {-99, "初始化失败,请检测连接端口"}
//};

class BaseProduce : public StatusProcess
{
    Q_OBJECT

public:
    BaseProduce();
    ~BaseProduce();

public slots:
    virtual void slotConnectBtnClicked();
    virtual void slotStartWorkBtnClicked();
    virtual void slotStopWorkBtnClicked();
    virtual void slotBoxClosed(QString text);
    virtual void slotClickedFinish(bool result);
    //打印标签
    virtual void slotprintSN();
    virtual void slotLifeTimeOutLimit(bool lifeOver);


signals:
    // chart
    void chartClearGraph();
    void chartAddGraph(QString name, QString xName, QString yName);
    void chartAddLine(QString lineName, int r, int g, int b, int graphNo = 0);
    void chartAddData(float x, float y, int graphNo = 0, int lineNo = 0);
    void chartClearData(int graphNo = 0, int lineNo = 0);
    void chartSetXRange(float min, float max, int graphNo = 0);
    void chartSetYRange(float min, float max, int graphNo = 0);
    void chartSetLegendAlign(Qt::Alignment align);

    // log
    // table
    void tableAddHeader(QStringList header, int slot = 0);
    void tableAddRowData(QStringList list, int slot = 0);
    void tableUpdateData(QString data, int col, int slot = 0); // 只更新最后一行的某一列，不支持更新前面行的列
    void tableUpdateRowData(QStringList data, int row, int slot = 0);
    void tableUpdateTestStatus(int status, int slot = 0); // 0 pass, 1 fail, 2 test
    void tableClearData(int slot = 0);
    void tableSaveCsv(QString path, int slot = 0);

    // Debug log
    void logAddPassLog(QString log, int slot = 0);
    void logAddFailLog(QString log, int slot = 0);
    void logAddWarningLog(QString log, int slot = 0);
    void logAddNormalLog(QString log, int slot = 0);
    void logClearData(int slot = 0);
    void logSaveTxt(QString path, int slot = 0);

    // result
    void resultShowDefault(int slot = 0);
    void resultShowPass(int slot = 0);
    void resultShowFail(QString errString, int slot = 0);
    void resultShowProcess(QString step, int slot = 0);

    // video
    void videoUpdate(const int handle, QByteArray frameData, qint64 timeT);
    void waterPrintAddin(int id, QImage img, int x, int y, int slot = 0);
    void waterPrintVisible(int id, bool visible, int slot = 0);
    void startRecord(QString saveDir, int slot = 0);
    void stopRecord(int slot = 0);

    void videoSaveImage(QString path, int slot = 0); // 队列的方式，从队列去取
    void videoPlay(int slot = 0);
    void videoIsPlayNormal(int slot = 0);
    void videoClose(int slot = 0);
    void videoPaintCurve(int slot = 0);
    void videoPaintRectangle(int slot = 0);

    // status
    void statusShowStep(QString log);
    void statusShowStatics(QString status);
    void statusShowStatus(QString status);
    void updateStatus(int status);
    void showMsgBox(QString sMsg, int type);
    void hideMsgBox();

    // 耗材寿命
    void updateLifeTime(bool usePlus = true);


public:
    QString msgBox(QString sMsg, int type = 0);
    // chart
    void cClearGraph();
    void cAddGraph(QString name, QString xName, QString yName);
    void cAddLine(QString lineName, int r, int g, int b, int graphNo = 0);
    void cAddData(float x, float y, int graphNo = 0, int lineNo = 0);
    void cClearData(int graphNo = 0, int lineNo = 0);
    void cSetXRange(float min, float max, int graphNo = 0);
    void cSetYRange(float min, float max, int graphNo = 0);

    // debug log
    void logPass(QString log, int slot = 0);
    void logFail(QString log, int slot = 0);
    void logWarming(QString log, int slot = 0);
    void logNormal(QString log, int slot = 0);
    void logClear(int slot = 0);
    void logTxt(QString path, int slot = 0);

    // result
    void showDefault(int slot = 0);
    void showPass(int slot = 0);
    void showFail(QString errString, int slot = 0);
    void showProcess(QString step, int slot = 0);

    void tHeader(QStringList header, int slot = 0);
    void tRowData(QStringList data, int slot = 0);
    void tUpdateData(QString data, int col, int slot = 0);
    void tUpdateTestStatus(int status, int slot = 0);// 0 pass, 1 fail, 2 test
    void tClear(int slot = 0);
    void tCsv(QString list, int slot = 0);

	void showPackageTable(MesCheckItem * item, QString value);
    void checkItem(MesCheckItem * items, int idx = -1);

    bool isOnlineMode(); // 生产模式
    bool isOfflineMode();// 无MES模式
    bool isGoldenMode(); // 点检模式
    bool isDebugMode(); // 调试模式

    bool isPressureMode(); // 压测模式
    
    bool isScreenUnlock();

    bool isWorking();//是否工作模式

    TestMode testMode();
    QString testModeStr();

    QString projectName();

    SnGetMethod snGetMethod();
    int snGetLength();
    QString snGetPrefix();

    QString logPath();
    QString dutIP();

    void setCurrentBarcode(QString sn);// 设置当前过站条码
public:
    QString m_StationName;
protected:
    QString m_errMsg;
    bool m_lstRes = true;
    bool m_bResultCode = true;
    WaitSignal m_signal;
    QString m_boxMsg;
    MesCheckItem * m_checkItem = nullptr;
    QString m_sn;
    int m_errorCode = 0;
    bool m_bWorkingStatus = false;
    // StatusProcess interface
    bool m_skipMesUpload = false;
    bool m_manualStoped = false;

    QTimer m_timerScreenLocker;
    bool m_consumLifeIsOver = false;

    // 按钮选择器
    CButtonSelector *m_btnSelector;
    bool m_useBtnSelector;

public:
    virtual void outMes();
    virtual void enterMes();
    virtual void getSn();
    virtual void jugdeCustomLifeTime(bool ifStopProcess = true);//耗材寿命判断
    virtual bool customLifeIsOver();
};

#endif // BASEPRODUCE_H

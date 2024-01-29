#ifndef DISPENSINGCURING_H
#define DISPENSINGCURING_H

#include <QObject>

#include <QString>
#include <QTcpSocket>
#include <QTime>
#include <baseproduce.h>

#include <XyScanDialog.h>
#include <mescom.h>
#include "comguidecfg.h"
#include "CSerialDirect.h"

enum signal_type_e{
    door_is_close=0x11,
    dispensing_is_over=0x12,
    curing_is_over=0x13,
    door_is_open=0x14,
};
struct lens_result_s{
    QString sn{""};//SN
    QString scanRet{""};//镜头扫码结果
    QString disRet{""};//点胶结果
};

//快门点胶网络通信
class ShutterSocket{
public:
    ShutterSocket()=default;
    ~ShutterSocket();
    bool init(const QString ip="192.168.2.250",const int port=501);
    bool startDispensing();
    bool startCuring();
    bool getCurStatus(const signal_type_e& state);
    bool getIsOpen() const;
    bool close();
private:
    QTcpSocket socket;
    bool isOpen{false};
};

//镜头点胶通信
class LensDisSerial : public CSerialDirect
{
public:
    LensDisSerial()=default;
    bool isStart();
    bool sendReply();
    bool getResult(QList<lens_result_s>&);
};

//镜头固化通信
class LensCurSerial : public CSerialDirect
{
public:
    LensCurSerial()=default;
    bool isStart();
    bool sendReply();
    bool getResult(QString& time);
};

class DispensingCuring : public BaseProduce
{
    Q_OBJECT
public:
    DispensingCuring();
    ~DispensingCuring();

    void init_CL1003();
    void doWork_CL1003();
    void finish_CL1003();
    void updateTable_CL1003(QString name, qint64 workTime, bool result);
    void DisplayResult_CL1003();
    void DisplayPicture_CL1003();

    void init_CL1005();
    void doWork_CL1005();
    void finish_CL1005();
    void updateTable_CL1005(QString sn,QString zpm,//SN,载盘码
                            QString start_time,QString end_time,//固化开始时间，固化结束时间
                            QString consume_time,QString norm_time,//实际固化时间，标准固化时间
                            QString result);//点胶和固化结果
    void DisplayResult_CL1005();



    void enterMes(QString sn);
    void outMes(bool result=true);

    void addLog(QString log, bool rst = true)
    {
        if (rst) {
            logAddNormalLog(log);
            showProcess(log);
        } else {
            logAddFailLog(log);
            showFail(log);
        }
    }




private:
    MesCom mMesObj;

    bool mBoolMesIgnore{false};

    QString m_DisCurIP{""};
    QString m_imgPath{""};
    QList<GuideCfgInfo> m_tipsList;

    QTime m_time_CL003;


    QList<lens_result_s> m_DisResult_CL1005;
    LensDisSerial m_lensDisSerial;
    LensCurSerial m_lensCurSerial;
    QString m_DisCom{""};
    QString m_CurCom{""};
    QString m_StartTime{""};
    QString m_EndTime{""};
    QString m_ConsumeTime{""};
    QString m_NormTime{"15"};


public slots:
    void slotConnectBtnClicked();
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
};



#endif // DISPENSINGCURING_H

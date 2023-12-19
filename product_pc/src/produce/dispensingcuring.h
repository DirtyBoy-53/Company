#ifndef DISPENSINGCURING_H
#define DISPENSINGCURING_H

#include <QObject>

#include <QString>
#include <QTcpSocket>
#include <baseproduce.h>

#include <XyScanDialog.h>
#include <mescom.h>

enum signal_type_e{
    door_is_close=0x11,
    dispensing_is_over=0x12,
    curing_is_over=0x13,
    door_is_open=0x14,
};

class ShutterSocket{
public:
    ShutterSocket()=default;
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



class DispensingCuring : public BaseProduce
{
    Q_OBJECT
public:
    DispensingCuring();
    //1.设备初始化
    //2.获取SN
    //3.入站
    void init();

    //1.等待关门信号
    //2.收到关门信号，显示点胶中，并发送点胶信号
    //3.收到点胶完成信号，吸纳黑丝固化中，并发送固化信号
    //4.收到固化完成信号，显示“等待产品自动回到下料位置”
    //5.收到开门信号，显示“请取出产品确认点胶是否OK”
    //6.根据按钮选择器提示对应信息
    //  PASS：提示“请更换产品，进行下一轮测试”
    //    NG：提示“请把不良品放入不良盒，进行下一轮测试”
    //7.等待按钮选择器按下（绿色按钮）
    void doWork();

    //1.出站
    //2.准备下一轮作业
    void finish();

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
    ShutterSocket mShutterSocket;

    bool mBoolMesIgnore{false};

    QString m_DisCurIP{""};

public slots:
    void slotConnectBtnClicked();
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
};



#endif // DISPENSINGCURING_H

#ifndef ARMCB1002_H
#define ARMCB1002_H

#include "baseproduce.h"
#include "comguidecfg.h"
#include <httpmanager.h>
#include <atomic>

#define SNEWCOUNT 7

struct SnewInfo{
    QString code;
    QString sn;
    QString biaofu;
    QString shifu;
    QString fucha;
    QString niuli;
    QString quanshu;
    QString yali;
    QString result;
    QString sTm;
    void clear(){
        sn = "";
        biaofu = "";
        shifu = "";
        fucha = "";
        niuli = "";
        quanshu = "";
        yali = "";
        result = "FAILED";
    }
};

struct SnewItem{
    QString screw_no;
    QString screw_torque_force;
    QString screw_turns_nums;
    QString screw_returns_nums;
    QString screw_result;
};

class ArmCb1002 : public BaseProduce
{
    Q_OBJECT
public:
    ArmCb1002();

    void getSn();
    void getLenSn();

public slots:
    void slotsStoped();
    void waitData();
    void waitData2();
    void waitPaste();
    void mesUpload();
    void slotConnectBtnClicked();

private:
    void initTipsInfo();
    void initStateMachine();
    void tipsWork();
    void addLog(QString log, const int  result = 0);
    void updateTable(QString name, qint64 workTime, bool result);
    void stopWork();
    void checkNext();
    void checkVideo();

    void handleReq(std::string sUri,
                   std::string &sIn,
                   std::string &sOut);
    void showItem(SnewInfo item);

    void showImage2UI(QString name);

    QList<GuideCfgInfo> m_tipsList;
    QString m_lenSn;
    int m_type;
    int m_frameCount = 0;
    HttpManager * m_http = nullptr;
    std::atomic_int m_waitint = 0;
    SnewInfo m_snews[SNEWCOUNT];
    bool m_bworkStatus = false;
    int m_curSnewsIdx = 0;

    int m_stepNumber = 0;
    // BaseProduce interface
public slots:
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
};


#endif // ARMCB1002_H

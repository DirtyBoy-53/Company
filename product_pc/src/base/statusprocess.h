#ifndef STATUSPROCESS_H
#define STATUSPROCESS_H
#include <QState>
#include <QStateMachine>
#include <functional>
#include <QVector>
#include <mystate.h>
#include <mescom.h>
#include <QElapsedTimer>
#include <QFinalState>

struct StatusInfo{
    int order;
    QString sName;
    STATUSFUN funHandler;
    STATUSFUN failHandler;
    QState * pstate;
    bool isDefault;
};

class StatusProcess:public QObject
{
    Q_OBJECT
public:
    StatusProcess();
    void add(int order, QString sName,  STATUSFUN handler, const bool isDefault = 0);
    void process();
    void startProcess();
    void stopProcess();
    ~StatusProcess();
    void resetProcess();

    //需要上传对应步骤结果
    void setMesResult(QString sName, bool bPass, QString sValue="");

public slots:
    // void slotMachineDoStop();

signals:
    // void signalWorkFinish();
    void signalStepCycle(double cost);
    void signalDoStop();
public:
    QStateMachine *pMachine();
private:
    QStateMachine * m_pMachine = nullptr;
    QThread* m_machineThread = nullptr;
    QVector <StatusInfo>  m_list;
    int m_iHasSet = 0;

    QElapsedTimer m_cycleTimer;
};

#endif // STATUSPROCESS_H

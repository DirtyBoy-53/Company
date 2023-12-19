#include "statusprocess.h"

#include <QObject>
#include <QThread>


StatusProcess::StatusProcess()
{
    m_iHasSet = 0;
    m_pMachine = new QStateMachine();
    m_machineThread = new QThread();
    connect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);
    m_cycleTimer.start();
}
void StatusProcess::add(int order, QString sName, STATUSFUN handler, const bool isDefault)
{
    StatusInfo info;
    info.failHandler = nullptr;
//    info.funHandler = pParam;
    info.order = order;
    info.sName = sName;
    info.pstate = new MyState(handler);
    info.isDefault = isDefault;
    connect(info.pstate, &QState::entered,  (MyState *)info.pstate, &MyState::process);
    m_list.append(info);
}

void StatusProcess::process()
{
    signalStepCycle((float)m_cycleTimer.elapsed() / 1000.0f);
    m_cycleTimer.restart();

//    if (m_pMachine) {
//        disconnect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);
//        delete m_pMachine;
//        m_pMachine = nullptr;
//    }
//    m_pMachine = new QStateMachine();
//    connect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);

//    if (m_machineThread) {
//        m_machineThread->terminate();
//        delete m_machineThread;
//        m_machineThread = nullptr;
//    }
//    m_machineThread = new QThread();

    if(0 == m_iHasSet) {
        m_iHasSet = 1;

        qSort(m_list.begin(), m_list.end(),[](const StatusInfo &infoA,const StatusInfo &infoB){
            return infoA.order < infoB.order;
        });

        for(int i = 0;i < m_list.size(); i++) {
            if(i < m_list.size() - 1) {
//                if(m_list[i].isDefault == 1) {
//                    m_list[i].pstate->addTransition(this, &StatusProcess::signalWorkFinish, m_list[i+1].pstate);
//                } else {
                    m_list[i].pstate->addTransition(m_list[i+1].pstate);
//                }
            }
            m_pMachine->addState(m_list[i].pstate);
        }
        if(m_list.size() > 0) {
            m_pMachine->setInitialState(m_list[0].pstate);
        }
        m_pMachine->moveToThread(m_machineThread);
        m_machineThread->start();
    }
}

void StatusProcess::startProcess()
{
    m_pMachine->start();
}

void StatusProcess::stopProcess()
{
    signalDoStop();
}


StatusProcess::~StatusProcess()
{
    if (m_pMachine) {
        m_pMachine->stop();
        disconnect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);
        delete m_pMachine;
        m_pMachine = nullptr;
    }
    if (m_machineThread) {
        m_machineThread->terminate();
        delete m_machineThread;
        m_machineThread = nullptr;
    }
}

void StatusProcess::resetProcess()
{
    if(m_pMachine) {
        disconnect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);
        m_pMachine->stop();
        delete m_pMachine;
    }

    m_pMachine = new QStateMachine();
    connect(this, &StatusProcess::signalDoStop, m_pMachine, &QStateMachine::stop);
}

//void StatusProcess::slotMachineDoStop()
//{
//    m_pMachine->stop();
//}


QStateMachine *StatusProcess::pMachine()
{
    return m_pMachine;
}



#ifndef HTTPPASSSTATION_H
#define HTTPPASSSTATION_H

#include "baseproduce.h"
#include "httpmanager.h"


class HttpPassStation : public BaseProduce
{
    Q_OBJECT
public:
    HttpPassStation();
    static int setPcbCodeCallBack(QList<PcbCodeStruct>& pcba, void* param);
    int setPcbCode(QList<PcbCodeStruct>& pcba);
    static int getConnectCodeCallBack(void* param);
    int getConnectCode();
    void getSn();

public slots:
    void slotConnectBtnClicked();
    void slotStopWorkBtnClicked();
    void slotsStoped();

private:
    void initStateMachine();
    void addLog(QString log, const int  result = 0);
    void updateTable(QString pcb1, QString pcb2);

    HttpManager m_httpServer;
    QList<PcbCodeStruct> m_list;
    bool m_isRunning = false;
};

#endif // HTTPPASSSTATION_H

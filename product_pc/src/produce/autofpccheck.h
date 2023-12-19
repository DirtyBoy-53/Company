#ifndef AUTOFPCCHECK_H
#define AUTOFPCCHECK_H

#include "baseproduce.h"
#include "fpchandler.h"
#include "csvlogger.h"
#include "comguidecfg.h"

class AutoFpcCheck : public BaseProduce
{
    Q_OBJECT
public:
    AutoFpcCheck();

public slots:
    void slotConnectBtnClicked();
    void slotsStoped();

private:
    void updateTable(QString name, qint64 workTime, bool result);
    void initStateMachine();
    void addLog(QString log, const int result = 0);
    void checkNext();
    void init();
    void getSn();
    void getLenSn();
    void tipsWork();
    void checkProduct();
    void checkShutter();
    void autoProcess();
    void mesUpload();
    void stopWork();

private:
    FpcHandler* m_fpc = nullptr;
    CsvLogger m_csvLogger;
    QList<GuideCfgInfo> m_tipsList;
    QString m_imgPath;
    QString m_logPath;
    QStringList m_csvList;

    bool m_fpcUsing = true;
};

#endif // AUTOFPCCHECK_H

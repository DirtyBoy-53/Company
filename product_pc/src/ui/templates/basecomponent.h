#ifndef BASECOMPONENT_H
#define BASECOMPONENT_H

#include <QObject>
#include <QLabel>
#include <QTableView>
#include <QTextEdit>
#include <inputmessagebox.h>

#include "XyProcessLog.h"
#include "XyResultLabel.h"
#include "XyLineChart.h"
#include "videowidget.h"

#define MAXVIDEO 4

enum ETipsType{
    ETipsNormal = 0,
    ETipsERROR,
    ETipsOK
};

enum ELogType {
    ELogOK = 0,
    ELogError,
    ELogNormal,
};

class BaseComponent : public QWidget
{
    Q_OBJECT
public:
    explicit BaseComponent(QWidget *parent = nullptr);
    void setVideoWidget(VideoWidget* widget, int slot = 0);
    void setResultWidget(XyResultLabel* widget, int slot = 0);
    void setProcessLogWidget(XyProcessLog* widget, int slot = 0);
    void setChartWidget(XyLineChart* widget);

public slots:
    //多个窗口时handle 0 -x
    virtual  void slotVideoUpdate(const int handle, QByteArray frameData, qint64 timeT);

    //showmsg getcontent
    virtual void showMsgBox(QString sMsg, int type = 0);
    virtual void hideMsgBox();
    virtual void slotChartClearGraph();
    virtual void slotChartClearData(int chart = 0, int line = 0);
    virtual void slotChartAddGraph(QString name, QString xName, QString yName);
    virtual void slotChartAddLine(QString lineName, int r, int g, int b, int chart);
    virtual void slotChartXRange(float min, float max, int chart = 0);
    virtual void slotChartYRange(float min, float max, int chart = 0);
    virtual void slotChartAddData(float x, float y, int chart = 0, int line = 0);
    virtual void slotChartLegendAlign(Qt::Alignment align);

    virtual void slotTableAddHeader(QStringList header, int slot = 0);
    virtual void slotTableAddRow(QStringList list, int slot = 0);
    virtual void slotTableUpdateItem(QString data, int col, int slot = 0);
    virtual void slotTableUpdateStatus(int status, int slot = 0); // 0 pass, 1 fail, 2 test
    virtual void slotTableClear(int slot = 0);
    virtual void slotTableSaveCsv(QString log, int slot = 0);
    virtual void slotTableUpdateRow(QStringList list, const int col, int slot = 0);

    virtual void slotLogAddPass(QString log, int slot = 0);
    virtual void slotLogAddFail(QString log, int slot = 0);
    virtual void slotLogAddNormal(QString log, int slot = 0);
    virtual void slotLogAddWarning(QString log, int slot = 0);
    virtual void slotLogClear(int slot = 0);
    virtual void slotLogSaveTxt(QString log, int slot = 0);

    virtual void slotResultShowDefault(int slot = 0);
    virtual void slotResultShowPass(int slot = 0);
    virtual void slotResultShowFail(QString err, int slot = 0);
    virtual void slotShowProcess(QString step, int slot = 0);

    virtual void slotWaterPrintAddin(int id, QImage img, int x, int y, int slot = 0);
    virtual void slotWaterPrintVisible(int id, bool visible, int slot = 0);
    virtual void slotStartRecord(QString dir, int slot = 0);
    virtual void slotStopRecord(int slot = 0);
signals:
    virtual void closeBox(QString sText);
    virtual void signalClickedFinish(bool result);
    virtual void sigPrint();

protected:

    XyResultLabel * m_labeTips = nullptr;

    // for multiple test
    XyResultLabel* m_labelTipsArray[MAXVIDEO] = {nullptr};
    VideoWidget * m_videos[MAXVIDEO] = {nullptr};
    XyProcessLog* m_tabArray[MAXVIDEO] = {nullptr};
    XyLineChart* m_chart = nullptr;

    InputMessageBox * m_msgBox = nullptr;
};

#endif // BASECOMPONENT_H

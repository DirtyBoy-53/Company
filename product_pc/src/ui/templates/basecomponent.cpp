#include "basecomponent.h"
#include <videoformatparser.h>
#include <QColor>

BaseComponent::BaseComponent(QWidget *parent)
    : QWidget{parent}
{
    m_msgBox = new InputMessageBox(this);
    connect(m_msgBox, &InputMessageBox::signalConfirmClicked,
            this, &BaseComponent::closeBox);
    connect(m_msgBox, &InputMessageBox::signalClickedFinish,
            this, &BaseComponent::signalClickedFinish);

    m_msgBox->hide();
}

void BaseComponent::setVideoWidget(VideoWidget *widget, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);
    m_videos[slot] = widget;
}

void BaseComponent::setResultWidget(XyResultLabel *widget, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);
    m_labelTipsArray[slot] = widget;
}

void BaseComponent::setProcessLogWidget(XyProcessLog *widget, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);
    m_tabArray[slot] = widget;
}

void BaseComponent::setChartWidget(XyLineChart *widget)
{
    m_chart = widget;
}

void BaseComponent::slotVideoUpdate(const int handle, QByteArray frameData, qint64 timeT)
{
    if(handle < 0 || handle >= MAXVIDEO) {
        qInfo("not init video ");
        return;
    }
    if(m_videos[handle]) {
        VideoFormatInfo info;
        VideoInfo::getInstance()->getVideoInfo(handle, info);
        m_videos[handle]->setVideoFormat(info);
        m_videos[handle]->updateImage(frameData, timeT);
    }else {
        qInfo("not init video ");
    }
}

void BaseComponent::showMsgBox(QString sMsg, int type)
{
    m_msgBox->setMsg(sMsg, type);
//    m_msgBox->show();
}

void BaseComponent::hideMsgBox()
{
    m_msgBox->hide();
}

void BaseComponent::slotChartClearGraph()
{
    if (!m_chart) return;

    m_chart->xyClearChart();
}

void BaseComponent::slotChartClearData(int chart, int line)
{
    if (!m_chart) return;

    m_chart->xyClearData(chart, line);
}

void BaseComponent::slotChartAddGraph(QString name, QString xName, QString yName)
{
    if (!m_chart) return;

    m_chart->xyAddChart(name, xName, yName);
}

void BaseComponent::slotChartAddLine(QString lineName, int r, int g, int b, int chart)
{
    if (!m_chart) return;

    m_chart->xyAddLine(lineName, QColor(r,g,b), chart);
}

void BaseComponent::slotChartXRange(float min, float max, int chart)
{
    if (!m_chart) return;

    m_chart->xySetXRange(min, max, chart);
}

void BaseComponent::slotChartYRange(float min, float max, int chart)
{
    if (!m_chart) return;

    m_chart->xySetYRange(min, max, chart);
}

void BaseComponent::slotChartAddData(float x, float y, int chart, int line)
{
    if (!m_chart) return;

    m_chart->xyAddData(x, y, chart, line);
}

void BaseComponent::slotChartLegendAlign(Qt::Alignment align)
{
    if (!m_chart) return;

    m_chart->xySetLegendAlign(align);
}

void BaseComponent::slotTableAddHeader(QStringList header, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;

    m_tabArray[slot]->xyAddTableHeader(header);
}

void BaseComponent::slotTableAddRow(QStringList list, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;

    m_tabArray[slot]->xyAddRowData(list);
}

void BaseComponent::slotTableUpdateItem(QString data, int col, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;

    m_tabArray[slot]->xyUpdateItemData(data, col);
}

void BaseComponent::slotTableUpdateStatus(int status, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyUpdateTestStatus((ItemStatus)status);
}

void BaseComponent::slotTableClear(int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyClearTable();
}

void BaseComponent::slotTableSaveCsv(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xySaveCsv(log);
}

void BaseComponent::slotTableUpdateRow(QStringList list, const int row, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyUpdateRowData(list, row);
}

void BaseComponent::slotLogAddPass(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyAddPassLog(log);
}

void BaseComponent::slotLogAddFail(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyAddFailLog(log);
}

void BaseComponent::slotLogAddNormal(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyAddNormalLog(log);
}

void BaseComponent::slotLogAddWarning(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyAddWarningLog(log);
}

void BaseComponent::slotLogClear(int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xyClearLog();
}

void BaseComponent::slotLogSaveTxt(QString log, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_tabArray[slot]) return;
    m_tabArray[slot]->xySaveTxt(log);
}

void BaseComponent::slotResultShowDefault(int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_labelTipsArray[slot]) return;
    m_labelTipsArray[slot]->xySetNormalStatus("");
}

void BaseComponent::slotResultShowPass(int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_labelTipsArray[slot]) return;
    m_labelTipsArray[slot]->xySetPassResult();
}

void BaseComponent::slotResultShowFail(QString err, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_labelTipsArray[slot]) return;
    m_labelTipsArray[slot]->xySetFailResult(err);
}

void BaseComponent::slotShowProcess(QString step, int slot)
{
    Q_ASSERT(slot >=0 && slot < MAXVIDEO);

    if (!m_labelTipsArray[slot]) return;
    m_labelTipsArray[slot]->xySetNormalStatus(step);
}

void BaseComponent::slotWaterPrintAddin(int id, QImage img, int x, int y, int slot)
{
    WatermarkInfo markInfor;
    markInfor.img = img;
    markInfor.visible = true;
    markInfor.x = x;
    markInfor.y = y;
    m_videos[slot]->addWatermark(id, markInfor);
}

void BaseComponent::slotWaterPrintVisible(int id, bool visible, int slot)
{
    m_videos[slot]->setWatermarkVisible(id, visible);
}

void BaseComponent::slotStartRecord(QString dir, int slot)
{
    m_videos[slot]->startRecord(dir);
}

void BaseComponent::slotStopRecord(int slot)
{
    m_videos[slot]->stopRecord();
}

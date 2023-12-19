#ifndef XYLINECHART_H
#define XYLINECHART_H

#include <QWidget>
#include <QGridLayout>
#include <QHash>
#include "qcustomplot.h"


class XyLineChart : public QWidget
{
    Q_OBJECT
#define PLOT_SIZE 256
public:
    explicit XyLineChart(QWidget *parent = nullptr);
    ~XyLineChart();

    void xyClearChart();
    int xyAddChart(QString name, QString xName, QString yName);
    int xyAddLine(QString lineName, QColor lineColor, int chartSequence);
    void xyAddData(float x, float y, int chartSequence = 0, int lineSequence = 0);
    void xySetXRange(float min, float max, int chartSequence = 0);
    void xySetYRange(float min, float max, int chartSequence = 0);
    void xyClearData(int chartSequence = 0, int lineSequence = 0);
    void xySetLegendAlign(Qt::Alignment align);
private:

    QGridLayout* mGridLayout;
    QHash<int, QCustomPlot*> mHashChartPtr;
    QHash<QPair<int, int>, QCPGraph*> mHashLinePtr;

    // 记录图标的线条数量，最大支持256副图线条
    int mLineCountInPlot[PLOT_SIZE];
    Qt::Alignment mLegendAlign;
};

#endif // XYLINECHART_H

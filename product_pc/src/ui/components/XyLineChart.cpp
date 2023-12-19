#include "XyLineChart.h"

XyLineChart::XyLineChart(QWidget *parent) :
    QWidget(parent)
{
    mGridLayout = new QGridLayout;
    this->setLayout(mGridLayout);
    this->setMinimumSize(500, 500);

    mHashChartPtr.clear();
    mHashLinePtr.clear();
    memset(mLineCountInPlot, 0, PLOT_SIZE);
}

XyLineChart::~XyLineChart()
{
}

void XyLineChart::xyClearChart()
{
    for (int foo = 0; foo < mHashChartPtr.size(); foo++) {
        mGridLayout->removeWidget(mHashChartPtr[foo]);
    }
    mHashChartPtr.clear();
    mHashLinePtr.clear();
    memset(mLineCountInPlot, 0, PLOT_SIZE);
    mLegendAlign = Qt::AlignTop | Qt::AlignLeft;
}

int XyLineChart::xyAddChart(QString name, QString xName, QString yName)
{
    QCustomPlot* plot = new QCustomPlot(this);

    QCPTextElement *title = new QCPTextElement(plot);
    title->setText(name);
    title->setFont(QFont("sans", 12, QFont::Bold));
    // then we add it to the main plot layout:
    plot->plotLayout()->insertRow(0); // insert an empty row above the axis rect
    plot->plotLayout()->addElement(0, 0, title); // place the title in the empty cell we've just created

    plot->xAxis->setLabel(xName);
    plot->yAxis->setLabel(yName);
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, mLegendAlign);

    mGridLayout->addWidget(plot);
    int chartSequence = mHashChartPtr.size();
    mHashChartPtr.insert(chartSequence, plot);
    return chartSequence;
}

int XyLineChart::xyAddLine(QString lineName, QColor lineColor, int chartSequence)
{
    QCustomPlot* plot = mHashChartPtr.value(chartSequence);

    QCPGraph* graph = plot->addGraph();
    graph->setName(lineName);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setColor(lineColor);
    graph->setPen(pen);

    int lineSequence = mLineCountInPlot[chartSequence];
    mHashLinePtr.insert(QPair<int, int>(chartSequence, lineSequence), graph);
    mLineCountInPlot[chartSequence]++;

    return lineSequence;
}

void XyLineChart::xyAddData(float x, float y, int chartSequence, int lineSequence)
{
//    QCPGraph* graph = mHashLinePtr.value(QPair<int,int>(chartSequence, lineSequence));
//    graph->addData(x, y);

    QCustomPlot *plot = mHashChartPtr.value(chartSequence);
    plot->graph(lineSequence)->addData(x, y);
    plot->replot();
}

void XyLineChart::xySetXRange(float min, float max, int chartSequence)
{
    QCustomPlot* plot = mHashChartPtr.value(chartSequence);
    plot->xAxis->setRange(min, max);
}

void XyLineChart::xySetYRange(float min, float max, int chartSequence)
{
    QCustomPlot* plot = mHashChartPtr.value(chartSequence);
    plot->yAxis->setRange(min, max);
}

void XyLineChart::xyClearData(int chartSequence, int lineSequence)
{
    QCustomPlot *plot = mHashChartPtr.value(chartSequence);
    plot->graph(lineSequence)->data().data()->clear();
    plot->replot();
}

void XyLineChart::xySetLegendAlign(Qt::Alignment align)
{
    mLegendAlign = align;
}

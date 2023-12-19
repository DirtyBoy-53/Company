#include "widgetmodel10.h"
#include "ui_widgetmodel10.h"
#include "StaticSignals.h"

WidgetModel10::WidgetModel10(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel10)
{
    ui->setupUi(this);
    setChartWidget(ui->chart);
    setVideoWidget(ui->widgetVideo);
    setProcessLogWidget(ui->log);
    setResultWidget(ui->label);
}

WidgetModel10::~WidgetModel10()
{
    delete ui;
}



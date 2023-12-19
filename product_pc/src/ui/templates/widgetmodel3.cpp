#include "widgetmodel3.h"
#include "ui_widgetmodel3.h"
#include <QDebug>

WidgetModel3::WidgetModel3(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel3)
{
    ui->setupUi(this);
    setVideoWidget(ui->widgetVideo1, 0);
    setVideoWidget(ui->widgetVideo2, 1);
    setResultWidget(ui->labelTips);
    setProcessLogWidget(ui->tableWidget);

    ui->widgetVideo1->setScaleZoom(0.5);
    ui->widgetVideo2->setScaleZoom(0.5);
}

WidgetModel3::~WidgetModel3()
{
    delete ui;
}

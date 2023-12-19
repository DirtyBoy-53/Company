#include "widgetmodel1.h"
#include "ui_widgetmodel1.h"
#include <QDebug>

WidgetModel1::WidgetModel1(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel1)
{
    ui->setupUi(this);

    setVideoWidget(ui->widgetVideo);
    setResultWidget(ui->labelTips);
    setProcessLogWidget(ui->tableWidget);
}

WidgetModel1::~WidgetModel1()
{
    delete ui;
}

#include "widgetmodel7.h"
#include "ui_widgetmodel7.h"
#include "StaticSignals.h"

WidgetModel7::WidgetModel7(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel7)
{
    ui->setupUi(this);
    setVideoWidget(ui->widget);
    setResultWidget(ui->labelTips);
    setProcessLogWidget(ui->widgetLog);
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdateImg, this, &WidgetModel7::updateImg);
    connect(StaticSignals::getInstance(), &StaticSignals::statusChangeVideoPage, this, &WidgetModel7::changePage);
}

WidgetModel7::~WidgetModel7()
{
    delete ui;
}

void WidgetModel7::changePage(const int type)
{
    ui->stackedWidget->setCurrentIndex(type);
}

void WidgetModel7::updateImg(QString path)
{
    if(m_imgPath != path) {
        m_imgPath = path;
        ui->labelImg->setPixmap(QPixmap(path).scaled(ui->labelImg->size()));
    }
}

#include "widgetmodel6.h"
#include "ui_widgetmodel6.h"

WidgetModel6::WidgetModel6(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel6)
{
    ui->setupUi(this);

    setResultWidget(ui->slot1Result, 0);
    setResultWidget(ui->slot2Result, 1);
    setResultWidget(ui->slot3Result, 2);
    setResultWidget(ui->slot4Result, 3);
    ui->slot1Result->setFontSize(15);
    ui->slot2Result->setFontSize(15);
    ui->slot3Result->setFontSize(15);
    ui->slot4Result->setFontSize(15);

    setProcessLogWidget(ui->slot1Process, 0);
    setProcessLogWidget(ui->slot2Process, 1);
    setProcessLogWidget(ui->slot3Process, 2);
    setProcessLogWidget(ui->slot4Process, 3);
    ui->slot1Process->xySetFontSize(12);
    ui->slot2Process->xySetFontSize(12);
    ui->slot3Process->xySetFontSize(12);
    ui->slot4Process->xySetFontSize(12);

    setVideoWidget(ui->slot1Video, 0);
    setVideoWidget(ui->slot2Video, 1);
    setVideoWidget(ui->slot3Video, 2);
    setVideoWidget(ui->slot4Video, 3);
}

WidgetModel6::~WidgetModel6()
{
    delete ui;
}

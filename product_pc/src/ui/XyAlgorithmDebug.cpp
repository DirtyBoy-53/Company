#include "XyAlgorithmDebug.h"
#include "ui_XyAlgorithmDebug.h"
#include "StaticSignals.h"
#include <QMessageBox>

XyAlgorithmDebug::XyAlgorithmDebug(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XyAlgorithmDebug)
{
    ui->setupUi(this);
}

XyAlgorithmDebug::~XyAlgorithmDebug()
{
    delete ui;
}

void XyAlgorithmDebug::on_btnStartRecordFT_clicked()
{
    int timeout = ui->editFTtimeout->text().toInt();
}


void XyAlgorithmDebug::on_btnStopRecordFT_clicked()
{
}


void XyAlgorithmDebug::on_btnStartCollectY16_clicked()
{
    int timeout = ui->editY16timeout->text().toInt();
    StaticSignals::getInstance()->kStartCollectY16(timeout * 60);
}


void XyAlgorithmDebug::on_btnStopCollectY16_clicked()
{
    StaticSignals::getInstance()->kStopCollectY16();
}


void XyAlgorithmDebug::on_btnCollectMtf_clicked()
{
    QMessageBox::information(this, "确认产品已放入工装设备", "确认产品已放入工装设备");
    StaticSignals::getInstance()->mtfGetOnceData(ui->lineEdit->text());
    ui->lineEdit->setText("");
}


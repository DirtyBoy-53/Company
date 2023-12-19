#include "widgetmodel8.h"
#include "ui_widgetmodel8.h"
#include "StaticSignals.h"
#include "configinfoelec.h"

WidgetModel8::WidgetModel8(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel8)
{
    ui->setupUi(this);
    setResultWidget(ui->labelTips);
    setProcessLogWidget(ui->widgetLog);

    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdatePos, this, &WidgetModel8::updatePos);
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdateEnable, this, &WidgetModel8::updateEnable);
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdateOffset, this, &WidgetModel8::updateOffset);
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdateImg, this, &WidgetModel8::updateImg);
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdateWorkResult, this, &WidgetModel8::updateWorkResult);

    ui->frame->setStyleSheet("background-color:rgba(230,230,230,1)");
    ui->frame_2->setStyleSheet("background-color:rgba(230,230,230,1)");
    ui->frame_3->setStyleSheet("background-color:rgba(230,230,230,1)");
    ui->frame_4->setStyleSheet("background-color:rgba(230,230,230,1)");
    ui->frame_5->setStyleSheet("background-color:rgba(230,230,230,1)");

}

WidgetModel8::~WidgetModel8()
{
    delete ui;
}

void WidgetModel8::updatePos(const int x, const int y)
{
    ui->labelPos->setText(QString("%1, %2").arg(QString::number(x)).arg(QString::number(y)));
}

void WidgetModel8::updateOffset(const int offset)
{
    ui->labelOffset->setText(QString::number(offset));
}

void WidgetModel8::updateEnable(const bool enable)
{
    if(enable) {
        ui->labelEnable->setStyleSheet("background-color:green");
    } else {
        ui->labelEnable->setStyleSheet("background-color:red");
    }
}

void WidgetModel8::updateImg(QString path)
{
    QString imgPath;
    ConfigInfoElecLock::getInstance()->GetImagePath(imgPath);
    QString img = imgPath + "/" + path;
    if(m_imgPath != img) {
        m_imgPath = img;
        ui->labelImg->setPixmap(QPixmap(img).scaled(ui->labelImg->size()));
    }
}

void WidgetModel8::updateWorkResult(QString mnm, QString reticle)
{
    ui->labelMnm->setText(mnm + " N.m");
    ui->labelReticle->setText(reticle + "圈");
}

#include "lidarconnectwidget.h"
#include "configinfo.h"
LidarConnectWidget::LidarConnectWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ConfigInfo::getInstance()->Init();
    ui.lineEdit_ip->setText(ConfigInfo::getInstance()->m_otherInfo.m_lidarIP);
    ui.lineEdit_port->setText("2368");
    connect(ui.btn_connect,&QPushButton::clicked,this,&LidarConnectWidget::slot_btn_clicked_lidar_init);
}

LidarConnectWidget::~LidarConnectWidget()
{

}

void LidarConnectWidget::slot_btn_clicked_lidar_init()
{
    if(ui.btn_connect->text() == "connect"){
        ui.btn_connect->setText("disconnect");
        emit sig_lidar_init(ui.lineEdit_ip->text());
    }else{
        ui.btn_connect->setText("connect");
        emit sig_lidar_disconnect();
    }
}

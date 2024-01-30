#pragma once

#include <QWidget>
#include "ui_lidarconnectwidget.h"

class LidarConnectWidget : public QWidget
{
	Q_OBJECT

public:
	LidarConnectWidget(QWidget *parent = nullptr);
	~LidarConnectWidget();
public slots:
	void slot_btn_clicked_lidar_init();
signals:
	void sig_lidar_init(QString);
	void sig_lidar_disconnect();

private:
	Ui::LidarConnectWidgetClass ui;
};

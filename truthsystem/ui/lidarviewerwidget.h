#pragma once

#include <QWidget>
#include "ui_lidarviewerwidget.h"
#include <qvector4d.h>
#include "lidar_AT128.h"
#include "utils/GQueue.h"
#include "lidarconnectwidget.h"
class LidarViewerWidget : public QWidget
{
	Q_OBJECT

public:
	LidarViewerWidget(QWidget *parent = nullptr);
	~LidarViewerWidget();
	void readPcdFile(const std::string& file_path, std::vector<QVector4D>& cloud);
	void LoadPcdFolder();

signals:
	void sig_set_camera_view(ENUM_VIEW view);
	void sig_set_projection_mode(Camera::Type model);
	void sig_set_isShow_grid(bool);
	void sig_set_isShow_polar(bool);
	void sig_set_isShow_coordinate(bool);
	void sig_update_lidar_data(PointView);
	void sig_update_lidar_timestamp(QString);

public slots:
	void slot_btn_clicked_tools();
	void slot_btn_clicked_playCtrl();
	void slot_lidar_init(QString ip);
	void slot_lidar_disconnect();
	void slot_recv_lidar_frame(PointView data);
	void slot_set_lidar_path(QString &path);
private:
	Ui::LidarViewerWidgetClass ui;

	QStringList m_pcdFileList;
	bool m_isShow_local_lidar_data{ false };
	bool m_isShow_net_lidar_data{false};
	bool m_isRun{ false };
	bool m_isRunSave{false};
	bool m_isStartRecv{false};
	QString m_savePath{""};
	LidarConnectWidget *m_lidarConWidget{nullptr};
	std::shared_ptr<pcl::PointCloud<PointXYZIT>> m_PointsPtr;	
	Lidar_AT128 *m_lidar_at128{nullptr};
	// GQueue<StructPointData*> m_queue;
	GQueue<PointView> m_SaveQueue;
	GQueue<PointView> m_ReadQueue;

	int m_pcd_cur_pos{0};
	void doWork();
	void saveProcess();
	void readProcess(QString path);
};

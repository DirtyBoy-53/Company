#ifndef CAMERADISWIDGET_H
#define CAMERADISWIDGET_H

#include <QWidget>
#include <QTimer>
#include "GLYuvWidget.h"
#include "sharedmembuffer.h"
#include "common.h"
// #include "vehviewer.h"
#include "lidar_AT128.h"
#include "radar_ars408.h"


namespace Ui {
class cameradiswidget;
}

class cameradiswidget : public QWidget
{
    Q_OBJECT

public:
    explicit cameradiswidget(QWidget *parent = 0);
    ~cameradiswidget();
    enum ScaleType{
        fitXY,
        fitCenter,
        CenterCorp
    };

private:
    Ui::cameradiswidget *ui;
    Lidar_AT128 *m_lidar_at128{nullptr};
    QTimer *m_timer{nullptr};
    // VehViewer *m_viewer{nullptr};
    unsigned char * m_rgb[PANE_NUM]{0};
    unsigned char * m_y16[PANE_NUM]{0};
    // QVector<GLYuvWidget*> m_showLabel;
    SharedMemBuffer m_reader[PANE_NUM];

    // Camera_Pcie *m_cameraPice{nullptr};
    void init();
    void startGet();

    GLYuvWidget* m_yuvPaint[PANE_NUM];

    bool m_shotSwitch[PANE_NUM];

    QMutex m_mutex;
    bool m_playRadar = false;

    
    
    
public slots:
    void showNewData();
    void slot_oneFrameProc(StructPointData* data);
    // void slot_RadaroneFrameProc(StructRadarDataPtr data);
    void slotShotOne(int id);
    void slot_lidar_view_dir(ENUM_VIEW view);
    void slot_set_lidar_path(QString &path);
    // void slot_set_radar_path(QString &path);
    void slot_lidar_init(QString ip);
    // void slot_radar_init(int port);

signals:
    void signal_points_master(PointView);
    void sign_update_ui();
    //void sigSetYuv(int slot, char* y, char* u, char* v);
    
    void sigSetYuv(int slot, uchar *Yptr,uchar *Uptr,uchar *Vptr,uint width,uint height, int type = (int)fitCenter);
    // void signal_dis_plot(uint32_t cnt,QVector<double> &_id,QVector<double> &_x,QVector<double> &_y);
};

#endif // CAMERADISWIDGET_H

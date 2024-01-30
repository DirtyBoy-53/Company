#ifndef CAMERAPARAMWIDGET_H
#define CAMERAPARAMWIDGET_H

#include <QWidget>
#include "transconfig.h"
#include "recordconfig.h"
#include "common.h"
namespace Ui {
class cameraparamwidget;
}

class cameraparamwidget : public QWidget
{
    Q_OBJECT

public:
    explicit cameraparamwidget(QWidget *parent = 0);
    ~cameraparamwidget();
    

private:
    Ui::cameraparamwidget *ui;
    TransConfig *m_transConfig{nullptr};
    RecordConfig *m_recordConfig{nullptr};
    CameraParam m_cameraParam;
    void init();

public slots:
    void slot_bt_Chick();

signals:
    void signal_camera_configure(CameraParam& param);
    void signal_lidar_view_dir(ENUM_VIEW view);
};

#endif // CAMERAPARAMWIDGET_H

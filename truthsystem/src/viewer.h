#ifndef Viewer_H
#define Viewer_H
#include <QObject>
#include <QDateTime>
#include "QGLViewer/qglviewer.h"
#include <QWidget>
#include <QPushButton>
#include <QSignalMapper>
#include <QThread>
#include <QDebug>

#include "common.h"
using namespace qglviewer;
using namespace std;


class Viewer : public QGLViewer
{
    Q_OBJECT
public:
    Viewer(QWidget* parent = 0, int slot = 0, bool needAssitant = false);

    QList<int> QL_ViewerSelectIds;
    
protected:
    //must overload
    virtual void init();
    virtual void draw();
    virtual void resizeGL(int w, int h);

    // Mouse events functions
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

    virtual void postSelection(const QPoint& point);

private:
    int _nGrid = 10;//圆圈数以及网格数/2
    float _gridR = 10.0f;//网格最小圆半径以及网格单位长度

    st_point _findP;
    bool _find{ false };
    QString m_timestamp{""};
    QDateTime _frameTime;
    // pcl::PointCloud<PointXYZIT>::Ptr _mPointsPtr=nullptr;
    std::shared_ptr<pcl::PointCloud<PointXYZIT>> _mPointsPtr;

    bool m_isShowGrid_flag{ false };
    bool m_isShowPolar_flag{ false };
    bool m_isShowCoordinate{ false };
    bool m_isShowRecord_flag{false};

    void set_display_text(QVector2D& pos, QString& content);
    void print_bitmap_string(void* font, const char* s);
    void drawGLString(float x, float y, float z, const char* cstr);
    void drawCoordinate();
    void drawTips();

signals:


public slots:
    //void updateView(QList<PointXYZ> plistPoints, QVector<PointCloudCluster> pvecCluster);
    void updateViewMaster(PointView);
    void updateViewTime(QString);
    void drawGridAndCircular();
    void convertRgbByIntensity(float intensity, float& r, float& g, float& b);
    void set_camera_view(ENUM_VIEW view);
    void set_projection_mode(Camera::Type model);
    void set_is_show_grid(bool state);
    void set_is_show_polar(bool state);
    void set_is_show_coordinate(bool state);
    void set_is_show_recordTips(bool state);

};
#endif // Viewer_H
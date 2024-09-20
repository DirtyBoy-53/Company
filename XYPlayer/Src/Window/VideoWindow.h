#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QTime>

#include "YFrame.h"
#include "YStruct.h"

class VideoWindow : public QObject
{
    Q_OBJECT
public:
    VideoWindow();
    virtual ~VideoWindow() {}

    virtual void setGeometry(const QRect& rc) = 0;
    virtual void update() = 0;

    virtual void setRecord(bool state);
protected:
    void calcFPS();


public:
    YFramePtr                   last_frame{nullptr};
    YFrame                      frame;
    int                         fps;
    bool                        draw_time{false};
    bool                        draw_fps{false};
    bool                        draw_resolution{false};
    bool                        draw_record{false};
    QTimer*                     recordTimer;
    QTime                       time;
    QString                     timeStr;
    QList<CustomWatermarkInfo>  watermarkList;
protected:
    // for calFPS
    uint64_t                    tick;
    int                         framecnt;
};

#endif // VIDEOWINDOW_H

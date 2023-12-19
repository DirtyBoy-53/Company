#ifndef VIDEOPAINTWIDGET_H
#define VIDEOPAINTWIDGET_H

#include <QObject>
#include <QWidget>
#include <functional>
#include <QPaintEvent>

#include "blockingqueue.h"
#include "framebuffer.h"

class VideoPaintWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPaintWidget(QWidget *parent = 0);
    void setImage(const QImage &img, qint64 timeT);

    void setScaleZoom(double zoomValue);
    void clear();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent* event);

    QWidget* m_parent;
    QImage m_currentImage;

    float m_currentZoomValue;
    float m_zoomValue;
    int m_zoomMode = 0;
    int m_iconSize;
    bool m_isFullScreen = false;


    int m_imgOffset = 0;
};

#endif // VIDEOPAINTWIDGET_H

#include "videopaintwidget.h"
#include <QPainter>
#include <QDebug>
#include "qttoopencv.h"
#include <QTime>

VideoPaintWidget::VideoPaintWidget(QWidget *parent) : m_parent(parent)
{
    m_zoomValue = 1;
}

void VideoPaintWidget::setImage(const QImage &img, qint64 timeT)
{
    m_currentImage = img;
    update();
}

void VideoPaintWidget::setScaleZoom(double zoomValue)
{
    m_zoomValue = zoomValue;
    setMinimumSize(640*m_zoomValue, 512*m_zoomValue);
}

void VideoPaintWidget::clear()
{
    m_currentImage.fill(qRgba(0,0,0,0));
    update();
}

void VideoPaintWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        if(!isFullScreen()) {
            setWindowFlags(Qt::Window);
            showFullScreen();
            float value1 = (float)width()/(float)m_currentImage.width();
            float value2 = (float)height()/(float)m_currentImage.height();
            m_currentZoomValue = m_zoomValue;
            m_zoomValue = value1 < value2 ? value1 : value2;
            m_isFullScreen = true;
            m_imgOffset = (width() - m_currentImage.width()*m_zoomValue)/2;
        } else {
            setWindowFlags(Qt::SubWindow);
            showNormal();
            m_zoomValue = m_currentZoomValue;
            m_isFullScreen = false;
            m_imgOffset = 0;
        }
        setFocus();
        update();
    }
}

void VideoPaintWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if(m_currentImage.isNull()) return;

    painter.drawImage(this->rect(),m_currentImage);
}

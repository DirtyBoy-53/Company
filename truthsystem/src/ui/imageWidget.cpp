
#include "imageWidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTime>


ImageWidget::ImageWidget(QWidget *parent):
    QOpenGLWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_wheelTimer = new QTimer(this);
    connect(m_wheelTimer, &QTimer::timeout, this, &ImageWidget::wheelTimeout);
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::setWidgetIndex(const int index)
{
    m_index = index;
}

void ImageWidget::zoomSwitch(const bool flag)
{
    m_zoomFlag = flag;
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(!m_initFlag) return;
    if(event->button() == Qt::LeftButton) {
//        setFocusPolicy(Qt::StrongFocus);
        emit signalMousePress(m_index, event->pos());
    }
}

//void ImageWidget::keyPressEvent(QKeyEvent *event)
//{
//    if(!m_initFlag) return;
//    emit signalKeyDown(m_index, event->key());
//}

// 布局双击，该布局充满
void ImageWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(!m_initFlag) return;
    if(event->button() == Qt::LeftButton) {
        emit signalMouseDoublePress(m_index);
    }
}

void ImageWidget::wheelEvent(QWheelEvent *event)
{
    if(!m_initFlag) return;

    m_wheelDirection = event->delta();
    startWheelTimer();
}

void ImageWidget::startWheelTimer()
{
    if(m_wheelTimer->isActive()) {
        m_wheelTimer->stop();
    }
    m_wheelTimer->start(500);
}

void ImageWidget::wheelTimeout()
{
    m_wheelTimer->stop();
    emit signalWheelEvent(m_index, m_wheelDirection);
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if(!m_initFlag) return;
    painter.drawPixmap(m_x, m_y, m_curPixmap);
}

void ImageWidget::onShowImg(QImage img, int imgW, int imgH)
{
    m_initFlag = true;

    float scaleImg = (float)imgW/(float)imgH;
    float scaleForm = (float)width()/(float)height();
    int w,h;

    float zoomValue = 1.0;
    if(scaleImg > scaleForm) {
        w = width();
        h = w / scaleImg;
        zoomValue = (float)w/(float)imgW;
        m_x = 0;
        m_y = (height() - h)/2;
    } else {
        h = height();
        w = h * scaleImg;
        zoomValue = (float)h/(float)imgH;
        m_x = (width() - w)/2;
        m_y = 0;
    }

    if(m_zoomFlag) {
        QImage dstImg = m_qConvert.ResizeRgb(img, zoomValue);
        m_curPixmap = QPixmap::fromImage(dstImg);
    } else {
        m_x = (width() - imgW)/2;
        m_y = (height() - imgH)/2;
        m_curPixmap = QPixmap::fromImage(img);
    }

    update();
}

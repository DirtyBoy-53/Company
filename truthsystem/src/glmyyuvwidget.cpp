
#include "glmyyuvwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTime>


GLMyYuvWidget::GLMyYuvWidget(QWidget *parent):
    QOpenGLWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_wheelTimer = new QTimer(this);
    connect(m_wheelTimer, &QTimer::timeout, this, &GLMyYuvWidget::wheelTimeout);
}

GLMyYuvWidget::~GLMyYuvWidget()
{
}

void GLMyYuvWidget::setWidgetIndex(const int index)
{
    m_index = index;
}

void GLMyYuvWidget::zoomSwitch(const bool flag)
{
    m_zoomFlag = flag;
}

void GLMyYuvWidget::mousePressEvent(QMouseEvent *event)
{
    if(!m_initFlag) return;
    if(event->button() == Qt::LeftButton) {
//        setFocusPolicy(Qt::StrongFocus);
        emit signalMousePress(m_index, event->pos());
    }
}

//void GLMyYuvWidget::keyPressEvent(QKeyEvent *event)
//{
//    if(!m_initFlag) return;
//    emit signalKeyDown(m_index, event->key());
//}

// 布局双击，该布局充满
void GLMyYuvWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(!m_initFlag) return;
    if(event->button() == Qt::LeftButton) {
        emit signalMouseDoublePress(m_index);
    }
}

void GLMyYuvWidget::wheelEvent(QWheelEvent *event)
{
    if(!m_initFlag) return;

    m_wheelDirection = event->delta();
    startWheelTimer();
}

void GLMyYuvWidget::startWheelTimer()
{
    if(m_wheelTimer->isActive()) {
        m_wheelTimer->stop();
    }
    m_wheelTimer->start(500);
}

void GLMyYuvWidget::wheelTimeout()
{
    m_wheelTimer->stop();
    emit signalWheelEvent(m_index, m_wheelDirection);
}

void GLMyYuvWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if(!m_initFlag) return;
    painter.drawPixmap(m_x, m_y, m_curPixmap);
}

void GLMyYuvWidget::onShowImg(QImage img, int imgW, int imgH)
{
    QTime time;
    time.start();
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
        // QImage dstImg = m_qConvert.ResizeRgb(img, zoomValue);
        // m_curPixmap = QPixmap::fromImage(dstImg);
        // QImage dstImg = img.scaled(width(), height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_curPixmap = QPixmap::fromImage(img);
        m_curPixmap = m_curPixmap.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        m_x = (width() - imgW)/2;
        m_y = (height() - imgH)/2;
        m_curPixmap = QPixmap::fromImage(img);
    }
    qDebug() << "paint time:" << time.elapsed();
    update();
}

#include "videopaintlabel.h"
#include <QPainter>

VideoPaintLabel::VideoPaintLabel(QWidget *parent) : QWidget(parent)
{
}

void VideoPaintLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!m_crtImage.isNull()) {
        painter.drawImage(0, 0, m_crtImage);
    }
}

void VideoPaintLabel::setImage(unsigned char *src, int width, int height)
{
    QImage img(src, width, height, QImage::Format_RGB888);
    m_crtImage = img;
    update();
}

void VideoPaintLabel::setImage(QImage img)
{
    m_crtImage = img;
    update();
}
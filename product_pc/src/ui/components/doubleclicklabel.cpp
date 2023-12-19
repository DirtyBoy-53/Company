#include "doubleclicklabel.h"
#include <QFile>
#include <QDebug>

DoubleClickLabel::DoubleClickLabel()
{
    m_imageShow = new QDialog();
    m_imageShow->setWindowFlags(Qt::FramelessWindowHint);
    // m_imageShow->setMaximumSize(400, 300);
    m_imageShow->setMinimumSize(400, 300);
    setStyleSheet("background-color: rgba(255,255,255,255)");
    // key functions...
    setMouseTracking(true);
}

void DoubleClickLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit sigDoubleClicked();
}

void DoubleClickLabel::enterEvent(QEvent *event)
{
    QString path = text();
    if (!QFile::exists(path)) {
        setToolTip("没有要显示的图片");
        return;
    }
    QPalette palle;
    QImage image;
    if (!image.load(path)) {
        setToolTip("图片加载失败");
        return;
    }

    QPixmap pix = QPixmap::fromImage(image);
    palle.setBrush(QPalette::Window, QBrush(pix));
    QPoint rct = cursor().pos();
    qDebug() << rct;
    m_imageShow->setGeometry(rct.x(), rct.y(), image.width(), image.height());
    m_imageShow->setPalette(palle);
    m_imageShow->show();
}

void DoubleClickLabel::leaveEvent(QEvent *event)
{
    m_imageShow->hide();
}

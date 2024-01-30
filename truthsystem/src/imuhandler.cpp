#include "imuhandler.h"
#include <QPainter>
#include <QFont>

ImuHandler::ImuHandler()
{

}

void ImuHandler::setImuParam(ImuParams param)
{
    m_imu.setParam(param);
}

void ImuHandler::drawImu(QImage& img, int16_t* imuData, int64_t tmT, int w, int h)
{
    QPainter painter(&img);
    QFont font;
    font.setFamily("Source Sans Pro");
    font.setPixelSize(14);
    painter.setFont(font);


    if(!m_imu.processImu(imuData, w, tmT, m_curVelocity)) return;

    painter.setPen(Qt::green);
    painter.drawText(4, h - font.pixelSize()*3, QString("vx:%1").arg(m_curVelocity.vx));
    painter.drawText(4, h - font.pixelSize()*2, QString("vy:%1").arg(m_curVelocity.vy));
    painter.drawText(4, h - font.pixelSize(), QString("vz:%1").arg(m_curVelocity.vz));
}

#ifndef NVSHANDLER_H
#define NVSHANDLER_H
#include <QImage>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
class NvsHandler
{
public:
    NvsHandler();
    bool drawNvs(QImage &img, QByteArray &nvsData);
    bool hasNvsObj(QByteArray nvsData);

private:
    int m_iAlarming = 0;
    qint64 m_tLastAlarm = 0;
    QPixmap m_imagePeople;
    QPixmap m_imageCar;
    QPixmap m_imageAnimal;
    QPixmap m_imageSound;
    int m_iconSize;
};

#endif // NVSHANDLER_H

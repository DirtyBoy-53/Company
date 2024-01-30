#include "nvshandler.h"
#include <nvs_result.h>
#include <QDateTime>
#include <QDebug>
#include "memcache.h"

NvsHandler::NvsHandler()
{
    m_imageAnimal.load(":/img/animal1.png");
    m_imageCar.load(":/img/car1.png");
    m_imagePeople.load(":/img/people1.png");

    m_iconSize = 45;
}

bool NvsHandler::hasNvsObj(QByteArray nvsData)
{
    BoxMsg_t box;
    memcpy(&box, nvsData.data(), sizeof(box));
    return box.obj_number>0;
}

bool NvsHandler::drawNvs(QImage &img, QByteArray &nvsData)
{
    bool hasObj = false;
    BoxMsg_t box;
    QPen pen;
    QBrush brush;
    QFont font;
    font.setFamily("Source Sans Pro");
    font.setPixelSize(14);

    QPainter painter(&img);
    painter.setFont(font);
    memcpy(&box, nvsData.data(), sizeof(box));
    qint64 tCurTm = QDateTime::currentMSecsSinceEpoch();
    float x,y,w,h;
    for(int i=0; i<box.obj_number; i++) {
        if(i >= MAX_OBJECT_NUMBER) {
            break;
        }

        x = box.bbox[i].coordinates.x1;
        y = box.bbox[i].coordinates.y1 ;
        w = (box.bbox[i].coordinates.x2 - box.bbox[i].coordinates.x1);
        h = (box.bbox[i].coordinates.y2 - box.bbox[i].coordinates.y1);

        if(box.bbox[i].class_id >= 0 && box.bbox[i].class_id <= 6) {
            if(box.bbox[i].is_alarm) {
                pen.setColor(QColor(255,0,0));
                brush.setColor(QColor(255,0,0,20));
            } else {
                pen.setColor(QColor(255,215,0));
                brush.setColor(QColor(255,215,0,20));
            }
        } else if(box.bbox[i].class_id >=7 && box.bbox[i].class_id <= 8) {
            if(box.bbox[i].is_alarm) {
                pen.setColor(QColor(255,0,0));
                brush.setColor(QColor(255,0,0,20));
            } else {
                pen.setColor(QColor(60,179,113));
                brush.setColor(QColor(60,179,113,20));
            }
        } else if(box.bbox[i].class_id >= 10 && box.bbox[i].class_id <= 12) {
            if(box.bbox[i].is_alarm) {
                pen.setColor(QColor(255,0,0));
                brush.setColor(QColor(255,0,0,20));
            } else {
                pen.setColor(QColor(230,203,247));
                brush.setColor(QColor(230,203,247,20));
            }
        } else {
            pen.setColor(QColor(255,215,0));
            brush.setColor(QColor(255,215,0,20));
        }

        if(box.is_alarm_frame == 0x1 || box.is_alarm_frame == 0x2 || box.is_alarm_frame == 0x3) {
            m_iAlarming = box.is_alarm_frame;
            m_tLastAlarm = tCurTm;
        }

        pen.setWidth(3);
        brush.setStyle(Qt::SolidPattern);
        painter.setPen(pen);
        painter.drawRect(x, y, w, h);
        QPainterPath painterPath;
        painterPath.addRect(x, y, w, h);
        painter.fillPath(painterPath, brush);

        font.setPixelSize(10);
        painter.setFont(font);
        painter.setPen(QColor(255,50,200));
        QString text = QString::number(box.bbox[i].position.y, 'f', 1) + "m";
        painter.drawText(x+4, y+font.pixelSize(), text);
        QString text1 = QString::number(box.bbox[i].position.x, 'f', 1) + "m";
        painter.drawText(x+4, y+font.pixelSize()*2, text1);
        hasObj = true;
    }
    int pixmapX = img.width()/2 - m_iconSize/2 ;
    if(m_tLastAlarm + 3000 > tCurTm) {
        if(m_iAlarming == 0x1) {
            painter.drawPixmap(pixmapX, 10, m_iconSize, m_iconSize, m_imageCar);
        } else if(m_iAlarming == 0x2) {
            painter.drawPixmap(pixmapX, 10, m_iconSize, m_iconSize, m_imagePeople);
        } else if(m_iAlarming == 0x3) {
            painter.drawPixmap(pixmapX, 10, m_iconSize, m_iconSize, m_imageAnimal);
        }
    }else {
        m_tLastAlarm = 0;
    }

    font.setPixelSize(14);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(5,15,QString::number(box.obj_frame_id));
    painter.drawText(5,35,QDateTime::fromMSecsSinceEpoch(box.obj_frame_id, Qt::UTC).toString("yyyy_MM_dd_hh_mm_ss_zzz"));

    painter.setPen(Qt::green);
    qDebug() << "lines number " << box.line_number;
    for(int i=0; i<box.line_number; i++) {
        painter.drawLine(box.lines[i].x0, box.lines[i].y0,
                         box.lines[i].x1, box.lines[i].y1);
    }
    return hasObj;
}

﻿#include "yrectangle.h"

YRectangle::YRectangle(const YShape::draw_mode_e type)
    : YShape::Shape(type)
{

}

void YRectangle::draw(QPainter* p, bool isdisEndPt, bool fill)
{
    Q_UNUSED(p) Q_UNUSED(isdisEndPt) Q_UNUSED(fill)
    if(m_points.size() <= 0) return;

    // build  rectangle
    QPainterPath path;
    QColor color(0, 255, 0);
    QColor fill_color(m_color.red(), m_color.green(), m_color.blue(), 100);

    auto points = m_points.getOriPoints();
    if (isdisEndPt)
        points.append(m_disPoit);
    auto p1 = points.at(0), p2 = points.at(1);
    path.addRect(QRectF(p1,p2));


    QPen pen(m_color, m_lineWidth, Qt::SolidLine);
    p->save();
    // Draw the Rectangle
    if (m_isClosed) {

        if (m_isActive || m_isSelect) {
            if (m_isSelect) {
                pen.setColor(Qt::white);
            }
            else {
                pen.setColor(m_color);
            }

        }
//        path.closeSubpath();
        p->setPen(pen);
    }
    else {
        p->setBrush(QColor(10, 0, 0, 100));
        p->setPen(QPen(color, m_lineWidth, Qt::SolidLine));
    }

    p->drawPath(path);
    p->restore();


    // Draw the control points
    if (m_isClosed) {

        QPen ptPen(m_color, m_lineWidth / 2, Qt::SolidLine);
        if (m_isSelect) {
            ptPen.setColor(Qt::white);
        }

        p->save();
        p->setPen(ptPen);
        p->setBrush(QColor(m_color));
        if (m_controlPtActive >= 0) {
            p->setBrush(QBrush(QColor(Qt::white), Qt::SolidPattern));
        }
        for (int i = 0; i < points.size(); ++i) {
            QPointF pos = points.at(i); auto ptSize = m_pointSize * 1.25;
            QRectF rect(pos.x() - ptSize, pos.y() - ptSize, ptSize * 2, ptSize * 2);
            if (m_controlPtActive == i) {
                ptSize = m_pointSize * 1.75;
                rect.setTopLeft(QPointF(pos.x() - ptSize, pos.y() - ptSize));
                rect.setSize(QSize(ptSize * 2, ptSize * 2));
                p->drawRect(rect);
            }
            else {
                p->drawEllipse(rect);
            }
        }
        p->restore();
    }
    else {
        p->setPen(color);
        p->setBrush(color);
        for (int i = 0; i < points.size(); ++i) {
            QPointF pos = points.at(i);
            QRectF rect(pos.x() - m_pointSize, pos.y() - m_pointSize, m_pointSize * 2, m_pointSize * 2);
            p->drawEllipse(rect);
        }
    }
    if (m_isActive || m_isSelect || m_controlPtActive >= 0)
        p->fillPath(path, fill_color);


}

bool YRectangle::isAreaHavPt(const QPointF &pt)
{
    if(m_points.size() < 2) return false;
    QRectF rect(m_points.getOriPoints().at(0),m_points.getOriPoints().at(1));
    return rect.contains(pt);
}

bool YRectangle::insertPoint(const QPointF &pt, int &pos)
{
    return false;
}
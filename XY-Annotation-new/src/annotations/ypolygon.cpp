#include "ypolygon.h"
#include <QPainterPath>
#include "labelmanager.h"
YPolygon::YPolygon(const YShape::draw_mode_e type)
    : YShape::Shape(type)
{

}
#include <QDebug>
void YPolygon::draw(QPainter *p, bool isdisEndPt, bool fill)
{
    //    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

    QPainterPath path;
    QColor color(0, 255, 0);

    auto points = m_points;
    if (isdisEndPt)
        points.append(m_disPoit);
    QPolygonF polygon(points.getOriPoints());
    path.addPolygon(polygon);

    // Draw the Polygon
    if (m_isClosed) {
        path.closeSubpath();
        p->setPen(QPen(m_color, m_lineWidth, Qt::SolidLine));
    }
    else {
        p->setBrush(QColor(10, 0, 0, 100));
        p->setPen(QPen(color, m_lineWidth, Qt::SolidLine));
    }

    if (fill) path.setFillRule(Qt::WindingFill);

    p->drawPath(path);

    // Draw the control points
    if (m_isClosed) {
        p->setBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 255));
        p->setPen(QPen(m_color));
        for (int i = 0; i < points.size(); ++i) {
            QPointF pos = points.at(i);
            p->drawEllipse(QRectF(pos.x() - m_pointSize,
                pos.y() - m_pointSize,
                m_pointSize * 2, m_pointSize * 2));
        }
        p->setPen(QPen(m_color, m_lineWidth, Qt::SolidLine));
        p->setBrush(Qt::NoBrush);
        p->drawPolyline(points.getOriPoints());
    }
    else {
        p->setPen(color);
        p->setBrush(color);
        for (int i = 0; i < points.size(); ++i) {
            QPointF pos = points.at(i);
            p->drawEllipse(QRectF(pos.x() - m_pointSize,
                pos.y() - m_pointSize,
                m_pointSize * 2, m_pointSize * 2));
        }
    }
}


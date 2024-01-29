#include "ypolygon.h"
#include <QPainterPath>

YPolygon::YPolygon(const YShape::draw_mode_e type)
    : YShape::Shape(type)
{

}

void YPolygon::draw(QPainter &p, bool fill)
{
    QPainterPath path;
    QPolygonF polygon(m_points);
    path.addPolygon(polygon);
    // Draw the Polygon
    if(m_isClosed){
        path.closeSubpath();
        p.setPen(QPen(m_color, m_lineWidth, Qt::SolidLine));
    }else{
        p.setBrush(QColor(10, 0, 0,100));
        p.setPen(QPen(Qt::green, m_lineWidth, Qt::SolidLine));
    }

    if (fill) path.setFillRule(Qt::WindingFill);
    p.save();


    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(path);


    // Draw the control points
    if(m_isClosed){
        p.setBrush(QColor(m_color.red(), m_color.green(), m_color.blue(),255));
        p.setPen(QPen(m_color));
        for (int i=0; i<m_points.size(); ++i) {
            QPointF pos = m_points.at(i);
            p.drawEllipse(QRectF(pos.x() - m_pointSize,
                                 pos.y() - m_pointSize,
                                 m_pointSize*2, m_pointSize*2));
        }
        p.setPen(QPen(m_color, m_lineWidth, Qt::SolidLine));
        p.setBrush(Qt::NoBrush);
        p.drawPolyline(m_points);
    }else{
        p.setPen(QColor(0, 255, 0, 255));
        p.setBrush(QColor(0, 255, 0, 255));
        for (int i=0; i<m_points.size(); ++i) {
            QPointF pos = m_points.at(i);
            p.drawEllipse(QRectF(pos.x() - m_pointSize,
                                 pos.y() - m_pointSize,
                                 m_pointSize*2, m_pointSize*2));
        }
    }
    p.restore();
}

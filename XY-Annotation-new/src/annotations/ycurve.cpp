#include "ycurve.h"


yCurve::yCurve(const YShape::draw_mode_e type)
    : YShape::Shape(type)
{

}

// 使用 De Casteljau 算法计算贝塞尔曲线上的某个点
QPointF deCasteljau(const QVector<QPointF>& controlPoints, double t) {
    QVector<QPointF> points = controlPoints;  // 创建临时副本
    int n = points.size();

    // 递归计算贝塞尔曲线上的点
    for (int k = 1; k < n; ++k) {
        for (int i = 0; i < n - k; ++i) {
            points[i] = (1 - t) * points[i] + t * points[i + 1];
        }
    }
    return points[0];  // 返回计算出的曲线上的点
}

void yCurve::draw(QPainter *p, bool isdisEndPt, bool fill)
{
    Q_UNUSED(p) Q_UNUSED(isdisEndPt) Q_UNUSED(fill)
    if(m_points.size() <= 0) return;
    QPainterPath path;
    QColor color(0, 255, 0);
    QColor fill_color(m_color.red(), m_color.green(), m_color.blue(), 100);

    auto points = m_points.getOriPoints();
    if (isdisEndPt)
        points.append(m_disPoit);
    auto p1 = points.at(0);

    QPen pen(m_color, m_lineWidth, Qt::SolidLine);
    p->save();
    path.moveTo(p1);

    // 计算贝塞尔曲线上若干点，绘制曲线
    const int numSteps = 100;  // 曲线分成多少段
    for (int i = 1; i <= numSteps; ++i) {
        double t = i / static_cast<double>(numSteps);  // 参数 t 范围为 [0, 1]
        QPointF pointOnCurve = deCasteljau(points, t);  // 计算 t 时刻的曲线点
        path.lineTo(pointOnCurve);  // 连接到计算出的点
    }


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
//        p->setBrush(color);
        for (int i = 0; i < points.size(); ++i) {
            QPointF pos = points.at(i);
            QRectF rect(pos.x() - m_pointSize, pos.y() - m_pointSize, m_pointSize * 2, m_pointSize * 2);
            p->drawEllipse(rect);
        }
    }

    p->setPen(pen);
    p->drawPath(path);
    p->restore();
}

bool yCurve::isAreaHavPt(const QPointF &pt)
{
    return false;
}

bool yCurve::isEdgeHavPt(const QPointF &pt, int &pos)
{
    return false;
}

bool yCurve::insertPoint(const QPointF &pt, int &pos)
{
    return false;
}

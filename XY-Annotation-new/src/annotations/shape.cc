#include "shape.h"
#include <QDebug>
#include <math.h>
#include "labelmanager.h"
using namespace YShape;

YShape::Shape::Shape(const draw_mode_e& type)
    : m_type(type)
    , m_label(new LabelProperty)
{

}

std::string Shape::drawModeToStr(const draw_mode_e& type)
{
    switch (type) {
    case Rectangle: return "rectangle"; break;
    case Line: return "line"; break;
    case Curve: return "curve"; break;
    case Polygon: return "polygon"; break;
    case None: return "none"; break;
    default: return ""; break;
    }
}

QString Shape::name() const
{
    return m_name;
}

int Shape::id() const
{
    return m_id;
}

QColor Shape::color() const
{
    return m_color;
}

bool Shape::isClosed() const
{
    return m_isClosed;
}

void Shape::setColor(const QColor& color)
{
    m_color = color;
}

bool Shape::isPtsHavPt(const QPointF& pt, int& pos)
{
    for (auto i = 0; i < m_points.size(); ++i) {
        auto curDis = QPointF(m_points.at(i) - pt).manhattanLength();
        if (curDis <= m_senseDis) {
            pos = i;
            return true;
        }
    }
    return false;
}

QPointF GetFootOfPerpendicular(
    const QPointF& pt,
    const QPointF& begin,
    const QPointF& end)
{
    double dx = begin.x() - end.x();
    double dy = begin.y() - end.y();
    if (fabs(dx) < 0x00000001 && fabs(dy) < 0x00000001) {
        return begin;
    }
    double u = (pt.x() - begin.x()) * (begin.x() - end.x()) +
        (pt.y() - begin.y()) * (begin.y() - end.y());
    u = u / ((dx * dx) + (dy * dy));
    double x = begin.x() + u * dx;
    double y = begin.y() + u * dy;
    return QPointF(x, y);
}

//Բ
struct Circle {
public:
    Circle() = default;
    Circle(double r, const QPointF& pt) :r(r), x(pt.x()), y(pt.y()) {}
    double r, x, y;
};
bool judge(QPointF p1, QPointF p2, Circle c)
{
    bool flag1 = (p1.x() - c.x) * (p1.x() - c.x) + (p1.y() - c.y) * (p1.y() - c.y) <= c.r * c.r;
    bool flag2 = (p2.x() - c.x) * (p2.x() - c.x) + (p2.y() - c.y) * (p2.y() - c.y) <= c.r * c.r;

    if (flag1 && flag2)	//���һ�����㶼��Բ�� :һ�����ཻ
        return false;
    else if (flag1 || flag2) //�������һ������Բ�ڣ�һ������Բ�⣺һ���ཻ
        return false;//�����ཻ�����ǻᵼ���߼����ң������Ͽ�
    else //������������㶼��Բ��
    {
        double A, B, C, dist1, dist2, angle1, angle2;
        //��ֱ��p1p2��Ϊһ��ʽ��Ax+By+C=0����ʽ���Ȼ�Ϊ����ʽ��Ȼ��������ʽ�ó�һ��ʽ
        A = p1.y() - p2.y();
        B = p2.x() - p1.x();
        C = p1.x() * p2.y() - p2.x() * p1.y();
        //ʹ�þ��빫ʽ�ж�Բ�ĵ�ֱ��ax+by+c=0�ľ����Ƿ���ڰ뾶
        dist1 = A * c.x + B * c.y + C;
        dist1 *= dist1;
        dist2 = (A * A + B * B) * c.r * c.r;
        if (dist1 > dist2)//Բ�ĵ�ֱ��p1p2�ľ�����ڰ뾶�����ཻ
            return false;
        angle1 = (c.x - p1.x()) * (p2.x() - p1.x()) + (c.y - p1.y()) * (p2.y() - p1.y());
        angle2 = (c.x - p2.x()) * (p1.x() - p2.x()) + (c.y - p2.y()) * (p1.y() - p2.y());
        if (angle1 > 0 && angle2 > 0)//����Ϊ����������ǣ�һ���ཻ
            return true;
        else
            return false;
    }
    return false;
}


bool Shape::isEdgeHavPt(const QPointF& point, int& pos)
{
    auto size = m_points.size();
    if (size < 2) return false;
    for (auto i = 0; i < size; i++) {
        QPointF begin = m_points.at(i), end;
        if (i + 1 < size)
            end = m_points.at(i + 1);
        else end = m_points.at(0);

        if (!judge(begin, end, Circle(m_senseDis, point))) continue;
        QPointF FootPt = GetFootOfPerpendicular(point, begin, end);
        auto curDis = (point - FootPt).manhattanLength();
        if (curDis <= m_senseDis) {
            pos = i + 1;
            return true;
        }
    }
    return false;
}

bool Shape::isAreaHavPt(const QPointF& point)
{
    return m_points.containsPoint(point);
}

bool Shape::updatePoint(const QPointF& point, int& pos)
{
    if (pos < m_points.size() && pos >= 0) {
        m_points.replace(point, pos);
        return true;
    }
    else throw ShapeException("<pos> out of range");
    return false;
}

bool Shape::insertPoint(const QPointF& point, int& pos)
{
    if (pos < m_points.size() && pos >= 0) {
        m_points.insert(point, pos);
    }
    else if (pos == m_points.size()) {
        m_points.append(point);
    }
    return true;
}

bool Shape::move(const QPointF& point)
{
    for (auto i = 0; i < m_points.size(); i++) {
        m_points.replace((m_points.at(i) + point), i);
    }
    return true;
}

draw_mode_e Shape::type() const
{
    return m_type;
}

void Shape::setType(draw_mode_e newType)
{
    m_type = newType;
}

void Shape::setName(const QString& newName)
{
    m_name = newName;
}

void Shape::appendPoint(const QPointF& point)
{
    m_points.append(point);
}

void Shape::updateEndPt(const QPointF& point)
{
    m_disPoit = point;
}



void Shape::deletePoint(const int& index)
{
    if (index >= m_points.size()) return;
    m_points.erase(index);
}

const YPolygonPoints& Shape::points() const
{
    return m_points;
}

LabelProperty* Shape::label() const
{
    return m_label;
}

void Shape::setLabel(const LabelProperty& label)
{
    *m_label = label;
    m_color = label.m_color;
}

bool Shape::isDrag() const
{
    return m_isDrag;
}

void Shape::setIsDrag(bool isDrag)
{
    m_isDrag = isDrag;
}

bool Shape::isPress() const
{
    return m_isPress;
}

bool Shape::isActive() const
{
    return m_isActive;
}

bool YShape::Shape::isSelect() const
{
    return m_isSelect;
}

void Shape::setIsPress(bool isPress)
{
    m_isPress = isPress;
}

void Shape::setIsActive(bool active)
{
    m_isActive = active;
}

void YShape::Shape::setIsSelect(bool select)
{
    m_isSelect = select;
}

void Shape::setControlPtActive(int active)
{
    m_controlPtActive = active;
}

void Shape::setIsClosed(bool isClosed)
{
    m_isClosed = isClosed;
}

qreal Shape::lineWidth() const
{
    return m_lineWidth;
}

void Shape::setLineWidth(qreal newLineWidth)
{
    m_lineWidth = newLineWidth;
}

int Shape::pointSize() const
{
    return m_pointSize;
}

void Shape::setPointSize(int newPointSize)
{
    m_pointSize = newPointSize;
}

int YShape::Shape::pointCount() const
{
    return m_points.size();
}

QString Shape::typeToString(draw_mode_e type)
{
    switch (type) {
    case YShape::Rectangle:
        return "Rectangle"; break;
    case YShape::Line:
        return "Line"; break;
    case YShape::Curve:
        return "Curve"; break;
    case YShape::Polygon:
        return "Ploygon"; break;
    default:
        return QString(""); break;
    }
}
draw_mode_e Shape::stringToType(QString& str, bool& result)
{
    if (str == "Rectangle") {
        result = false;
        return YShape::Rectangle;
    }
    else if (str == "Line") {
        result = false;
        return YShape::Line;
    }
    else if (str == "Curve") {
        result = false;
        return YShape::Curve;
    }
    else if (str == "Polygon") {
        result = false;
        return YShape::Polygon;
    }
    else {}

    result = true;
    return YShape::None;
}

inline const QPointF YShape::YPolygonPoints::at(const int& idx)
{
    if (size() < idx)
        throw ShapeException(std::string(__FUNCDNAME__) + "idx out of range");
    return m_oriPoints.at(idx);
}

inline void YShape::YPolygonPoints::replace(const QPointF& point, int& pos)
{
    if (size() < pos)
        throw ShapeException(std::string(__FUNCDNAME__) + "idx out of range");
    m_oriPoints.replace(pos, point);
    m_imgPoints.replace(pos, point + offset);
}

inline void YShape::YPolygonPoints::insert(const QPointF& point, int& pos)
{
    if (size() < pos)
        throw ShapeException(std::string(__FUNCDNAME__) + "idx out of range");
    m_oriPoints.insert(pos, point);
    m_imgPoints.insert(pos, point + offset);
}

void YShape::YPolygonPoints::erase(const int& idx)
{
    if (m_oriPoints.begin() + idx >= m_oriPoints.end())
        throw ShapeException(std::string(__FUNCDNAME__) + "idx out of range");
    m_oriPoints.erase(m_oriPoints.begin() + idx);
    m_imgPoints.erase(m_imgPoints.begin() + idx);
}

bool YShape::YPolygonPoints::containsPoint(const QPointF& point)
{
    return m_oriPoints.containsPoint(point, Qt::WindingFill);
}

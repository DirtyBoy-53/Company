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
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method";
        return false;
    }
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

//圆
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

    if (flag1 && flag2)	//情况一、两点都在圆内 :一定不相交
        return false;
    else if (flag1 || flag2) //情况二、一个点在圆内，一个点在圆外：一定相交
        return false;//尽管相交，但是会导致逻辑混乱，不予认可
    else //情况三、两个点都在圆外
    {
        double A, B, C, dist1, dist2, angle1, angle2;
        //将直线p1p2化为一般式：Ax+By+C=0的形式。先化为两点式，然后由两点式得出一般式
        A = p1.y() - p2.y();
        B = p2.x() - p1.x();
        C = p1.x() * p2.y() - p2.x() * p1.y();
        //使用距离公式判断圆心到直线ax+by+c=0的距离是否大于半径
        dist1 = A * c.x + B * c.y + C;
        dist1 *= dist1;
        dist2 = (A * A + B * B) * c.r * c.r;
        if (dist1 > dist2)//圆心到直线p1p2的距离大于半径，不相交
            return false;
        angle1 = (c.x - p1.x()) * (p2.x() - p1.x()) + (c.y - p1.y()) * (p2.y() - p1.y());
        angle2 = (c.x - p2.x()) * (p1.x() - p2.x()) + (c.y - p2.y()) * (p1.y() - p2.y());
        if (angle1 > 0 && angle2 > 0)//余弦为正，则是锐角，一定相交
            return true;
        else
            return false;
    }
    return false;
}


bool Shape::isEdgeHavPt(const QPointF& point, int& pos)
{
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method";
        return false;
    }
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
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method.";
        return false;
    }
    return m_points.containsPoint(point);
}

bool Shape::updatePoint(const QPointF& point, int& pos)
{
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method.";
        return false;
    }
    if (pos < m_points.size() && pos >= 0) {
        m_points.replace(point, pos);
        return true;
    }
    else throw ShapeException("<pos> out of range");
    return false;
}

bool Shape::insertPoint(const QPointF& point, int& pos)
{
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method.";
        return false;
    }
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
    if (m_type == YShape::Rectangle || m_type == YShape::Line) {
        qInfo() << "cur type:" << m_name << " Unable to use, need to override this method.";
        return false;
    }
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

const YPolygonPoints &Shape::points() const
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

bool Shape::isSelect() const
{
    return m_isSelect;
}

void Shape::setIsSelect(bool isSelect)
{
    m_isSelect = isSelect;
}

void Shape::setIsClosed(bool isClosed)
{
    m_isClosed = isClosed;
}

int Shape::lineWidth() const
{
    return m_lineWidth;
}

void Shape::setLineWidth(int newLineWidth)
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
    m_oriPoints.replace(pos, point);
    m_imgPoints.replace(pos, point + offset);
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

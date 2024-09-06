#ifndef YCURVE_H
#define YCURVE_H

#include <shape.h>
class yCurve : public YShape::Shape
{
public:
    yCurve() = delete;
    explicit yCurve(const YShape::draw_mode_e type);

    // Shape interface
public:
    virtual void draw(QPainter *p, bool isdisEndPt, bool fill) override;
    virtual bool isAreaHavPt(const QPointF &pt) override;
    virtual bool isEdgeHavPt(const QPointF &pt, int &pos) override;
    virtual bool insertPoint(const QPointF &pt, int &pos) override;
};

#endif // YCURVE_H

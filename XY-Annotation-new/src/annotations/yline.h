#ifndef YLINE_H
#define YLINE_H

#include "shape.h"

class yLine : public YShape::Shape
{
public:
    yLine() = delete;
    explicit yLine(const YShape::draw_mode_e type);
    // Shape interface
public:
    virtual void draw(QPainter *p, bool isdisEndPt, bool fill) override;
    virtual bool isAreaHavPt(const QPointF &pt) override;
    virtual bool isEdgeHavPt(const QPointF &pt, int &pos) override;
    virtual bool insertPoint(const QPointF &pt, int &pos) override;
};

#endif // YLINE_H

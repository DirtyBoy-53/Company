#ifndef YPOLYLINE_H
#define YPOLYLINE_H

#include "shape.h"
class YPolyline : public YShape::Shape
{
public:
    YPolyline() = delete;
    explicit YPolyline(const YShape::draw_mode_e type);

    // Shape interface
public:
    virtual void draw(QPainter *p, bool isdisEndPt, bool fill) override;
    virtual bool isAreaHavPt(const QPointF &pt) override;

};

#endif // YPOLYLINE_H

#ifndef YPOLYGON_H
#define YPOLYGON_H
#include "shape.h"

class YPolygon : public YShape::Shape
{
public:
    YPolygon() = delete;
    explicit YPolygon(const YShape::draw_mode_e type);

    // Shape interface
public:
    virtual void draw(QPainter* p, bool isdisEndPt, bool fill) override;

};


#endif // YPOLYGON_H

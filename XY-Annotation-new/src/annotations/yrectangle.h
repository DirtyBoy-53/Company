#ifndef YRECTANGLE_H
#define YRECTANGLE_H
#include "shape.h"

class YRectangle : public YShape::Shape
{
public:
    YRectangle() = delete;
    explicit YRectangle(const YShape::draw_mode_e type);

    // Shape interface
public:
    virtual void draw(QPainter *p, bool isdisEndPt, bool fill) override;
};

#endif // YRECTANGLE_H

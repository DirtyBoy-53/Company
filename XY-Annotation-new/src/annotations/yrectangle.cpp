#include "yrectangle.h"

YRectangle::YRectangle(const YShape::draw_mode_e type)
    : YShape::Shape(type)
{

}

void YRectangle::draw(QPainter* p, bool isdisEndPt, bool fill)
{
    Q_UNUSED(p) Q_UNUSED(isdisEndPt) Q_UNUSED(fill)
}

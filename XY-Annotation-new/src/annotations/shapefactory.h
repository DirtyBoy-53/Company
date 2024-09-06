#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H
#include "shape.h"
#include "CanvasBase.h"
#include "ypolygon.h"
#include "yrectangle.h"
#include "yline.h"
#include "ycurve.h"
#include "ypolyline.h"
class ShapeFactory{
public:
    static ShapePtr create(const YShape::draw_mode_e type){
        switch(type){
        case YShape::Rectangle :
            return std::make_shared<YRectangle>(type);
            break;
        case YShape::Line :
            return std::make_shared<yLine>(type);
            break;
        case YShape::Curve :
            return std::make_shared<yCurve>(type);
            break;
        case YShape::Polygon :
            return std::make_shared<YPolygon>(type);
            break;
        case YShape::Polyline :
            return std::make_shared<YPolyline>(type);
            break;
        default :
            return nullptr;
        }
        return nullptr;
    }
};


#endif // SHAPEFACTORY_H

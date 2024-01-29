#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H
#include "shape.h"
#include "ypolygon.h"
#include "documentbase.h"
class ShapeFactory{
public:
    static ShapePtr create(const YShape::draw_mode_e type){
        switch(type){
        case YShape::Rectangle :
            break;
        case YShape::Line :

            break;
        case YShape::Curve :

            break;
        case YShape::Polygon :
            return std::make_shared<YPolygon>(type);
            break;
        default :
            return nullptr;
        }
        return nullptr;
    }
};


#endif // SHAPEFACTORY_H

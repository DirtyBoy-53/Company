#ifndef OBJECT_H
#define OBJECT_H

#include "QGLViewer/frame.h"
#include "common.h"
using namespace qglviewer;
class Object{
    //颜色
    float fr;
    float fg;
    float fb;
    //坐标
    Vec pos3d;
    Vec pos2d;//投影到屏幕的坐标
    //点序
    int id;
    bool IsSelect;//是否被选中

    Object(float x_,float y_,float z_,float r_,float g_,float b_);
};
#endif
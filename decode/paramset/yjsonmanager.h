#ifndef YJSONMANAGER_H
#define YJSONMANAGER_H

#include "fifo_map.hpp"
#include "json.hpp"
#include <fstream>
#include <QDebug>
#include <QVector>

namespace shape_json {
    struct root_s{
        //相机内参
        double camera_in_11_fx  ;
        double camera_in_12     ;
        double camera_in_13_Cx  ;
        double camera_in_21     ;
        double camera_in_22_fy  ;
        double camera_in_23_Cy  ;
        double camera_in_31     ;
        double camera_in_32     ;
        double camera_in_33     ;

        //旋转矩阵
        double rotate_X;
        double rotate_Y;
        double rotate_Z;
        //平移向量
        double translate_X;
        double translate_Y;
        double translate_Z;

        //畸变系数
        double distortion_k1;
        double distortion_k2;
        double distortion_k3;
        double distortion_p1;
        double distortion_p2;

        //毫米波单应性矩阵
        double radar_in_11;
        double radar_in_12;
        double radar_in_13;
        double radar_in_21;
        double radar_in_22;
        double radar_in_23;
        double radar_in_31;
        double radar_in_32;
        double radar_in_33;
    };
}

class YJsonManager
{
public:
    YJsonManager() = delete;
    static bool readJson(const std::string &filename, shape_json::root_s &root);
    static bool writeJson(const std::string &filename, const shape_json::root_s &root);
};

#endif // YJSONMANAGER_H

#include "yjsonmanager.h"

template<class K, class V, class dummy_compare, class A>
using YWorkaRoundFifoMap = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using YJson = nlohmann::basic_json<YWorkaRoundFifoMap>;

namespace shape_json {
    void from_json(const YJson &j, root_s &root){
        //相机内参
        root.camera_in_11_fx = j.at("camera_in_11_fx");
        root.camera_in_12    = j.at("camera_in_12");
        root.camera_in_13_Cx = j.at("camera_in_13_Cx");
        root.camera_in_21    = j.at("camera_in_21");
        root.camera_in_22_fy = j.at("camera_in_22_fy");
        root.camera_in_23_Cy = j.at("camera_in_23_Cy");
        root.camera_in_31    = j.at("camera_in_31");
        root.camera_in_32    = j.at("camera_in_32");
        root.camera_in_33    = j.at("camera_in_33");

        //旋转矩阵
        root.rotate_X = j.at("rotate_X");
        root.rotate_Y = j.at("rotate_Y");
        root.rotate_Z = j.at("rotate_Z");
        //平移向量
        root.translate_X = j.at("translate_X");
        root.translate_Y = j.at("translate_Y");
        root.translate_Z = j.at("translate_Z");

        //畸变系数
        root.distortion_k1 = j.at("distortion_k1");
        root.distortion_k2 = j.at("distortion_k2");
        root.distortion_k3 = j.at("distortion_k3");
        root.distortion_p1 = j.at("distortion_p1");
        root.distortion_p2 = j.at("distortion_p2");

        //毫米波单应性矩阵
        root.radar_in_11 = j.at("radar_in_11");
        root.radar_in_12 = j.at("radar_in_12");
        root.radar_in_13 = j.at("radar_in_13");
        root.radar_in_21 = j.at("radar_in_21");
        root.radar_in_22 = j.at("radar_in_22");
        root.radar_in_23 = j.at("radar_in_23");
        root.radar_in_31 = j.at("radar_in_31");
        root.radar_in_32 = j.at("radar_in_32");
        root.radar_in_33 = j.at("radar_in_33");
    }
    void to_json(YJson &j, const root_s &root){
        j["camera_in_11_fx"] = root.camera_in_11_fx;
        j["camera_in_12"]    = root.camera_in_12   ;
        j["camera_in_13_Cx"] = root.camera_in_13_Cx;
        j["camera_in_21"]    = root.camera_in_21   ;
        j["camera_in_22_fy"] = root.camera_in_22_fy;
        j["camera_in_23_Cy"] = root.camera_in_23_Cy;
        j["camera_in_31"]    = root.camera_in_31   ;
        j["camera_in_32"]    = root.camera_in_32   ;
        j["camera_in_33"]    = root.camera_in_33   ;

        j["rotate_X"] = root.rotate_X;
        j["rotate_Y"] = root.rotate_Y;
        j["rotate_Z"] = root.rotate_Z;

        j["translate_X"] = root.translate_X;
        j["translate_Y"] = root.translate_Y;
        j["translate_Z"] = root.translate_Z;

        j["distortion_k1"] = root.distortion_k1;
        j["distortion_k2"] = root.distortion_k2;
        j["distortion_k3"] = root.distortion_k3;
        j["distortion_p1"] = root.distortion_p1;
        j["distortion_p2"] = root.distortion_p2;

        j["radar_in_11"] = root.radar_in_11;
        j["radar_in_12"] = root.radar_in_12;
        j["radar_in_13"] = root.radar_in_13;
        j["radar_in_21"] = root.radar_in_21;
        j["radar_in_22"] = root.radar_in_22;
        j["radar_in_23"] = root.radar_in_23;
        j["radar_in_31"] = root.radar_in_31;
        j["radar_in_32"] = root.radar_in_32;
        j["radar_in_33"] = root.radar_in_33;
    }
}

bool YJsonManager::readJson(const std::string &filename, shape_json::root_s &root)
{
    try{
        std::ifstream i(filename);
        YJson j;
        i >> j;
        shape_json::from_json(j, root);
    }catch(std::exception e){
        qWarning() << e.what();
        return false;
    }
    return true;
}

bool YJsonManager::writeJson(const std::string &filename, const shape_json::root_s &root)
{
    try{
        YJson j;
        shape_json::to_json(j, root);
        std::ofstream o(filename);
        o << j.dump(4) << std::endl;// 4表示缩进空格数
    }catch(std::exception e){
        qWarning() << e.what();
        return false;
    }
    return true;
}

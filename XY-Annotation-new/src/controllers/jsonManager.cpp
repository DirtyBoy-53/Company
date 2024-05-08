#include "jsonManager.h"

template<class K, class V, class dummy_compare, class A>
using YWorkaRoundFifoMap = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using YJson = nlohmann::basic_json<YWorkaRoundFifoMap>;

namespace shape_json {
    void from_json(const YJson& j, QPointF &point) {
        point.setX((double)j[0]);
        point.setY((double)j[1]);
    }
    void to_json(YJson& j, const QPointF &point) {
        j.emplace_back(YJson::array({point.x(),point.y()}));
    }

    void from_json(const YJson& j, shape_s &shape) {
        shape.label         = j.at("label").get<std::string>();
        shape.description   = j.at("description").get<std::string>();
        shape.shape_type    = j.at("shape_type").get<std::string>();
        shape.flags         = j.at("flags").get<std::string>();
        shape.group_id      = j.at("group_id").get<int>();
        shape.mask          = j.at("mask").get<int>();
        for(auto &points : j["points"]){
            QPointF p;
            from_json(points, p);
            shape.points.push_back(p);
        }
    }
    void to_json(YJson& j, const shape_s &shape) {
        j["label"]  = shape.label;
        YJson obj_point;
        for(const QPointF &var : shape.points){
            to_json(obj_point, var);
        }
        j["points"]     = obj_point;
        j["group_id"]   =  shape.group_id;
        j["description"]= shape.description;
        j["shape_type"] = shape.shape_type;
        j["flags"]      = shape.flags;
        j["mask"]       = shape.mask;
    }

    void from_json(const YJson& j, root_s &root){
        root.version    = j.at("version").get<std::string>();
        root.flags      = j.at("flags").get<std::string>();
        for(auto &shapes : j["shapes"]){
            shape_s s;
            from_json(shapes, s);
            root.shapes.push_back(s);
        }
        root.imagePath  = j.at("imagePath").get<std::string>();
        root.imageData  = j.at("imageData").get<std::string>();
        root.imageHeight= j.at("imageHeight").get<std::string>();
        root.imageWidth = j.at("imageWidth").get<std::string>();
    }
    void to_json(YJson& j, const root_s &root){
        j["version"] = root.version;
        j["flags"] = root.flags;
        auto obj_shape = YJson::array();
        for(const shape_s &var : root.shapes){
            YJson obj;
            to_json(obj, var);
            obj_shape.emplace_back(obj);
        }
        j["shapes"] = obj_shape;
        j["imagePath"] = root.imagePath;
        j["imageData"] = root.imageData;
        j["imageHeight"] = root.imageHeight;
        j["imageWidth"] = root.imageWidth;
    }
}

JsonManager::JsonManager(){}

bool JsonManager::readJson(const std::string &filename, shape_json::root_s& root){
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

bool JsonManager::writeJson(const std::string &filename, const shape_json::root_s& root){
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

#ifndef JSONMANAGER_HPP
#define JSONMANAGER_HPP

#include "fifo_map.hpp"
#include "json.hpp"
#include <fstream>
#include <QDebug>
#include <QVector>


namespace shape_json {
    struct shape_s{
        std::string label;
        QVector<QPointF> points;
        int group_id;
        std::string description;
        std::string shape_type;
        std::string flags;
        int mask;
    };
    struct root_s{
        std::string version;
        std::string flags;
        QVector<shape_s> shapes;
        std::string imagePath;
        std::string imageData;
        std::string imageHeight;
        std::string imageWidth;
    };
}

class JsonManager
{

public:
    JsonManager();
    static bool readJson(const std::string &filename, shape_json::root_s &root);
    static bool writeJson(const std::string &filename, const shape_json::root_s &root);
};

#endif // JSONMANAGER_HPP

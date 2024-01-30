#pragma once
#include <QByteArray>
#include <vector>
#include <list>
#include "../common.h"
#include <QTime>
namespace EnumType{
    enum match_type_e{
        none_match,
        lidar_camera_match,
        radar_camera_match,
        lidar_radar_match,
        lidar_radar_camera_match,
    };

    enum match_state_e{
        unmatch,
        match_successfully,
        match_during,
        match_failure,
    };
}


//camera
struct camera_frame_t{
    QByteArray buf;
    uint32_t len{0};
    double time{0.0f};
    int matchCtn{0};
    bool matchFlag{false};
    EnumType::match_state_e match_state{EnumType::unmatch};
    camera_frame_t& operator = (const camera_frame_t& frame){
        this->buf =  frame.buf;
        this->time = frame.time;
        return *this;
    }
};
struct camera_data_t{
    std::list<camera_frame_t> frame;
};
// struct camera_frame_t{
//     QByteArray buf;
//     uint32_t len{0};
//     double time{0.0f};
//     camera_frame_t& operator = (const camera_frame_t& frame){
//         this->buf =  frame.buf;
//         this->time = frame.time;
//         return *this;
//     }
// };
// struct camera_data_t{
//     std::list<camera_frame_t> frame;
//     int time{-1};
//     int matchCtn{0};
//     bool matchFlag{false};
//     EnumType::match_state_e match_state{EnumType::unmatch};
//     void clear(){
//         frame.clear();
//         time = -1;
//     }
// };


//radar
struct radar_frame_t{
    std::vector<StructRadarInfo> buf;
    double time{0.0f};
    int matchCtn{0};
    bool matchFlag{false};
    EnumType::match_state_e match_state{EnumType::unmatch};

    radar_frame_t& operator = (const radar_frame_t& frame){
        this->buf =  std::move(frame.buf);
        this->match_state = frame.match_state;
        this->matchCtn = frame.matchCtn;
        this->matchFlag = frame.matchFlag;
        this->time = frame.time;
        return *this;
    }
};
struct radar_data_t{
    std::list<radar_frame_t> frame;
};  

//lidar
struct lidar_frame_t{
    // pcl::PointCloud<PointXYZIT>::Ptr buf;
    std::shared_ptr<pcl::PointCloud<PointXYZIT>> buf;
    double time{0.0f};
    int matchCtn{0};
    bool matchFlag{false};
    EnumType::match_state_e match_state{EnumType::unmatch};
    lidar_frame_t& operator = (const lidar_frame_t& frame){
        this->buf =  frame.buf;
        this->match_state = frame.match_state;
        this->matchCtn = frame.matchCtn;
        this->matchFlag = frame.matchFlag;
        this->time = frame.time;
        return *this;
    }
};
struct lidar_data_t{
    std::list<lidar_frame_t> frame;
};

typedef lidar_data_t lidarDB;
typedef radar_data_t radarDB;
typedef camera_data_t cameraDB;

//match
struct match_data_t{
    // pcl::PointCloud<PointXYZIT>::Ptr lidarData;
    std::shared_ptr<pcl::PointCloud<PointXYZIT>> lidarData;
    std::vector<StructRadarInfo> radarData;
    cv::Mat cameraData;
    EnumType::match_type_e match_type{EnumType::none_match};
    bool isTakeOut{false};
};



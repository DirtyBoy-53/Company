#pragma once

#include "sensordata.hpp"
#include <QTime>
#include <QDebug>
#include "../utils/global.hpp"
#include "../utils/GQueue.h"
#include <QMutexLocker>
#include <QMutex>
class SensorMatch{

private:
    
    bool lidar_camera_match(lidar_frame_t& lidar_match,
                                camera_frame_t& camera_match);
    bool radar_camera_match(radar_frame_t& radar_match,
                                camera_frame_t& camera_match);

    bool lidar_radar_camera_match(  lidar_frame_t& lidar_match,
                                        radar_frame_t& radar_match,
                                        camera_frame_t& camera_match);

    bool first_match(   lidar_frame_t& lidar_match, 
                            radar_frame_t& radar_match);

    bool second_match(  const double& time,
                            const uint8_t& error_ms, 
                            camera_frame_t& camera_match);

    void discard_invalid_data();
    void match();

    void add_match_data(lidar_frame_t& lidar_match,
                            radar_frame_t& radar_match,
                            camera_frame_t& camera_match,
                            const EnumType::match_type_e& match_type);

    void QByteArrayToMat(QByteArray frame,const uint32_t& len,cv::Mat& mat);
private:
    bool mRunFlag{false};
    EnumType::match_type_e mMatchType;
    std::list<match_data_t> mMatchData;
    // GQueue<match_data_t> mMatchData;
    int mCameraChannel{0};

    radarDB mRadarData;
    lidarDB mLidarData;
    cameraDB mCameraData;

    QTime lidar_time_stamp;
    QTime radar_time_stamp;
    QTime camera_time_stamp;
    bool mStopRecvFlag{true};
    uint8_t* m_rgb{nullptr};
    QMutex camera_mtx;
    QMutex lidar_mtx;
    QMutex radar_mtx;
public:
    explicit SensorMatch();
    ~SensorMatch();
    bool checkSesorHavData();
    void setCameraChannel(const int& channel){
        mCameraChannel = channel;
    }
    const int& getCameraChannel(){
        return mCameraChannel;
    }

    void setMatchType(const EnumType::match_type_e& match_type){
        mMatchType = match_type;
    }
    const EnumType::match_type_e& getMatchType(){
        return mMatchType;
    }

    std::list<match_data_t>& getMatchData(){
        return mMatchData;
    }

    void addRadarData(const radar_frame_t& radar_data){
        qDebug()<<"recv addRadarData:" <<YDateTime::currentTime_ms();
        radar_time_stamp = QTime::currentTime();
        radar_mtx.lock();
        mRadarData.frame.emplace_back(radar_data);
        radar_mtx.unlock();
    }
    void addLidarData(const lidar_frame_t& lidar_data){
        // qDebug()<<"recv addLidarData:" <<YDateTime::currentTime_ms();
        lidar_time_stamp = QTime::currentTime();
        lidar_mtx.lock();
        mLidarData.frame.emplace_back(lidar_data);
        lidar_mtx.unlock();
    }
    // void addCameraData(const camera_data_t& camera_data){
    //     // qDebug()<<"recv addCameraData:" <<YDateTime::currentTime_ms() << "  Size:" << camera_data.frame.size();

    //     camera_time_stamp = QTime::currentTime();
    //     mCameraData.emplace_back(camera_data);
    // }

    void addCameraData(const camera_frame_t& camera_data){
        // qDebug()<<"recv addCameraData:" <<YDateTime::currentTime_ms() << "  Size:" << camera_data.frame.size();

        camera_time_stamp = QTime::currentTime();
        camera_mtx.lock();
        mCameraData.frame.emplace_back(camera_data);
        camera_mtx.unlock();
    }
    radarDB& getRadarData() {
        return mRadarData;
    }
    lidarDB& getLidarData() {
        return mLidarData;
    }
    cameraDB& getCameraData() {
        return mCameraData;
    }
    void cleanALLData(){
        mRadarData.frame.clear();
        mLidarData.frame.clear();
        mCameraData.frame.clear();
    }
    
    void setStopRecv(bool status){
        mStopRecvFlag = status;
        if(mStopRecvFlag == false){
            qDebug()<<"recv start:" <<YDateTime::currentTime_ms();
            lidar_time_stamp = QTime::currentTime();
            camera_time_stamp = QTime::currentTime();
            radar_time_stamp = QTime::currentTime();
        }
    }
    bool& getStopRecv(){
        return mStopRecvFlag;
    }

};
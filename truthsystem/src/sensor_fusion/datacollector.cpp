#include "datacollector.h"


void DataCollector::setSensorMatch(SensorMatch *p)
{
    mSensorMatch = p;
}

void DataCollector::add(const StructRadarDataPtr data)
{
    
    if(!mSensorMatch) return;
    if(mSensorMatch->getStopRecv()) return;
    if(mSensorMatch->getMatchType() == EnumType::lidar_camera_match || 
        mSensorMatch->getMatchType() == EnumType::none_match) {
        return;
    }
    radar_frame_t frame;
    frame.time = data.get()->_UnixTime.toDouble();
    for(auto i = 0;i < data.get()->_objNum;i++){
        frame.buf.emplace_back(data.get()->_RadarInfo[i]);
    }
    mSensorMatch->addRadarData(frame);
}  

void DataCollector::add(const PointView &data)
{
    
    if(!mSensorMatch) return;
    if(mSensorMatch->getStopRecv()) return;
    if(mSensorMatch->getMatchType() == EnumType::radar_camera_match || 
        mSensorMatch->getMatchType() == EnumType::none_match) {
        return;
    }
    qDebug() << "DataCollector::add(const PointView &data)<<<<<<<<<<<<<<<<<<<<<";
    lidar_frame_t frame;
    frame.buf = data.pointsPtr;
    frame.time = data.time;
    mSensorMatch->addLidarData(frame);
}  

#define TIMESTAMP_DECIMALPART   (1000000) //时间戳 微妙
#define TIMESTAMP_MSEC          (1000)      //时间戳 毫秒
#define DECIMAL(time)  ((time%TIMESTAMP_DECIMALPART)/TIMESTAMP_MSEC)
#define INTERVAL_TIME           (50)      //相机保存一个队列时间间隔是50ms
void DataCollector::add(const char *data,const uint32_t &size,const uint64_t &time,const uint32_t &channel)
{
    
    if(!mSensorMatch) return;
    if(mSensorMatch->getStopRecv()) return;//调试时打开
    if(mSensorMatch->getCameraChannel() != channel) return;
    if(mSensorMatch->getMatchType() == EnumType::lidar_radar_match || 
        mSensorMatch->getMatchType() == EnumType::none_match) {
        return;
    }
   
    // static camera_data_t dataSet;   //在同一个100ms内的数据

    // int iDecimal = DECIMAL(time);
    // if(dataSet.time == -1){
    //     dataSet.time = iDecimal;
    // }

    // if(dataSet.time != -1 && abs(iDecimal-dataSet.time) >= INTERVAL_TIME){
    //     mSensorMatch->addCameraData(dataSet);
    //     dataSet.clear();
    //     qDebug()<<"size:"<<size;
    // }


    camera_frame_t frame;
    frame.buf = QByteArray(data,size);
    frame.len = size;
    frame.time = (time*1.0)/TIMESTAMP_DECIMALPART;
    mSensorMatch->addCameraData(frame);
    // dataSet.frame.emplace_back(frame);
}    

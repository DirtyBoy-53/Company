#include "sensormatch.h"
#include <thread>
#include <QBuffer>
#include <QImageReader>
#include <QString>
#include <string>
#define INVALID_MATCH_MAX   10   //最大无效匹配次数
#define LIDAR_RADAR_CAMERA_DIF     20  //激光毫米波相机时间误差
#define ABNORMAL_TIME   500     //数据异常最大时间 200ms
void QByteArrayToMat(QByteArray array, cv::Mat& mat);
bool time_error_calculation(const double& t1,const double& t2,const uint8_t& error_ms);

SensorMatch::SensorMatch()
{
    mRunFlag = true;
    m_rgb = (unsigned char *)malloc(640*512*10);
    new std::thread(&SensorMatch::match,this);
}

SensorMatch::~SensorMatch()
{
    if(m_rgb) delete m_rgb;
    mRunFlag = false;
}

bool SensorMatch::checkSesorHavData()
{
    QTime time = QTime::currentTime();
    EnumType::match_type_e match_type = mMatchType;
    bool is_have_data = true;
    switch (match_type)
    {
    case EnumType::none_match: break;
    case EnumType::lidar_camera_match:
        if(abs(lidar_time_stamp.msecsTo(time)) > ABNORMAL_TIME ||
            abs(camera_time_stamp.msecsTo(time)) > ABNORMAL_TIME ) {
                is_have_data = false;
                qDebug() << "lidar_camera_match lidar_time_stamp:" << lidar_time_stamp.msecsTo(time) 
                        << "camera_time_stamp:" << camera_time_stamp.msecsTo(time);
            }
            
    break;
    case EnumType::radar_camera_match:
        if( abs(radar_time_stamp.msecsTo(time)) > ABNORMAL_TIME ||
            abs(camera_time_stamp.msecsTo(time)) > ABNORMAL_TIME ){
                is_have_data = false;
                qDebug() << "radar_camera_match radar_time_stamp:" << radar_time_stamp.msecsTo(time)
                         << "camera_time_stamp:" << camera_time_stamp.msecsTo(time);
            }
            
    break;
    case EnumType::lidar_radar_match:
      
    break;
    case EnumType::lidar_radar_camera_match:
        if( abs(lidar_time_stamp.msecsTo(time)) > ABNORMAL_TIME ||
            abs(radar_time_stamp.msecsTo(time)) > ABNORMAL_TIME ||
            abs(camera_time_stamp.msecsTo(time)) > ABNORMAL_TIME ){
                is_have_data = false;
                qDebug() << "lidar_radar_camera_match lidar_time_stamp:" << lidar_time_stamp.msecsTo(time)
                         << "radar_time_stamp:" << radar_time_stamp.msecsTo(time)
                         << "camera_time_stamp:" << camera_time_stamp.msecsTo(time);
            }
            
    break;
    default: break;
    }
    return is_have_data;
}

bool SensorMatch::lidar_camera_match(lidar_frame_t &lidar_match, camera_frame_t &camera_match)
{
    bool matchFlag{false};
    auto lidar_iter = mLidarData.frame.begin();
    auto lidar_size = mLidarData.frame.size();
    for (size_t lidarCnt = 0; lidarCnt < lidar_size; lidarCnt++) {
            auto camera_size = mCameraData.frame.size();
            auto camera_iter = mCameraData.frame.begin();
            for (size_t cameraCnt = 0; cameraCnt < camera_size; cameraCnt++) {
                if(time_error_calculation(lidar_iter->time,camera_iter->time,LIDAR_RADAR_CAMERA_DIF)){
                    lidar_iter->match_state = EnumType::match_successfully;
                    camera_iter->match_state = EnumType::match_successfully;
                    // qDebug() << "match success:lidar_time:" << QString::number(lidar_iter->time,'f',6) << 
                    // "camera_time:" << QString::number(camera_iter->time,'f',6);
                    lidar_match = *lidar_iter;
                    camera_match = *camera_iter;
                    matchFlag = true;
                }
                if(!lidar_iter->matchFlag){
                    lidar_iter->matchFlag = true;
                    lidar_iter->matchCtn++;
                    if(lidar_iter->matchCtn >= INVALID_MATCH_MAX){
                        lidar_iter->match_state = EnumType::match_failure;
                        qDebug() << "match failure:lidar_time:" << QString::number(lidar_iter->time,'f',6);
                    }
                    else lidar_iter->match_state = EnumType::match_during;
                }
                if(!camera_iter->matchFlag){
                    camera_iter->matchFlag = true;
                    camera_iter->matchCtn++;
                    if(camera_iter->matchCtn >= INVALID_MATCH_MAX){
                        camera_iter->match_state = EnumType::match_failure;
                        // qDebug() << "match failure:camera_time:" << QString::number(camera_iter->time,'f',6);
                    }
                    else camera_iter->match_state = EnumType::match_during;
                }
                if(matchFlag) break;
                camera_iter++;
            }
        if(matchFlag) break;
        lidar_iter++;
    }

    //恢复匹配标志
    lidar_iter = mLidarData.frame.begin();
    auto camera_iter = mCameraData.frame.begin();
    auto camera_size = mCameraData.frame.size();
    for (size_t lidarCnt = 0; lidarCnt < lidar_size; lidarCnt++) {
        lidar_iter->matchFlag = false;
        lidar_iter++;
    }
    for(size_t cameraCnt = 0; cameraCnt < camera_size; cameraCnt++) {
        camera_iter->matchFlag = false;
        camera_iter++;
    }

    return matchFlag;
}
bool SensorMatch::radar_camera_match(radar_frame_t &radar_match, camera_frame_t &camera_match)
{
    bool matchFlag{false};
    auto radar_iter = mRadarData.frame.begin();
    auto radar_size = mRadarData.frame.size();

    for (size_t radarCnt = 0; radarCnt < radar_size; radarCnt++) {
            auto camera_size = mCameraData.frame.size();
            auto camera_iter = mCameraData.frame.begin();
            for (size_t cameraCnt = 0; cameraCnt < camera_size; cameraCnt++) {
                if(time_error_calculation(radar_iter->time,camera_iter->time,LIDAR_RADAR_CAMERA_DIF)){
                    radar_iter->match_state = EnumType::match_successfully;
                    camera_iter->match_state = EnumType::match_successfully;
                    qDebug() << "match success:radar_time:" << QString::number(radar_iter->time,'f',6) << 
                    "camera_time:" << QString::number(camera_iter->time,'f',6);
                    radar_match = *radar_iter;
                    camera_match = *camera_iter;
                    matchFlag = true;
                }
                if(!radar_iter->matchFlag){
                    radar_iter->matchFlag = true;
                    radar_iter->matchCtn++;
                    if(radar_iter->matchCtn >= INVALID_MATCH_MAX){
                        radar_iter->match_state = EnumType::match_failure;
                         qDebug() << "match failure:radar_time:" << QString::number(radar_iter->time,'f',6);
                    }
                        
                    else radar_iter->match_state = EnumType::match_during;
                }
                if(!camera_iter->matchFlag){
                    camera_iter->matchFlag = true;
                    camera_iter->matchCtn++;
                    if(camera_iter->matchCtn >= INVALID_MATCH_MAX){
                        camera_iter->match_state = EnumType::match_failure;
                        qDebug() << "match failure:camera_time:" << QString::number(camera_iter->time,'f',6);
                    }
                        
                    else camera_iter->match_state = EnumType::match_during;
                }
                if(matchFlag) break;
                camera_iter++;
            }
        if(matchFlag) break;
        radar_iter++;
    }

    //恢复匹配标志
    radar_iter = mRadarData.frame.begin();
    auto camera_iter = mCameraData.frame.begin();
    auto camera_size = mCameraData.frame.size();
    for (size_t radarCnt = 0; radarCnt < radar_size; radarCnt++) {
        radar_iter->matchFlag = false;
        radar_iter++;
    }
    for(size_t cameraCnt = 0; cameraCnt < camera_size; cameraCnt++) {
        camera_iter->matchFlag = false;
        camera_iter++;
    }
    return matchFlag;
}


#define AVG_TIME(t1,t2) ((t1+t2)/2)
bool SensorMatch::lidar_radar_camera_match(lidar_frame_t &lidar_match,
                                           radar_frame_t &radar_match,
                                           camera_frame_t &camera_match)
{
    bool ret = first_match(lidar_match,radar_match);
    if(!ret) return ret;
    double avg_time = AVG_TIME(lidar_match.time,radar_match.time);
    ret = second_match(avg_time,LIDAR_RADAR_CAMERA_DIF,camera_match);
    return ret;
}

#define SEC2MILLISEC(sec)   ((sec*1.0)/1000)    
bool time_error_calculation(const double& t1,const double& t2,const uint8_t& error_ms){
    bool ret{false};
    double difValue = t1-t2;
    return abs(difValue) <= SEC2MILLISEC(error_ms);
}

#define LIDAR_RADAR_DIF     50  //激光毫米波时间误差
//lidar and radar match.
bool SensorMatch::first_match(lidar_frame_t& lidar_match, radar_frame_t& radar_match)
{

    auto lidar_iter = mLidarData.frame.begin();
    auto lidar_size = mLidarData.frame.size();
    
    bool matchFlag{false};
    for (size_t lidarCnt = 0; lidarCnt < lidar_size; lidarCnt++) {
        auto radar_iter = mRadarData.frame.begin();
        auto radar_size = mRadarData.frame.size();
        for(size_t radarCnt = 0; radarCnt < radar_size; radarCnt++){
            if(time_error_calculation(lidar_iter->time,radar_iter->time,LIDAR_RADAR_DIF)){
                lidar_iter->match_state = EnumType::match_successfully;
                radar_iter->match_state = EnumType::match_successfully;
                qDebug() << "match success:lidar_time:" << QString::number(lidar_iter->time,'f',6) << 
                    "radar_time:" << QString::number(radar_iter->time,'f',6);
                lidar_match = *lidar_iter;
                radar_match = *radar_iter;
                matchFlag = true;
                break;
            }
            if(!lidar_iter->matchFlag){
                lidar_iter->matchFlag = true;
                lidar_iter->matchCtn++;
                if(lidar_iter->matchCtn >= INVALID_MATCH_MAX)
                    lidar_iter->match_state = EnumType::match_failure;
                else lidar_iter->match_state = EnumType::match_during;
            }
            if(!radar_iter->matchFlag){
                radar_iter->matchFlag = true;
                radar_iter->matchCtn++;
                if(radar_iter->matchCtn >= INVALID_MATCH_MAX)
                    radar_iter->match_state = EnumType::match_failure;
                else radar_iter->match_state = EnumType::match_during;
            }
        }
        if(matchFlag) break;
        lidar_iter++;
    }

    //恢复匹配标志
    lidar_iter = mLidarData.frame.begin();
    auto radar_iter = mRadarData.frame.begin();
    auto radar_size = mRadarData.frame.size();
    for (size_t lidarCnt = 0; lidarCnt < lidar_size; lidarCnt++) {
        lidar_iter->matchFlag = false;
    }
    for(size_t radarCnt = 0; radarCnt < radar_size; radarCnt++) { 
        radar_iter->matchFlag = false;
    }

    return matchFlag;
}

//first_match value and camera match.
bool SensorMatch::second_match(const double& time,const uint8_t& error_ms, camera_frame_t& camera_match)
{
    auto camera_size = mCameraData.frame.size();
    auto camera_iter = mCameraData.frame.begin();
    bool matchFlag{false};
    for (size_t cameraCnt = 0; cameraCnt < camera_size; cameraCnt++) {     
        if(time_error_calculation(camera_iter->time,time,error_ms)){
            camera_iter->match_state = EnumType::match_successfully;
            camera_match = *camera_iter;
            matchFlag = true;
            if(matchFlag) break;
        }
        
        
        camera_iter->matchCtn++;
        if(camera_iter->matchCtn >= INVALID_MATCH_MAX){
            camera_iter->match_state = EnumType::match_failure;
        }else{
            camera_iter->match_state = EnumType::match_during;
        }
        camera_iter++;
    }
    return matchFlag;
}

void SensorMatch::discard_invalid_data()
{
    //lidar
    auto lidar_size = mLidarData.frame.size();
    auto lidar_iter = mLidarData.frame.begin();
    lidar_mtx.lock();
    for(auto i = 0;i < lidar_size;i++){
        if(lidar_iter->match_state == EnumType::match_failure || 
            lidar_iter->match_state == EnumType::match_successfully){
            // qDebug() << "remove lidar:" << QString::number(lidar_iter->time,'f',6);
            mLidarData.frame.erase(lidar_iter);
            break;//一次只删除一帧数据
        }
        lidar_iter++;
    }
    lidar_mtx.unlock();
    //radar
    auto radar_size = mRadarData.frame.size();
    auto radar_iter = mRadarData.frame.begin();
    radar_mtx.lock();
    for(auto i = 0;i < radar_size;i++){
        if(radar_iter->match_state == EnumType::match_failure ||
            radar_iter->match_state == EnumType::match_successfully){
            // qDebug() << "remove radar:" << QString::number(radar_iter->time,'f',6);
            mRadarData.frame.erase(radar_iter);
            break;//一次只删除一帧数据
        }
        radar_iter++;
    }
    radar_mtx.unlock();

    //camera
    auto camera_size = mCameraData.frame.size();
    auto camera_iter = mCameraData.frame.begin();
    camera_mtx.lock();
    for(auto i = 0;i < camera_size;i++){
        if(camera_iter->match_state == EnumType::match_failure ||
            camera_iter->match_state == EnumType::match_successfully){
            // qDebug()  << "  remove camera_time" << QString::number(camera_iter->time,'f',6)
            //     << "  remove_befor_size:" << mCameraData.frame.size();
            mCameraData.frame.erase(camera_iter);
            // qDebug() << "  remove_after_size" << mCameraData.frame.size();
            break;//一次只删除一帧数据
        }
        camera_iter++;
    }
    camera_mtx.unlock();


}

void SensorMatch::match()
{

    while(mRunFlag){
        bool ret{false};
        lidar_frame_t  lidar_match;
        camera_frame_t camera_match;
        radar_frame_t  radar_match;
        EnumType::match_type_e match_type = mMatchType;
        switch (match_type)
        {
        case EnumType::none_match: break;
        case EnumType::lidar_camera_match:
            if( mLidarData.frame.size()  < 1 || 
                mCameraData.frame.size() < 2 )
                break;
            ret = lidar_camera_match(lidar_match,camera_match);
            if(!ret) break;
            add_match_data(lidar_match,radar_match,camera_match,match_type);

        break;
        case EnumType::radar_camera_match:
            if( mRadarData.frame.size()  < 1 || 
                mCameraData.frame.size() < 2 )
                break;
            ret = radar_camera_match(radar_match,camera_match);
            if(!ret) break;
            add_match_data(lidar_match,radar_match,camera_match,match_type);

        break;
        case EnumType::lidar_radar_match:
            if( mLidarData.frame.size()  < 1 || 
                mRadarData.frame.size()  < 1 )
                break;
            
        break;
        case EnumType::lidar_radar_camera_match:
            if( mLidarData.frame.size()  < 1 || 
                mRadarData.frame.size()  < 1 || 
                mCameraData.frame.size() < 2 )
                break;
            ret = lidar_radar_camera_match(lidar_match,radar_match,camera_match);
            if(!ret) break;
            add_match_data(lidar_match,radar_match,camera_match,match_type);
        break;
        default: break;
        }
        discard_invalid_data();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


void SensorMatch::QByteArrayToMat(QByteArray frame,const uint32_t& len,cv::Mat& mat){

    // delete data;
    FrameInfo_S frameInfo;
    memcpy(&frameInfo, frame.data(), sizeof(FrameInfo_S));
    uint8_t * yuvFrame = (uint8_t*)frame.data() + 128 + frameInfo.uNvsLen;;

    uint32_t width = frameInfo.width;
    uint32_t height = frameInfo.height;

    try {
        cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
        cv::Mat rgbImg(height, width, CV_8UC3, m_rgb);
        cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_YUYV);

        mat = rgbImg.clone();
    }
    catch(cv::Exception& e) {
        std::cerr << e.what() << "\n";
    }

    

    // static bool flag{false};
    // if(!flag){
    //     flag = true;
    //     cv::imwrite("./camera.jpg",mat);
    // }

}

void QByteArrayToMat(QByteArray array, cv::Mat& mat)
{
    // qDebug() << "QByteArrayToMat" << array.size();
    // QBuffer buffer(&array);
    // buffer.open(QIODevice::ReadOnly);
    // QImageReader reader(&buffer, "JPG");
    // QImage image = reader.read();

    // switch (image.format())
    // {
    // case QImage::Format_ARGB32:
    // case QImage::Format_RGB32:
    // case QImage::Format_ARGB32_Premultiplied:
    // {
    //     mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
    //     std::vector<cv::Mat>channels;
    //     split(mat, channels);
    //     channels.pop_back();
    //     cv::merge(channels, mat);
    // }
    // break;
    // case QImage::Format_RGB888:
    //     mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
    //     //cv::cvtColor(mat, mat, CV_BGR2RGB);
    //     break;
    // case QImage::Format_Indexed8:
    //     mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
    //     break;
        
    // }
}

void SensorMatch::add_match_data(lidar_frame_t &lidar_match, radar_frame_t &radar_match, camera_frame_t &camera_match,
                                    const EnumType::match_type_e& match_type)
{
    match_data_t match_data;
    match_data.isTakeOut = false;
    match_data.match_type = match_type;
    switch (match_type)
    {
        case EnumType::none_match: break;
        case EnumType::lidar_camera_match:
            match_data.lidarData = lidar_match.buf;
            QByteArrayToMat(camera_match.buf,camera_match.len,match_data.cameraData);
            mMatchData.emplace_back(match_data);
        break;
        case EnumType::radar_camera_match:
            match_data.radarData = std::move(radar_match.buf);
            QByteArrayToMat(camera_match.buf,camera_match.len,match_data.cameraData);
            mMatchData.emplace_back(match_data);

        break;
        case EnumType::lidar_radar_match:
            match_data.lidarData = lidar_match.buf;
            match_data.radarData = std::move(radar_match.buf);
            mMatchData.emplace_back(match_data);
        break;
        case EnumType::lidar_radar_camera_match:
            match_data.lidarData = lidar_match.buf;
            match_data.radarData = std::move(radar_match.buf);
            QByteArrayToMat(camera_match.buf,camera_match.len,match_data.cameraData);
            mMatchData.emplace_back(match_data);
        break;
        default: break;
    }

}

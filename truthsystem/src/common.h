#ifndef _COMMON_H_
#define _COMMON_H_

#include "log.h"
//opencv
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>
//pcl
#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>
#include <pcl/octree/octree.h>
#include <boost/thread/thread.hpp>
#include <pcl/common/transforms.h>
#include <pcl/search/kdtree.h>
#include <pcl/common/time.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>
// using namespace pcl;

//QGLViewer
#if 1
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#endif

#include <QObject>

//else
#include "define.h"
#include <qglobal.h>
#include "log.h"
#include <memory>
#include "pandarSwiftSDK.h"
#include "videoinfo.h"

struct st_point {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    st_point() {}

    st_point(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
};

struct PointInfo
{
    double X;
    double Y;
    double Z;
    float Intensity;
    int ring;
    double time;

    PointInfo(): X(0), Y(0), Z(0), Intensity(0), ring(0), time(0){}
};

struct StructPointData{
    PointInfo point[AT128_DATASIZE_MAX];
    // PPointCloud point;
    uint64_t pointsize;
    std::string time;
};
// typedef std::shared_ptr<StructPointData> StructPointDataPtr;
typedef std::shared_ptr<uchar> uchar_ptr;

struct PointView
{
    QString frameTime;
    double time;
    // pcl::PointCloud<PointXYZIT>::Ptr pointsPtr;
    std::shared_ptr<pcl::PointCloud<PointXYZIT>> pointsPtr;
    // PointView() {
    //     pointsPtr = std::make_shared<pcl::PointCloud<PointXYZIT> >();
    // }
};

enum class ENUM_VIEW{
    Enum_front,
    Enum_right,
    Enum_top,
};
/*
// 工装盒
// 视频默认  帧头   + 算法数据 + YUV视频 + Y16视频 + 参数行  + 可见光视频
//         128帧头 + nvsLen + yuvLen + y16Len + paramLen + vlLen
struct FrameInfo_S{
    uint64_t uHeadFlag;//0xefefefef80808080
    int32_t uTotalLen;
    int64_t uTimeTm;
    int16_t uNvsLen;
    int32_t uY16Len;
    int32_t uYuvLen;
    int32_t uParamLen;
    int8_t uFormat; //1 yuv422 3 yuv16 5 y16+yuv 6 yuv420  7 x16  0xffff 原始数据
    int8_t uScale;//1 正常  2 2倍 4 4倍
    int8_t vlFormat;//1 yuv422p y+u+v 2 yuv420 3 h264
    int32_t vlLen;
    int16_t vlWidth = 1920;
    int16_t vlHeight = 1280;
    int16_t width;
    int16_t height;
    uint8_t ext[40];//环境CollectInfo:ext[0]:fps, 温度，湿度，季节，城市，天气，目标类型，场景
    bool isValid() {
        return uHeadFlag == 0x80808080efefefef ? true:false;
    }
    FrameInfo_S() {
        uHeadFlag = 0x80808080efefefef;
        uTotalLen = 0;
        uTimeTm = 0;
        uNvsLen = 0;
        uY16Len = 0;
        uYuvLen = 0;
        uParamLen = 0;
        uFormat = 0;
        uScale = 1;
        vlFormat = 0;
        vlLen = 0;
        vlWidth = 0;
        vlHeight = 0;
        width = 0;
        height = 0;
        memset (ext, 0, 40);
    }
};

struct FileInfo_S {
    uint64_t uHeadFlag;//0xefefefef80808080
    uint8_t uWeather;
    uint8_t uDayOrNight;
    uint8_t uTemprature;
    uint8_t uHuminity;
    uint8_t uProject[8];
    uint8_t uSence[16];
};
// struct FrameHead{
//     qint64 tm; // 时间戳
//     float temp;  // 温度
//     float wet;   // 湿度
//     int season;  // 季节
//     int city;    // 城市
//     int weather; // 天气
//     int type;    // 目标类型
//     int scene;   // 场景
//     uint32_t uFlag = 0x76752e2e; //特殊格式 .uv 0x76752e2e
//     int iUvMode = 1;//uv录制 0 默认  1 uv
//     int nvsLen = 0;  //nvs长度
//     int format1=1; //帧格式 VIDEO_DATA_FORMAT
//     int paramLen = 0; //参数行长度
//     uint64_t idx;
//     int format;
//     int width;
//     int height;
//     int frameSize;
//     int fps;
// };
*/
struct FrameInfo{
    int count = 0;
    uint64_t tm = 0;
    int fps = 0;
};

struct _xtime {
    int year; int month;  int day;
    int hour; int minute;  int second; int msecond;

    _xtime() {}

    _xtime(int y, int m, int d, int h, int mi, int s, int ms)
    {
        year = y;
        month = m;
        day = d;
        hour = h;
        minute = mi;
        second = s;
        msecond = ms;
    }
};


struct StructCameraData
{
	int id; //camera id
	int frameWidth;
	int frameHeight;
    QDateTime frameTime;

	u_int8_t frameBuf[20 * 1024 * 1024]{0};
	int64 frameBufSize{0};
    uint64_t timestamp{0};
};
typedef std::shared_ptr<StructCameraData> StructCameraDataPtr;

struct CameraParam
{
    int id;
    QString cmd_path;
    QString cmd_Channel;
    QString cmd_stream;

    int frameWidth;
    int frameHeight;
    int fps;
};


class GlobalSignals : public QObject{
	Q_OBJECT
public:
	GlobalSignals(QObject* parent = nullptr) : QObject(parent) {
    	qRegisterMetaType<PointView>("PointView");
	}

	static GlobalSignals* getInstance() {
		static GlobalSignals gs;
		return &gs;
	}
signals:
	void signal_one_frame_proc(PointView data);
	void signal_one_frame_light(int slot, unsigned char* data, int len);
};

struct StructRadarInfo{
    int _id;
    double _x;
    double _y;
    double _xVel;
    double _yVel;
};

struct StructRadarData{
    uint32_t _RecvTime;
    QString _UnixTime;
    int _objNum;
    int _idx;
    int _devID;
    StructRadarInfo _RadarInfo[RADAR_ARS408_OBJNUM_MAX];
};

typedef std::shared_ptr<StructRadarData> StructRadarDataPtr;


enum class EnumCanDevType{
    Enum_USBCAN_II=4,
    Enum_PCI9820=5,
    Enum_PCI9840=14,
    Enum_PCI9820i=16,
    Enum_USBCANFD=33,
};
enum class EnumCanDevIdx{
    Enum_Idx1=0,
    Enum_Idx2=1,
    Enum_Idx3=2,
    Enum_Idx4=3,
};

enum class EnumFormat{
    Enum_Yuv444,
    Enum_Yuv422,
    Enum_Yuv420
};
#endif

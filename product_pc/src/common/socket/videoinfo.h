#ifndef VIDEOINFO_H
#define VIDEOINFO_H

#include <QObject>
#include <comdll.h>
#include <QVector>

enum VIDEO_DATA_FORMAT {
    VIDEO_FORMAT_NV21 = 0,
    VIDEO_FORMAT_UYUV,
    VIDEO_FORMAT_Y16,
    VIDEO_FORMAT_X16,
    VIDEO_FORMAT_YUV420,
    VIDEO_FORMAT_Y16_Y8,
};

enum VIDEO_SENSOR_COMS {
    VIDEO_SENSOR_COMS_YUV = 0,
    VIDEO_SENSOR_COMS_RAW,
    VIDEO_SENSOR_COMS_YUV420,
};

struct VideoFormatInfo {
    int sensorFormat;
    int format;
    int width;
    int height;
    int nvsLen;
    int frameLen;
    int paramLen;
    VideoFormatInfo() {
        format = VIDEO_FORMAT_NV21;
        width = 640;
        height = 512;
        nvsLen = width*4;
        frameLen = width*height*3/2;
        paramLen = width*4*2;
    }
};

class Q_API_FUNCTION VideoInfo : public QObject
{
    Q_OBJECT
public:
    VideoInfo();
    ~VideoInfo();
    static VideoInfo* getInstance();

    void getVideoInfo(const int handle, VideoFormatInfo& info);
    void setVideoInfo(const int handle, const VideoFormatInfo info);

private:
    QVector<VideoFormatInfo> m_info;
};

#endif // VIDEOINFO_H

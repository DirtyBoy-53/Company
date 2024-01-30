#ifndef VIDEOINFO_H
#define VIDEOINFO_H

#include <QObject>
#include <QMap>
#include <singleton.h>
#include "if/h264decode.h"

#define HEADSIZE 128
#define FILEHEADSIZE 2*640

enum VIDEO_DATA_FORMAT {
    VIDEO_FORMAT_INVALID = -1,
    VIDEO_FORMAT_NV21 = 0,
    VIDEO_FORMAT_UYVY = 1,
    VIDEO_FORMAT_YUYV = 2,
    VIDEO_FORMAT_Y16 = 3,
    VIDEO_FORMAT_NV12 = 4,
    VIDEO_FORMAT_Y16_UYVY = 5,
    VIDEO_FORMAT_YUV420 = 6,
    VIDEO_FORMAT_X16 = 7,
    VIDEO_FORMAT_YVYU = 8,
    VIDEO_FORMAT_H264 = 0xa0,
    VIDEO_FORMAT_Y = 0xa1,
    VIDEO_FORMAT_YUV422P = 0xa2,
    VIDEO_FORMAT_SRC = 0xffff,
};

// 视频默认  帧头   + 算法数据 + YUV视频 + Y16视频 + 参数行  + 可见光视频
//         128帧头 + nvsLen + yuvLen + y16Len + paramLen + vlLen
struct FrameInfo_S{
    uint64_t uHeadFlag{0x80808080efefefef};//0xefefefef80808080
    int32_t uTotalLen{0};
    int64_t uTimeTm{0};
    int16_t uNvsLen{0};
    int32_t uY16Len{0};
    int32_t uYuvLen{0};
    int32_t uParamLen{0};
    uint8_t uFormat{0}; //1 yuv422 3 yuv16 5 y16+yuv 6 yuv420  7 x16  0xffff 原始数据
    int8_t uScale{0};//1 正常  2 2倍 4 4倍
    int8_t vlFormat{0};//1 yuv422p y+u+v 2 yuv420 3 h264
    int32_t vlLen{0};
    int16_t vlWidth{0};
    int16_t vlHeight{0};
    int16_t width{0};
    int16_t height{0};
    int32_t imuLen{0};
    uint8_t ext[40]{0};//环境CollectInfo
    int endian() {
        if(uHeadFlag == 0xefefefef80808080) {
            return 0;
        } else if(uHeadFlag == 0x80808080efefefef) {
            return 1;
        } else {
            return -1;
        }
    }
    FrameInfo_S(){
        uHeadFlag = 0x80808080efefefef;
        uTotalLen = 0;uTimeTm = 0;uNvsLen = 0;uY16Len = 0;
        uYuvLen = 0;uParamLen = 0;uFormat = 0;uScale = 0;vlFormat = 0;vlLen = 0;vlWidth = 0;
        vlHeight = 0;width = 0;height = 0;imuLen = 0;memset(ext,0,sizeof(uint8_t)*40);
    }
};

struct VideoData_S {
    int fileHeadLen; //文件头长度
    int format;      //视频格式
    qint64 totalLen;    //帧总长度
    int headLen;     //帧头长度
    int nvsLen;      //算法长度
    int irFrameLen;  //红外帧数据长度
    int paramLen;    //参数行长度
    int irWidth;       //视频宽
    int irHeight;      //视频高
    int vlFrameLen = 0;
    int vlWidth = 0;
    int vlHeight = 0;
    int iAuto = 0;//是否自定义
};

const QMap<int, QString> g_mapStr = {
    {VIDEO_FORMAT_NV21, "NV21"},
    {VIDEO_FORMAT_UYVY, "UYUV"},
    {VIDEO_FORMAT_YUYV, "YUYV"},
    {VIDEO_FORMAT_Y16, "Y16"},
    {VIDEO_FORMAT_NV12, "NV12"},
    {VIDEO_FORMAT_Y16_UYVY, "Y16+UYVY"},
    {VIDEO_FORMAT_YUV420,"YUV420P"},
    {VIDEO_FORMAT_X16, "X16"},
    {VIDEO_FORMAT_YVYU, "YVYU"},
    {VIDEO_FORMAT_H264, "h264"},
    {VIDEO_FORMAT_Y, "Y数据"},
    {VIDEO_FORMAT_YUV422P, "YUV422P"},
};

class VideoFormatParser : public Singleton<VideoFormatParser>
{
public:
    VideoFormatParser();

    QByteArray srcFrameToRgb(uint8_t * pSrcData);

    QByteArray srcToRgb(H264Decode* decoder, int format, int w, int h, uint8_t *pSrcData, int len);
    QByteArray vlSrcToRgb(H264Decode* decoder, int format, int w, int h, uint8_t* srcData, int len);
    QByteArray vlSrcToYuv(H264Decode* decoder, int format, int w, int h, uint8_t* srcData, int len);

    bool srxToFrameData(QByteArray srcArray, VideoData_S & frameData);
    bool hasHeader(QByteArray data, VideoData_S & frameData);
private:
    uint8_t* m_vlYuv = nullptr;
};

#endif // VIDEOINFO_H

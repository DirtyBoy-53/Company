#ifndef YDEFINE_H
#define YDEFINE_H

typedef enum {
    MEDIA_TYPE_FILE = 0,
    MEDIA_TYPE_NETWORK,
    MEDIA_TYPE_PCIE,
    MEDIA_TYPE_NB
} media_type_e;

#define DEFAULT_MEDIA_TYPE  MEDIA_TYPE_PCIE
#define MEDIA_TYPE_NONE     MEDIA_TYPE_NB



//Frame
#define FRAME_HEADSIZE_MAX          128         //帧头大小
#define VIDEO_HEADSIZE_MAX          128         //视频头大小

//Video
#define VIDEO_FRAME_MAX_W       1920
#define VIDEO_FRAME_MAX_H       1080
#define VIDEO_FRAME_MAX_SIZE    (VIDEO_FRAME_MAX_W*VIDEO_FRAME_MAX_H*4)
#define VIDEO_WINDOW_MAX        16

#define DEFAULT_RETRY_INTERVAL  10000  // ms
#define DEFAULT_RETRY_MAXCNT    6

#define DEFAULT_SUFFIX          "bin"
#define DEFAULT_PREFIX          "Video"


#define VIDEO_RECORD_INTERVAL   (1000*60*1)     //1min

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

//37-红外 31-可见光(艾利光) 38-红外（1280-XP04）
enum PCIE_CAMERA_TYPE{
    PCIE_CAMERA_LIGHT  = 31,
    PCIE_CAMERA_IR     = 37,
    PCIE_CAMERA_IR_XP04= 38
};

enum MSG_TYPE{
    MSG_NORMAL,
    MSG_WARNING,
    MSG_SUCCESS,
};


typedef enum {
    PIX_FMT_NONE = 0,

    PIX_FMT_GRAY,       // YYYYYYYY

    PIX_FMT_YUV_FIRST = 100,
    PIX_FMT_YUV_PLANAR_FIRST = 200,
    PIX_FMT_IYUV,       // YYYYYYYYUUVV
    PIX_FMT_YV12,       // YYYYVVYYVVUU
    PIX_FMT_NV12,       // YYUVYYYYUVUV
    PIX_FMT_NV21,       // YYVUYYYYVUVU
    PIX_FMT_YUV_PLANAR_LAST,
    PIX_FMT_YUV_PACKED_FIRST = 300,
    PIX_FMT_YUY2,       // YUYVYUYV
    PIX_FMT_YVYU,       // YVYUYVYU
    PIX_FMT_UYVY,       // UYVYUYVY
    PIX_FMT_YUV_PACKED_LAST,
    PIX_FMT_YUV_LAST,

    PIX_FMT_RGB_FIRST = 400,
    PIX_FMT_RGB,        // RGBRGB
    PIX_FMT_BGR,        // BGRBGR
    PIX_FMT_RGBA,       // RGBARGBA
    PIX_FMT_BGRA,       // BGRABGRA
    PIX_FMT_ARGB,       // ARGBARGB
    PIX_FMT_ABGR,       // ABGRABGR
    PIX_FMT_RGB_LAST,
} pix_fmt_e;



typedef enum {
    AVSTREAM_TYPE_VIDEO,
    AVSTREAM_TYPE_AUDIO,
    AVSTREAM_TYPE_SUBTITLE,
    AVSTREAM_TYPE_NB
} avstream_type_e;

typedef enum {
    ASPECT_FULL,            // 100%
    ASPECT_PERCENT,         // 50%
    ASPECT_ORIGINAL_RATIO,  // w:h
    ASPECT_ORIGINAL_SIZE,   // wxh
    ASPECT_CUSTOM_RATIO,    // 4:3 16:9 ...
    ASPECT_CUSTOM_SIZE,     // 1280x720 640*480 ...
} aspect_ratio_e;
#define DEFAULT_ASPECT_RATIO    ASPECT_FULL

typedef enum{
    PRINT_RED = 31,
    PRINT_GREEN = 32,
    PRINT_GRAY = 90,
} print_color_e;
#endif // YDEFINE_H

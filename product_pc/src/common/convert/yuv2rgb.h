#ifndef YUV2RGB_H
#define YUV2RGB_H
#include <comdll.h>

class Q_API_FUNCTION Yuv2Rgb
{
public:
    Yuv2Rgb();
    static bool YuvToRgbNV21(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbNV12(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbUYVY(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool ResizeRgb(unsigned char* srcData, unsigned char* dstData, double value, int width, int height);
    static bool RgbaToYuvNV12(unsigned char* rgbFrame, unsigned char* yuvFrame, int width, int height);
    static bool YuvToRgbYUYV(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgb420(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool Y8ToRgb(unsigned char* y8, unsigned char* rgb, int width, int height);
};




#endif // YUV2RGB_H

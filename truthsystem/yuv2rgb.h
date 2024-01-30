#ifndef YUV2RGB_H
#define YUV2RGB_H

#define CLIPVALUE(x, minValue, maxValue) ((x) < (minValue) ? (minValue) : ((x) > (maxValue) ? (maxValue) : (x)))
#define YUVToR(Y, U, V) ( (Y) + 1.4075 * ((V) - 128) )
#define YUVToG(Y, U, V) ( (Y) - 0.3455 * ((U) - 128) - 0.7169 * ((V) - 128) )
#define YUVToB(Y, U, V) ( (Y) + 1.779 * ((U) - 128) )

#define RGBToY(R, G, B) ( 0.299 * R + 0.587 * G + 0.114 * B )
#define RGBToU(R, G, B) ( -0.147 * R - 0.289 * G + 0.436 * B )
#define RGBToV(R, G, B) ( 0.615R - 0.515G -0.100B )

class Yuv2Rgb
{
public:
    Yuv2Rgb();
    static bool YsrcToRgb(unsigned char* yFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgb420p(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbNV21(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbNV12(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbUYVY(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToBgrUYVY(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbYUYV(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgbYVYU(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);
    static bool YuvToRgb420(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height);  
    static bool YuvToRgb422p(unsigned char *yuvFrame, unsigned char *rgbFrame, int width, int height);

    static bool ResizeRgb(unsigned char* srcData, unsigned char* dstData, double value, int width, int height);
    static bool RgbaToYuvNV12(unsigned char* rgbFrame, unsigned char* yuvFrame, int width, int height);
    static bool RgbToYuv420(unsigned char* rgbFrame, unsigned char *yuvFrame, int width, int height);
    static bool RgbToUYUV(unsigned char *rgbFrame, unsigned char *yuvFrame, int width, int height);
    static bool ResizeUYVY(unsigned char* yuvFrame, unsigned char* dstData, double value, int width, int height);
};




#endif // YUV2RGB_H

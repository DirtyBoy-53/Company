#include "yuv2rgb.h"
#include <stdio.h>

#include "opencv2/opencv.hpp"
using namespace cv;

Yuv2Rgb::Yuv2Rgb()
{

}

bool Yuv2Rgb::YsrcToRgb(unsigned char *yFrame, unsigned char *rgbFrame, int width, int height)
{
    if(yFrame == nullptr) {
        return false;
    }
    for(int i=0; i<width*height; i++) {
        rgbFrame[3*i] = yFrame[i];
        rgbFrame[3*i+1] = yFrame[i];
        rgbFrame[3*i+2] = yFrame[i];
    }
    return true;
}

bool Yuv2Rgb::YuvToRgb420p(unsigned char *yuvFrame, unsigned char *rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height*3/2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV420p2BGR);
    return true;
}

bool Yuv2Rgb::YuvToRgbNV21(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height*3/2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_NV21);
    return true;
}

bool Yuv2Rgb::YuvToRgbNV12(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height*3/2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_NV12);
    return true;
}

bool Yuv2Rgb::YuvToRgbUYVY(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_UYVY);
    return true;
}

bool Yuv2Rgb::YuvToBgrUYVY(unsigned char *yuvFrame, unsigned char *rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_UYVY);
    return true;
}

bool Yuv2Rgb::YuvToRgb420(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height * 3 / 2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_I420);
    return true;
}

bool Yuv2Rgb::RgbToYuv420(unsigned char *rgbFrame, unsigned char *yuvFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height * 3 / 2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(rgbImg, yuvImg, CV_RGB2YUV_I420);
    return true;
}

bool Yuv2Rgb::RgbToUYUV(unsigned char *rgbFrame, unsigned char *yuvFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(rgbImg, yuvImg, CV_YUV2RGB_UYVY);
    return true;
}

bool Yuv2Rgb::YuvToRgbYUYV(unsigned char* yuvFrame, unsigned char* rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_YUYV);
    return true;
}

bool Yuv2Rgb::YuvToRgbYVYU(unsigned char *yuvFrame, unsigned char *rgbFrame, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }
    cv::Mat yuvImg(height, width, CV_8UC2, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC3, rgbFrame);
    cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_YVYU);
    return true;
}

bool Yuv2Rgb::ResizeRgb(unsigned char* srcData, unsigned char* dstData, double value, int width, int height)
{
    if(srcData == nullptr) {
        return false;
    }

    cv::Mat srcRgb(height, width, CV_8UC3, srcData);
    cv::Mat rgbImg(height*value, width*value, CV_8UC3, dstData);
    cv::resize(srcRgb, rgbImg, cv::Size(srcRgb.cols*value, srcRgb.rows*value), 0, 0, INTER_NEAREST);
    return true;
}

bool Yuv2Rgb::ResizeUYVY(unsigned char* yuvFrame, unsigned char* dstData, double value, int width, int height)
{
    if(yuvFrame == nullptr) {
        return false;
    }

    cv::Mat srcRgb(height, width, CV_8UC2, yuvFrame);
    int tmpW = width / value;
    int tmpH = height / value;
    cv::Mat img = srcRgb(cv::Rect((width-tmpW)/2, (height-tmpH)/2, tmpW, tmpH));
    cv::Mat dstImg(height, width, CV_8UC2, dstData);
    cv::resize(img, dstImg, cv::Size(width, height), 0, 0, INTER_NEAREST);
    return true;
}


bool Yuv2Rgb::RgbaToYuvNV12(unsigned char* rgbFrame, unsigned char* yuvFrame, int width, int height)
{
    cv::Mat yuv;
    cv::Mat yuvImg(height*3/2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC4, rgbFrame);
    cv::cvtColor(rgbImg, yuv, cv::COLOR_RGB2YUV_I420);
    cv::cvtColor(yuv, yuvImg, cv::COLOR_YUV2RGB_NV12);
    return true;
}

bool Yuv2Rgb::YuvToRgb422p(unsigned char *yuvFrame, unsigned char *rgbFrame, int width, int height)
{
    if (yuvFrame == nullptr) {
        return false;
    }

    int y, u, v;
    int frameSize = width * height;

    for (int j = 0; j < height; j++){
        unsigned char* pRGB = rgbFrame + j * width * 3;
        unsigned char* pY = yuvFrame + j * width;
        unsigned char* pU = yuvFrame + frameSize + (j >> 1) * width;
        unsigned char* pV = pU + (frameSize >> 1);
        for (int i = 0; i < width; i++)
        {
            y = *(pY + i);
            u = *(pU + (i >> 1));
            v = *(pV + (i >> 1));

            *(pRGB) = CLIPVALUE(YUVToR(y, u, v), 0, 255);
            *(pRGB + 1) = CLIPVALUE(YUVToG(y, u, v), 0, 255);
            *(pRGB + 2) = CLIPVALUE(YUVToB(y, u, v), 0, 255);
            pRGB += 3;
        }
    }

    return 0;
}

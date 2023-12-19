#include "yuv2rgb.h"
#include <stdio.h>

#include "opencv2/opencv.hpp"

using namespace cv;

Yuv2Rgb::Yuv2Rgb()
{

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

bool Yuv2Rgb::RgbaToYuvNV12(unsigned char* rgbFrame, unsigned char* yuvFrame, int width, int height)
{
    cv::Mat yuv;
    cv::Mat yuvImg(height*3/2, width, CV_8UC1, yuvFrame);
    cv::Mat rgbImg(height, width, CV_8UC4, rgbFrame);
    cv::cvtColor(rgbImg, yuv, cv::COLOR_RGB2YUV_I420);
    cv::cvtColor(yuv, yuvImg, cv::COLOR_YUV2RGB_NV12);
    return true;
}

#ifndef YUV_TO_IMG_H
#define YUV_TO_IMG_H

#include <opencv2/opencv.hpp>
#include <QImage>

class YuvToImg{
public:
    YuvToImg();


    // 输入YUV数据，输出RGB数据
    bool yuv422pToRgb(uint8_t *rgbFrame, uint8_t *yuvData, int width, int height);

    QImage rgbToQImage(uint8_t *rgbData, int width, int height);

};
#endif

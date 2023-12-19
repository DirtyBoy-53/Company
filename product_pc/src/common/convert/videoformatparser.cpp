#include "videoformatparser.h"
#include <yuv2rgb.h>

struct FrameInfo_S{
    uint64_t uHeadFlag;//0xefefefef80808080
    int32_t uTotalLen;
    int64_t uTimeTm;
    int32_t uNvsLen;
    int32_t uY16Len;
    int32_t uYuvLen;
    int32_t uParamLen;
    int32_t uFormat; //1 yuv422 3 yuv16 5 y16+yuv 7 x16  0xffff 原始数据
};

VideoFormatParser::VideoFormatParser()
{

}

void VideoFormatParser::rawToRgb(QByteArray &rawData, QByteArray &rgbData)
{
    FrameInfo_S frameInfo;
    memcpy(&frameInfo, rawData.data(), sizeof(frameInfo));
    if(frameInfo.uFormat != 1) {
        return;
    }
    QByteArray uyuvData = rawData.mid(128
                                      + frameInfo.uNvsLen
                                      + frameInfo.uY16Len,
                                      frameInfo.uYuvLen);
    uint8_t *pRgb = (uint8_t *)malloc(1024 * 1024 * 2);
    Yuv2Rgb::YuvToRgbUYVY((unsigned char *)uyuvData.data(),
                          pRgb,
                          640,
                          512);
    QByteArray data = QByteArray::fromRawData((char *)pRgb, 640 * 512 * 3);
    rgbData = data;
}

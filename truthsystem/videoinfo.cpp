#include "videoinfo.h"
#include <yuv2rgb.h>
#include <mapping.h>
#include <configinfo.h>


VideoFormatParser::VideoFormatParser()
{
    m_vlYuv = (uint8_t*)malloc(1024 * 1024 * 20);
}

QByteArray VideoFormatParser::srcFrameToRgb(uint8_t * pSrcData)
{
    QByteArray aDst;
    FrameInfo_S * pFrame = (FrameInfo_S *) pSrcData;
    int headLen = HEADSIZE;
    aDst.resize(pFrame->width * pFrame->height * 3);
    if(pFrame->uFormat == VIDEO_FORMAT_UYVY || pFrame->uFormat == VIDEO_FORMAT_SRC) {
        uint8_t * pData = pSrcData + headLen + pFrame->uNvsLen;
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *) pData,
                              (unsigned char *) aDst.data(),
                              pFrame->width, pFrame->height);
    }else if(pFrame->uFormat == VIDEO_FORMAT_Y16) {
        uint8_t * pData = pSrcData + headLen;
        Mapping map;
        map.Data16ToRGB24((short*)pData,
                          (unsigned char *) aDst.data(),
                          pFrame->width * pFrame->height, 9);
    }else if(pFrame->uFormat == VIDEO_FORMAT_X16) {
        uint8_t * pData = pSrcData + headLen;
        Mapping map;
        map.Data16ToRGB24((short*)pData,
                          (unsigned char *) aDst.data(),
                          pFrame->width * pFrame->height, 9);
    }else if(pFrame->uFormat == VIDEO_FORMAT_NV21) {
        uint8_t * pData = pSrcData + headLen;
        Yuv2Rgb::YuvToRgbNV12((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              pFrame->width, pFrame->height);

    }else if(pFrame->uFormat == VIDEO_FORMAT_YUV420) {
        uint8_t * pData = pSrcData + headLen;
        Yuv2Rgb::YuvToRgb420((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              pFrame->width, pFrame->height);

    }else if(pFrame->uFormat == VIDEO_FORMAT_YUV420) {
        uint8_t * pData = pSrcData + headLen;
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              pFrame->width, pFrame->height);

    }else {
        uint8_t * pData = pSrcData + headLen;
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *) pData,
                              (unsigned char *) aDst.data(),
                              pFrame->width, pFrame->height);
    }
    return aDst;
}

QByteArray VideoFormatParser::srcToRgb(H264Decode* decoder, int format, int w, int h, uint8_t *pSrcData, int len)
{
    QByteArray aDst;
    uint8_t * pData = pSrcData;
    aDst.resize(w * h * 3);
    if(format == VIDEO_FORMAT_UYVY || format == VIDEO_FORMAT_SRC) {
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *) pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }else if(format == VIDEO_FORMAT_Y16) {
        Mapping map;
        map.Data16ToRGB24((short*)pData,
                          (unsigned char *) aDst.data(),
                          w * h, 0);
    }else if(format == VIDEO_FORMAT_X16) {
        Mapping map;
        map.Data16ToRGB24((short*)pData,
                          (unsigned char *) aDst.data(),
                          w * h, 0);
    }else if(format == VIDEO_FORMAT_NV21) {
        Yuv2Rgb::YuvToRgbNV12((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }else if(format == VIDEO_FORMAT_YUV420) {
        Yuv2Rgb::YuvToRgb420((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }else if(format == VIDEO_FORMAT_YUV420) {
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }else if(format == VIDEO_FORMAT_H264) {
        unsigned char* irYuv = new unsigned char[w*h*3/2];
        unsigned char* irU = irYuv+w*h;
        unsigned char* irV = irYuv+w*h + w*h/4;;
        decoder->h264ToYuv(pData, irYuv, irU, irV, len, w, h);
        Yuv2Rgb::YuvToRgb420p(irYuv, (unsigned char*)aDst.data(), w, h);
        delete irYuv;
    }else if(format == VIDEO_FORMAT_Y) {
        uint8_t* irYuv = new unsigned char[w*h*3/2];
        uint8_t* irUV = irYuv + w*h;
        memcpy(irYuv, pData, w*h);
        memset(irUV, 0x80, w*h/2);
        Yuv2Rgb::YuvToRgb420p(irYuv, (unsigned char*)aDst.data(), w, h);
        delete irYuv;
    } else {
        Yuv2Rgb::YuvToRgbUYVY((unsigned char *) pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }
    return aDst;
}

bool VideoFormatParser::srxToFrameData(QByteArray srcArray, VideoData_S &frameData)
{
    if(srcArray.size() < HEADSIZE) {
        return false;
    }
    FrameInfo_S * pData = (FrameInfo_S *) srcArray.data();
    frameData.headLen = HEADSIZE;
    frameData.irHeight = pData->height;
    frameData.irWidth = pData->width;
    frameData.format = pData->uFormat;
    frameData.irFrameLen = pData->width*pData->height*2;
    frameData.nvsLen = pData->uNvsLen;
    frameData.paramLen = pData->uParamLen;
    frameData.totalLen = pData->uTotalLen;
    frameData.vlWidth = pData->vlWidth;
    frameData.vlHeight = pData->vlHeight;
    frameData.vlFrameLen = pData->vlLen;
    if(frameData.format == VIDEO_FORMAT_NV21 || frameData.format == VIDEO_FORMAT_YUV420) {
        frameData.irFrameLen = pData->width*pData->height*3/2;
    }
    return true;
}

bool VideoFormatParser::hasHeader(QByteArray data, VideoData_S &frameData)
{
    if(data.size() < FILEHEADSIZE) {
        return false;
    }

    frameData.fileHeadLen = FILEHEADSIZE;
    frameData.headLen = 0;
    frameData.irHeight = 512;
    frameData.irWidth = 640;
    frameData.format = VIDEO_FORMAT_NV21;
    frameData.irFrameLen = 640*512*3/2;
    frameData.nvsLen = 640 * 4;
    frameData.paramLen = 640 * 4 * 2;
    frameData.totalLen = frameData.irFrameLen + frameData.nvsLen + frameData.paramLen;
    return true;
}

QByteArray VideoFormatParser::vlSrcToRgb(H264Decode* decoder, int format, int w, int h, uint8_t *srcData, int len)
{
    QByteArray aDst;
    uint8_t * pData = srcData;
    aDst.resize(w * h * 3);
    if(format == 1) {
        unsigned char* vlYuvUyvy = new unsigned char[w*h*2];
        unsigned char* vlY = pData;
        unsigned char* vlU = vlY + w * h;
        unsigned char* vlV = vlY + w * h + w * h/2;
        for(int i=0; i<w*h/2; i++) {
            vlYuvUyvy[4*i]   = vlU[i];
            vlYuvUyvy[4*i+1] = vlY[2*i];
            vlYuvUyvy[4*i+2] = vlV[i];
            vlYuvUyvy[4*i+3] = vlY[2*i+1];
        }
        Yuv2Rgb::YuvToRgbUYVY(vlYuvUyvy, (unsigned char *) aDst.data(), w, h);
        delete vlYuvUyvy;
    }else if(format == 2) {
        Yuv2Rgb::YuvToRgb420p((unsigned char *)pData,
                              (unsigned char *) aDst.data(),
                              w, h);
    }else if(format == 3) {
        unsigned char* vlYuv = new unsigned char[w*h*3/2];
        unsigned char* vlU = vlYuv+w*h;
        unsigned char* vlV = vlYuv+w*h + w*h/4;;
        decoder->h264ToYuv(pData, vlYuv, vlU, vlV, len, w, h);
        Yuv2Rgb::YuvToRgb420p(vlYuv, (unsigned char*)aDst.data(), w, h);
        delete vlYuv;
    }else if(format == 4){
        for (auto i = 0; i < w * h * 2; i += 4) {
            m_vlYuv[i] = pData[i + 1];//U
            m_vlYuv[i + 1] = pData[i];//Y1
            m_vlYuv[i + 2] = pData[i + 3];//V
            m_vlYuv[i + 3] = pData[i + 2];//Y2
        }
        Yuv2Rgb::YuvToRgbUYVY(m_vlYuv, (unsigned char*)aDst.data(), w, h);
    }
    else {
        Yuv2Rgb::YuvToRgb420p((unsigned char*)pData,(unsigned char*)aDst.data(), w, h);
    }
    return aDst;
}

QByteArray VideoFormatParser::vlSrcToYuv(H264Decode* decoder, int format, int w, int h, uint8_t *srcData, int len)
{
    QByteArray aDst;
    uint8_t * pData = srcData;

    if(format == 3) {
        unsigned char* vlYuv = new unsigned char[w*h*3/2];
        unsigned char* vlU = vlYuv+w*h;
        unsigned char* vlV = vlYuv+w*h + w*h/4;;
        decoder->h264ToYuv(pData, vlYuv, vlU, vlV, len, w, h);
        aDst.resize(w*h*3/2);
        memcpy(aDst.data(), vlYuv, w*h*3/2);
        delete vlYuv;
    } else {
        aDst.resize(len);
        memcpy(aDst.data(), srcData, len);
    }
    return aDst;
}



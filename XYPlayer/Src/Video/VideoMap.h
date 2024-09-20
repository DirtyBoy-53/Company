#ifndef VIDEOMAP_H
#define VIDEOMAP_H

#include <string.h>
#include <cstdio>

#define FRAME_SMOOTH_NUM        (16)//(100)
// #define maxA(a, b) ((a) > (b) ? (a) : (b))
// #define minA(a, b) ((a) < (b) ? (a) : (b))
#define ARGB_A(p) (((p) & 0xFF000000) >> 24)
#define ARGB_R(p) (((p) & 0x00FF0000) >> 16)
#define ARGB_G(p) (((p) & 0x0000FF00) >>  8)
#define ARGB_B(p)  ((p) & 0x000000FF)

#define ClipU8(x) ((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))
// #ifndef maxA
// #define maxA(a,b)            (((a) > (b)) ? (a) : (b))
// #endif
// #ifndef minA
// #define minA(a,b)            (((a) < (b)) ? (a) : (b))
// #endif
// ABGR
#define MAKE_ABGR(r, g, b, a) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))
#define nHistSize (65536)
#define GRAYLEVEL_16BIT         (65536)
struct SPOINT
{
    int x;
    int y;
};

void swap_endian(char *array, size_t size);
void  rgb2yuv422Planar(const unsigned char *rgbData, int height, int width, int widthstep,unsigned char*yuvData);
bool Data16ToRGB24(short *srcData, unsigned char *destData, int size, int paletteIndex);

class VideoMap {
public:
    void GetHist(unsigned short *pHist, int *pnMaxOri, int *pnMinOri, int *pnMaxDisc, int *pnMinDisc, int *pGrayMean, short *pusSrcImg, int n_width,
                 int n_height, int nHistsize, int nUpDiscardRatio, int nDownDiscardRatio);

    /**
    * 手动调光接口函数
    * 输入参数：
    * @param puc_dst 调光后y8
    * @param pus_src Y16数据
    * @param n_width Y16数据宽
    * @param n_height Y16数据高
    * @param nMixContrastExp 对比度期望，默认值255
    * @param nMixBrightExp   亮度期望，默认值80
    * @param nRestrainRangeThre 均匀面抑制阈值，默认值128
    * @param n_mixrange 混合调光映射范围，默认值200
    * @param nMidValue  混合调光映射中值 ，默认值90
    * @param nPlatThresh 混合调光平台阈值，默认值50
    * @param nUpDiscardRatio 上抛点，默认值5
    * @param nDownDiscardRatio 下抛点，默认值5
    * @param isImgEnhance 细节增强算法开关
    * @return
    */
    void DRC_Mix(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int isImgEnhance);
    void y8ToRgb(unsigned char* y8Src, unsigned char* rgbData, int width, int height);

private:
    const int GrayRangeMax = 128;
    const int Hist_Length = 65536;
    int Hist_Offset = Hist_Length / 2;
    const float Dispose_Percent = 0.02f;
    short Y16_OFFSET = 16383;// Y16偏移量

    volatile int gBrightExp;
    volatile int gContrastExp;

    int *pHist;
    void *pixels = NULL;
    void *pRecordingPixels = NULL;
    void *pMarkerPixels = NULL;
    void *pLogoPixels = NULL;
    short *pGrayRange = NULL;
    short *pGrayMean = NULL;
    short *pHistMap = NULL;
    int gLogoWidth;
    int gLogoHeight;
    //BYTE *pOriginalRgba;// 原始探测器分辨率的RGBA

    int HIST_OFFSET = (640 * 512);	//修改，根据分辨率来计算
    int gIrWidth = 640;
    int gIrHeight = 512;
    int gImageLen = (640 * 512); //图像分辨率
    int g_nFrameCnt = 0;
    int g_nLastFrameRange = 0;
    int g_nLastFrameMean = 0;
    int g_nMaxContrast = 4;
    int g_nGrayRange = 0;

    int gCurFrameNum = 0;
    int nAlpha = 0;
    int nBeta = 0;
    int lThrHigh = 300;
    int lThrLow = 100;
    short pLinearMap[65536] = {0};

    //调光相关
    bool isManualMode = false;
    short gMaxMappingY16;
    short gMinMappingY16;
    int g_nRangeSmooth[FRAME_SMOOTH_NUM] = {0};

    //调光开放调试变量
    int nMixContrastExp = 255;  //对比度
    int nMixBrightExp = 80; //亮度
    int nRestrainRangeThre=128; // 均匀面抑制值
    int n_mixrange = 200;  //直方图映射范围
    int nMidValue = 90; // 映射中值
    int nPlatThresh = 50; //平台阈值
    int g_nOffset = 32768;
    unsigned short gFrameRangeValue = 64;// 均匀面抑制
    unsigned short gFrameRangeScale = 4;// 均匀面抑制倍数
    // 区域调光
    bool isLocalManualMode = false;
    struct SPOINT sLocalManualPoint;
};
#endif // VIDEOMAP_H

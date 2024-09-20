#include "YuvToImg.h"

#define CLIPVALUE(x, minValue, maxValue) ((x) < (minValue) ? (minValue) : ((x) > (maxValue) ? (maxValue) : (x)))
#define YUVToR(Y, U, V) ( (Y) + 1.4075 * ((V) - 128) )
#define YUVToG(Y, U, V) ( (Y) - 0.3455 * ((U) - 128) - 0.7169 * ((V) - 128) )
#define YUVToB(Y, U, V) ( (Y) + 1.779 * ((U) - 128) )

#define RGBToY(R, G, B) ( 0.299 * R + 0.587 * G + 0.114 * B )
#define RGBToU(R, G, B) ( -0.147 * R - 0.289 * G + 0.436 * B )
#define RGBToV(R, G, B) ( 0.615 * R - 0.515 * G - 0.100 * B )

YuvToImg::YuvToImg()
{

}

bool YuvToImg::yuv422pToRgb(uint8_t *rgbFrame, uint8_t *yuvData, int width, int height)
{
    if (!yuvData || !rgbFrame) {
        return false;
    }

    int y, u, v;
    int frameSize = width * height;

    for (int j = 0; j < height; j++){
        uint8_t* pRGB = rgbFrame + j * width * 3;
        uint8_t* pY = yuvData + j * width;
        uint8_t* pU = yuvData + frameSize + (j >> 1) * width;
        uint8_t* pV = pU + (frameSize >> 1);
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
    return true;
}

QImage YuvToImg::rgbToQImage(uint8_t *rgbData, int width, int height)
{
    return QImage(rgbData, width, height, QImage::Format_RGB888);
}

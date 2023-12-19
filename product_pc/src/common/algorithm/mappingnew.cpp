//MeanFilter_16bit(ps_detail_img, y16_buf, gIrWidth, gIrHeight);
//DRC_Mix(y8_buf, y16_buf, gIrWidth, gIrHeight, nMixContrastExp, nMixBrightExp,nRestrainRangeThre, n_mixrange, nMidValue, nPlatThresh, 5, 5, 1);
#include "mappingnew.h"
#include <string.h>
#include <cstdio>
#include <QDebug>

#define max_ir(a,b)            (((a) > (b)) ? (a) : (b))
#define min_ir(a,b)            (((a) < (b)) ? (a) : (b))

static short *ps_detail_img = new short[640*512];

inline void MeanFilter_16bit(short *pus_dst, short *pus_src, int n_width, int n_height)
{
    // 垫衬扩展范围
	int temp = pus_src[0] + pus_src[0 + 1] + pus_src[0 + 2]
                   + pus_src[0] + pus_src[0 + 1] + pus_src[0 + 2]
                   + pus_src[n_width + 0] + pus_src[n_width +  1];
	temp = temp>>3;
	//左上角
    pus_dst[0] = pus_src[0] - temp;
	temp = pus_src[n_width - 3] + pus_src[n_width - 2] + pus_src[n_width - 1]
                   + pus_src[n_width - 3] + pus_src[n_width - 2] + pus_src[n_width - 1]
                   + pus_src[n_width * 2 - 3] + pus_src[n_width * 2 - 2];
	temp = temp>>3;
	////右上角
    pus_dst[n_width - 1] = pus_src[n_width - 1] - temp;
	temp = pus_src[(n_height - 2)*n_width] + pus_src[(n_height - 2)*n_width + 1] + pus_src[(n_height - 2)*n_width + 2]
               + pus_src[(n_height - 1)*n_width] + pus_src[(n_height - 1)*n_width + 1] + pus_src[(n_height - 1)*n_width + 2]
               + pus_src[(n_height - 1)*n_width] + pus_src[(n_height - 1)*n_width + 1];
	temp = temp>>3;
    pus_dst[(n_height - 1)*n_width] = pus_src[(n_height - 1)*n_width] - temp;
	temp = pus_src[(n_height - 1)*n_width - 3] + pus_src[(n_height - 1)*n_width - 2] + pus_src[(n_height - 1)*n_width - 1]
               + pus_src[n_height*n_width - 3] + pus_src[n_height*n_width - 2] + pus_src[n_height*n_width - 1]
               + pus_src[n_height*n_width - 3] + pus_src[n_height*n_width - 2];
	temp = temp>>3;
    pus_dst[n_height*n_width - 1] = pus_src[n_height*n_width - 1]- temp;
    unsigned char n_dr = 20;
    for(int i=0; i<n_width - 2; i++)
    {
        temp = pus_src[i] + pus_src[i + 1] + pus_src[i + 2]
                   + pus_src[i] + pus_src[i + 1] + pus_src[i + 2]
                   + pus_src[n_width + i] + pus_src[n_width + i + 1];
        temp = temp>>3;
        pus_dst[i + 1] = pus_src[i + 1] - temp;

        temp = pus_src[(n_height - 2)*n_width + i] + pus_src[(n_height - 2)*n_width + i + 1] + pus_src[(n_height - 2)*n_width + i + 2]
               + pus_src[(n_height - 1)*n_width + i] + pus_src[(n_height - 1)*n_width + i + 1] + pus_src[(n_height - 1)*n_width + i + 2]
               + pus_src[(n_height - 1)*n_width + i] + pus_src[(n_height - 1)*n_width + i + 1];
        temp = temp>>3;
        pus_dst[(n_height - 1)*n_width + i + 1] = pus_src[(n_height - 1)*n_width + i + 1] - temp;
    }

    for(int i=0; i<n_height - 2; i++)
    {
        temp = pus_src[i*n_width] + pus_src[i*n_width] + pus_src[i*n_width + 1]
                   + pus_src[(i+1) * n_width] + pus_src[(i+1) * n_width] + pus_src[(i+1) * n_width + 1]
                   + pus_src[(i+2) * n_width] + pus_src[(i+2) * n_width];
        temp = temp>>3;
        pus_dst[(i+1) * n_width] = pus_src[(i+1) * n_width] - temp;

        temp = pus_src[i*n_width + n_width - 2] + pus_src[i*n_width + n_width - 1] + pus_src[i*n_width  + n_width - 1]
               + pus_src[(i+1) * n_width + n_width - 2] + pus_src[(i+1) * n_width + n_width - 1] + pus_src[(i+1) * n_width  + n_width - 1]
               + pus_src[(i+2) * n_width + n_width - 2] + pus_src[(i+2) * n_width + n_width - 1];
        temp = temp>>3;
        pus_dst[(i+1) * n_width + n_width - 1] = pus_src[(i+1) * n_width + n_width - 1] - temp;
    }
    //-------------------------------------------------------------------
    for (int i = 0; i < n_height - 2; i++)
    {
        for (int j = 0; j < n_width - 2; j++)
        {
            temp = pus_src[i*n_width + j] + pus_src[i*n_width + j + 1] + pus_src[i*n_width + j + 2]
                       + pus_src[(i+1)*n_width + j ] + pus_src[(i+1)*n_width + j + 1] + pus_src[(i+1)*n_width + j + 2]
                       + pus_src[(i+2)*n_width + j] + pus_src[(i+2)*n_width + j + 1];
            temp = temp>>3;
            pus_dst[(i+1)*n_width + j + 1] = pus_src[(i+1)*n_width + j + 1] - temp;
            pus_dst[(i+1)*n_width + j + 1] = min_ir(n_dr, max_ir(pus_dst[(i+1)*n_width + j + 1], -n_dr));
        }
    }
}

void MappingNew::GetHist(unsigned short* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short *pusSrcImg, int n_width, int n_height, int nHistsize, int nUpDiscardRatio, int nDownDiscardRatio)
{
    int i;
    int nPixVal;
    int nHistmax = 0;
    int nHistmin = nHistsize;
    int n_len = n_width * n_height;
    int nSigma_1 = (int)(nDownDiscardRatio * n_len / 1000);//下抛点比例
    int nSigma_2 = (int)(nUpDiscardRatio * n_len / 1000); //上抛点比例
    int nCnt = 0;
    int nOrimin = nHistsize;
    int nOrimax = 0;
    int bFindVal[4] = { 0 };
    long long nGraySum = 0;

    for (i = 0; i < n_len; i++)
    {
        nPixVal = (int)(pusSrcImg[i] + g_nOffset);
		if (pHist[nPixVal] < 65535)
		{
			pHist[nPixVal]++;
		}
		
		nGraySum += nPixVal;
		if (nPixVal < nOrimin)
        {
            nOrimin = nPixVal;
        }
        else if (nPixVal > nOrimax)
        {
            nOrimax = nPixVal;
        }
    }
    *pnMaxOri = (int)(nOrimax - g_nOffset);
    *pnMinOri = (int)(nOrimin - g_nOffset);
    *pGrayMean = (int)(nGraySum / n_len - g_nOffset); // 灰度均值

    for (i = nOrimin; i <= nOrimax; i++)
    {
        nCnt += pHist[i];

        if (nCnt >= nSigma_1 && pHist[i] > 0 && bFindVal[0] == 0)
        {
            nHistmin = i;
            bFindVal[0] = 1;
            break;
        }
    }

    nCnt = 0;
    for (i = nOrimax; i >= nOrimin; i--)
    {
        nCnt += pHist[i];

        if (nCnt >= nSigma_2 && pHist[i] > 0 && bFindVal[1] == 0)
        {
            nHistmax = i;
            bFindVal[1] = 1;
            break;
        }
    }

    *pnMaxDisc = nHistmax - g_nOffset;
    *pnMinDisc = nHistmin - g_nOffset;
}

void MappingNew::y8ToRgb(unsigned char* y8Src, unsigned char* rgbData, int width, int height)
{
    unsigned char y0;
    float r0, g0, b0;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            y0 = (y8Src[i * width + j]) & 0xFF;
            r0 = y0;
            g0 = y0;
            b0 = y0;

            r0 = r0 > 255 ? (255) : (r0);
            r0 = r0 < 0 ? (0) : (r0);
            g0 = g0 > 255 ? (255) : (g0);
            g0 = g0 < 0 ? (0) : (g0);
            b0 = b0 > 255 ? (255) : (b0);
            b0 = b0 < 0 ? (0) : (b0);

            rgbData[i * width * 3 + j * 3 + 0] = (unsigned char)r0;
            rgbData[i * width * 3 + j * 3 + 1] = (unsigned char)g0;
            rgbData[i * width * 3 + j * 3 + 2] = (unsigned char)b0;

        }
    }
}

void MappingNew::DRC_Mix(unsigned char* puc_dst, short *pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nUpDiscardRatio, int nDownDiscardRatio, int detail_switch)
{
    int i = 0;
    int n_len = n_width * n_height;
    int nGrayRange = 0;  //抛点后动态范围
    int nBright = 0;
    int nContrast = 0;
    int nCompensate = 0;//64;  //动态调整动态范围补偿值，防止动态范围过小时，出现图像对比度过大的现象
    int nGrayMean = 0;
    int nOriMax = 0;
    int nOriMin = 0;
    int nDiscMax = 0;
    int nDiscMin = 0;
    int nIndex;
    int nY8start;
    int nY8Range;
    int nZoom = 1024;
    int nHistCumMax;
    unsigned short aHist[GRAYLEVEL_16BIT] = { 0 };
    unsigned int pHistCum[GRAYLEVEL_16BIT] = { 0 };
    unsigned char pMapTable[GRAYLEVEL_16BIT] = { 0 };

    // 统计直方图
    GetHist(aHist, &nOriMax, &nOriMin, &nDiscMax, &nDiscMin, &nGrayMean, pus_src, n_width, n_height, GRAYLEVEL_16BIT, nUpDiscardRatio, nDownDiscardRatio);
    nGrayRange = nDiscMax - nDiscMin + 1;
    if (gCurFrameNum < FRAME_SMOOTH_NUM)
    {
        g_nRangeSmooth[gCurFrameNum] = nGrayRange;
        nGrayRange = 0;
        for (i = 0; i < (gCurFrameNum + 1); i++)
        {
            nGrayRange += g_nRangeSmooth[i];
        }
        nGrayRange /= (gCurFrameNum + 1);
    }
    else
    {
        memcpy(g_nRangeSmooth, g_nRangeSmooth + 1, (FRAME_SMOOTH_NUM - 1) * sizeof(int));
        g_nRangeSmooth[FRAME_SMOOTH_NUM - 1] = nGrayRange;
        nGrayRange = 0;
        for (i = 0; i < FRAME_SMOOTH_NUM; i++)
        {
            nGrayRange += g_nRangeSmooth[i];
        }
        nGrayRange /= FRAME_SMOOTH_NUM;
    }
    gCurFrameNum++;

    //// 自适应计算补偿量
    if (nGrayRange < nRestrainRangeThre)
    {
        nCompensate = nRestrainRangeThre - nGrayRange;
    }
    else
    {
        nCompensate = 0;
    }
    nContrast = nZoom * nMixContrastExp / (nGrayRange + nCompensate * 4);

    // 最大对比度限制
    if (nContrast > nZoom * g_nMaxContrast)
    {
        nContrast = nZoom * g_nMaxContrast;
    }
    nBright = nMixBrightExp - (nGrayMean * nContrast) / nZoom;

    // 平台阈值处理并累计
    unsigned int temp = 0;
    for (i = nOriMin; i <= nOriMax; i++)
    {
        nIndex = i + g_nOffset;
        if (aHist[nIndex] > nPlatThresh)   //某一灰度级直方图数量超过平台阈值
        {
            aHist[nIndex] = (unsigned short)nPlatThresh;
        }
        temp += aHist[nIndex];     //直方图前面的累加和
        pHistCum[nIndex] = temp;
    }
	nHistCumMax = (int)(pHistCum[nOriMax + g_nOffset]);
    //新增 当Y16动态范围较小时减小直方图均衡的程度，抑制锅盖
    if (nGrayRange < n_mixrange)
    {
        n_mixrange = nGrayRange;
    }

    // 自适应权值
    int nAlpha = 0;
    int lThrHigh = 300;
    int lThrLow = 100;
    int nThrLowL = lThrLow * 0.2;
    if (nGrayRange < lThrHigh && nGrayRange > lThrLow)
    {
        nAlpha = 616 * (lThrHigh - nGrayRange) / (lThrHigh - lThrLow) + 204;
    }
    else if (nGrayRange >= lThrHigh)
    {
        nAlpha = 204;
    }
    else if (nGrayRange > nThrLowL && nGrayRange <= lThrLow)
    {
        nAlpha = 204 * (lThrLow - nGrayRange) / (lThrLow - nThrLowL) + 820;
    }
    else if (nGrayRange <= nThrLowL)
    {
        nAlpha = 1024;
    }
    int nBeta = 1024 - nAlpha;

    nY8Range = n_mixrange;
    nY8start = nMidValue - nY8Range / 2;
    short nHistMapValue;
    short nLinearMapValue;
    for (i = nOriMin; i <= nOriMax; i++)
    {
        nIndex = i + g_nOffset;
        nHistMapValue = short(((pHistCum[nIndex] * nY8Range / nHistCumMax)) + nY8start);
        nLinearMapValue = short(((nContrast * i) >> 10) + nBright);
        pMapTable[nIndex] = (unsigned char)(ClipU8((nAlpha * nLinearMapValue + nBeta * nHistMapValue) >> 10));
    }

    // 增强开关放到for循环外面判断，减少for循环时间
    if(detail_switch == 1){
        MeanFilter_16bit(ps_detail_img, pus_src, gIrWidth, gIrHeight);
        short nTemp;
        for (i = 0; i < n_len; i++)
        {
            nTemp = pMapTable[pus_src[i] + g_nOffset] + (ps_detail_img[i] << 1);
            if(nTemp < 0)
                puc_dst[i] = 0;
            else if(nTemp > 255)
                puc_dst[i] = 255;
            else
                puc_dst[i] = nTemp;
        }
    } else {
        for (i = 0; i < n_len; i++)
        {
            puc_dst[i] = pMapTable[pus_src[i] + g_nOffset];
        }
    }
}

#include "netd.h"
#include <windows.h>
#include <math.h>

// TODO: reference additional headers your program requires here
#include <stdio.h>
#include "stdlib.h"

//#include "opencv2/core/core.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include <opencv2/opencv.hpp>
//#include "contrib.hpp"

#define WIDTH				640	//384				//图像宽
#define HEIGHT				512	//288				//图像高



//int main()
//{
//	string dir_path20 = "C:\\Users\\04486gzy\\Desktop\\111\\2018-11-29 095338.raw";  //20度视频路径
//	string dir_path25 = "C:\\Users\\04486gzy\\Desktop\\111\\2018-11-29 095856.raw";  //25度视频路径
//	string dir_path30 = "C:\\Users\\04486gzy\\Desktop\\111\\2018-11-29 095915.raw";  //30度视频路径
//
//	unsigned int *mean_20 = (unsigned int*)malloc(WIDTH * HEIGHT * sizeof(unsigned int));
//	memset(mean_20, 0, WIDTH * HEIGHT * sizeof(unsigned int));
//	float *std_25 = (float*)malloc(WIDTH * HEIGHT * sizeof(float));
//	memset(std_25, 0, WIDTH * HEIGHT * sizeof(float));
//	unsigned int *mean_30 = (unsigned int*)malloc(WIDTH * HEIGHT * sizeof(unsigned int));
//	memset(mean_30, 0, WIDTH * HEIGHT * sizeof(unsigned int));
//	unsigned short *para_src = (unsigned short*)malloc(5 * sizeof(unsigned short));
//	memset(para_src, 0, 5 * sizeof(unsigned short));
//	float *temp = (float*)malloc(WIDTH * HEIGHT * sizeof(float));
//	memset(temp, 0, WIDTH * HEIGHT * sizeof(float));
//
//	int w, h;
//	int NETD0 = 0;
//	float NETD1 = 0;
//	float NETD = 0;
//	cal_rawmean(dir_path20, mean_20);
//	cal_rawstd(dir_path25, std_25);
//	cal_rawmean(dir_path30, mean_30);
//	for (w = 0; w < WIDTH; w++)
//		for (h = 0; h < HEIGHT; h++)
//		{
//			NETD0 = mean_30[h * WIDTH + w] - mean_20[h * WIDTH + w];
//			NETD1 = 1 / (10.0 * std_25[h * WIDTH + w]);
//			//NETD = (mean_30[h * WIDTH + w] - mean_20[h * WIDTH + w]) / (10.0 * std_25[h * WIDTH + w]);
//			NETD += NETD0 * NETD1;
//			printf("%f\n", NETD);
//		}
//	NETD = NETD / (WIDTH * HEIGHT / 1000);
//
//	free(mean_20);
//	free(std_25);
//	free(mean_30);
//	free(para_src);
//	return 0;
//}
//
//
//void cal_rawmean(string dirpath, unsigned int *dst)
//{
//	unsigned short *pus_src = (unsigned short*)malloc(WIDTH * HEIGHT * sizeof(unsigned short));
//	memset(pus_src, 0, WIDTH * HEIGHT * sizeof(unsigned short));
//	unsigned short *para_dst = (unsigned short*)malloc(5 * sizeof(unsigned short));
//	memset(para_dst, 0, 5 * sizeof(unsigned short));
//
//	int nFrameCurNum, w, h;
//
//	const char* dir_path = dirpath.data();
//	FILE *raw_file = fopen(dir_path, "rb+");  //打开视频
//	if (raw_file == NULL) {
//		printf("Open File failed.\n");
//		exit(-1);
//	}
//	//计算帧数
//	fseek(raw_file, 0, 2);
//	int count = ftell(raw_file);
//	fseek(raw_file, 0, 0);
//	int nFrameCount = (int)(count / ((HEIGHT*WIDTH + 5) * 2));
//
//	if (nFrameCount >= 64)
//	{
//		for (nFrameCurNum = 0; nFrameCurNum < 64; nFrameCurNum++)
//		{
//			//读每帧参数数据
//			fseek(raw_file, nFrameCurNum * (WIDTH * HEIGHT + 5) * sizeof(short), SEEK_SET);
//			fread(para_dst, 2, 5, raw_file);
//			//读每帧视频数据
//			fseek(raw_file, 5 * sizeof(short) + nFrameCurNum * (WIDTH * HEIGHT + 5) * sizeof(short), SEEK_SET);
//			fread(pus_src, 2, WIDTH * HEIGHT, raw_file);
//
//
//			for (h = 0; h < HEIGHT; h++)
//				for (w = 0; w < WIDTH; w++)
//				{
//					dst[h * WIDTH + w] += (pus_src[h * WIDTH + w]);
//				}
//		}
//	}
//	else
//	{
//		printf("视频帧数不足");
//		exit(0);
//	}
//	for (w = 0; w < WIDTH; w++)
//		for (h = 0; h < HEIGHT; h++)
//		{
//			dst[h * WIDTH + w] /= 64;
//		}
//	free(pus_src);
//	free(para_dst);
//}
//
//void cal_rawstd(string dirpath, float *dst)
//{
//	unsigned int *pus_mean = (unsigned int*)malloc(WIDTH * HEIGHT * sizeof(unsigned int));
//	memset(pus_mean, 0, WIDTH * HEIGHT * sizeof(unsigned int));
//	unsigned short *pus_src = (unsigned short*)malloc(WIDTH * HEIGHT * sizeof(unsigned short));
//	memset(pus_src, 0, WIDTH * HEIGHT * sizeof(unsigned short));
//	unsigned short *para_dst = (unsigned short*)malloc(5 * sizeof(unsigned short));
//	memset(para_dst, 0, 5 * sizeof(unsigned short));
//	unsigned int *pus_temp = (unsigned int*)malloc(WIDTH * HEIGHT * sizeof(unsigned int));
//	memset(pus_temp, 0, WIDTH * HEIGHT * sizeof(unsigned int));
//
//	int nFrameCurNum, w, h;
//
//	const char* dir_path = dirpath.data();
//	FILE *raw_file = fopen(dir_path, "rb+");  //打开视频
//	if (raw_file == NULL) {
//		printf("Open File failed.\n");
//		exit(-1);
//	}
//	//计算帧数
//	fseek(raw_file, 0, 2);
//	int count = ftell(raw_file);
//	fseek(raw_file, 0, 0);
//	int nFrameCount = (int)(count / ((HEIGHT*WIDTH + 5) * 2));
//
//	if (nFrameCount >= 128)
//	{
//		for (nFrameCurNum = 0; nFrameCurNum < 128; nFrameCurNum++)
//		{
//			//读每帧参数数据
//			fseek(raw_file, nFrameCurNum * (WIDTH * HEIGHT + 5) * sizeof(short), SEEK_SET);
//			fread(para_dst, 2, 5, raw_file);
//			//读每帧视频数据
//			fseek(raw_file, 5 * sizeof(short) + nFrameCurNum * (WIDTH * HEIGHT + 5) * sizeof(short), SEEK_SET);
//			fread(pus_src, 2, WIDTH * HEIGHT, raw_file);
//
//			for (w = 0; w < WIDTH; w++)
//				for (h = 0; h < HEIGHT; h++)
//				{
//					pus_mean[h * WIDTH + w] += pus_src[h * WIDTH + w];
//				}
//		}
//		//求64帧平均
//		for (w = 0; w < WIDTH; w++)
//			for (h = 0; h < HEIGHT; h++)
//			{
//				pus_mean[h * WIDTH + w] /= 128;
//			}
//		//计算标准差
//		for (nFrameCurNum = 0; nFrameCurNum < 128; nFrameCurNum++)
//		{
//			//读每帧视频数据
//			fseek(raw_file, 5 * sizeof(short) + nFrameCurNum * (WIDTH * HEIGHT + 5) * sizeof(short), SEEK_SET);
//			fread(pus_src, 2, WIDTH * HEIGHT, raw_file);
//
//			for (w = 0; w < WIDTH; w++)
//				for (h = 0; h < HEIGHT; h++)
//				{
//					pus_temp[h * WIDTH + w] += (pus_src[h * WIDTH + w] - pus_mean[h * WIDTH + w]) * (pus_src[h * WIDTH + w] - pus_mean[h * WIDTH + w]);
//				}
//		}
//		for (w = 0; w < WIDTH; w++)
//			for (h = 0; h < HEIGHT; h++)
//			{
//				dst[h * WIDTH + w] = sqrt(pus_temp[h * WIDTH + w] / 128.0);
//			}
//	}
//	else
//	{
//		printf("视频帧数不足");
//		exit(0);
//	}
//	free(pus_mean);
//	free(pus_src);
//	free(para_dst);
//}

//获取累积平均值图像
//pMeanImageData：输出累积平均值图像数据
//pImageData:输入Y8图像；
//width:图像宽
//height:图像高
//roiMYRECT:设定的矩形区域；
//返回值：程序状态量0/-1;
int GetMeanImage(float *pMeanImageData, unsigned char *pImageData, int width, int height, MYRECT roiMYRECT)
{
    int col = roiMYRECT.x + roiMYRECT.width;
    int row = roiMYRECT.y + roiMYRECT.height;
    if (col > width || row > height)
    {
        return -1;
    }

    for (int h = roiMYRECT.y; h < row; h++)
    {
        for (int w = roiMYRECT.x; w < col; w++)
        {
            pMeanImageData[h * width + w] += (pImageData[h * width + w]);
        }
    }

    return 0;
}

//获取视频图像数据
//pVedioImageData：输出视频图像数据
//pImageData:输入Y8图像；
//width:图像宽
//height:图像高
//返回值：保存的图像帧数据；
void GetVedioImage(unsigned char *pVedioImageData, unsigned char *pImageData, int width, int height, int num)
{
    int frameLen = width * height;
    memcpy(pVedioImageData + num * frameLen, pImageData, frameLen);
    num++;
}

//获取图像标准差
//pStdImageData：输出图像标准差图像
//pVedioImageData:输入视频数据
//frameNum:输入Y8图像帧数；
//pMeanImageData:低温黑体累积均值图像数据
//width:图像宽
//height:图像高
//lowROIMYRECT:低温黑体矩形区域；
//返回值：程序状态量0/-1;；
int GetImageStd(double *pStdImageData, unsigned char *pVedioImageData, int frameNum, float *pMeanImageData, int width, int height, MYRECT lowROIMYRECT)
{
    int frameLen = width * height;
    int col = lowROIMYRECT.x + lowROIMYRECT.width;
    int row = lowROIMYRECT.y + lowROIMYRECT.height;
    if (col > width || row > height)
    {
        return -1;
    }

    for (int h = lowROIMYRECT.y; h < row; h++)
    {
        for (int w = lowROIMYRECT.x; w < col; w++)
        {
            pMeanImageData[h * width + w] = (pMeanImageData[h * width + w] / frameNum + 0.5f);
        }
    }

    for (int frameID = 0; frameID < frameNum; frameID++)
    {
        for (int h = lowROIMYRECT.y; h < row; h++)
        {
            for (int w = lowROIMYRECT.x; w < col; w++)
            {
                unsigned char pixel = (pVedioImageData + frameNum * frameLen)[h * width + w];
                pStdImageData[h * width + w] = (pixel - pMeanImageData[h * width + w]) * (pixel - pMeanImageData[h * width + w]);
            }
        }
    }

    for (int h = lowROIMYRECT.y; h < row; h++)
    {
        for (int w = lowROIMYRECT.x; w < col; w++)
        {
            pStdImageData[h * width + w] = sqrt(pStdImageData[h * width + w] / frameNum + 0.5);
        }
    }

    return 0;
}

//获取图像标准差
//pNETDImageData：输出图像NETD数据
//pStdImageData:输入低温黑体标准差数据
//pLowMeanImageData:输入低温黑体均值图像；
//pHighMeanImageData:输入高温黑体累积均值图像
//frameNum:视频帧数
//width:图像宽
//height:图像高
//lowROIMYRECT:低温黑体矩形区域；
//highROIMYRECT:高温黑体矩形区域；
//返回值：平均NETD数值；
double GetImageNETD(double *pNETDImageData, double *pStdImageData, float *pLowMeanImageData, float *pHighMeanImageData, int frameNum, int width, int height, MYRECT lowROIMYRECT, MYRECT highROIMYRECT)
{
    int frameLen = width * height;
    int col = highROIMYRECT.x + highROIMYRECT.width;
    int row = highROIMYRECT.y + highROIMYRECT.height;

    for (int i = highROIMYRECT.y; i < row; i++)
    {
        for (int j = highROIMYRECT.x; j < col; j++)
        {
            pHighMeanImageData[i * width + j] = (pHighMeanImageData[i * width + j] / frameNum + 0.5f);
        }
    }

    for (int h = lowROIMYRECT.y, i = highROIMYRECT.y; i < row; h++, i++)
    {
        for (int w = lowROIMYRECT.x, j = highROIMYRECT.x; j < col; w++, j++)
        {
            pNETDImageData[h * width + w] = 10 * pStdImageData[h * width + w] / (pHighMeanImageData[i * width + j] - pLowMeanImageData[h * width + w]) ;

        }
    }

    double sum = 0.0;
    double netd = 0.0;
    for (int h = lowROIMYRECT.y, i = highROIMYRECT.y; i < row; h++, i++)
    {
        for (int w = lowROIMYRECT.x, j = highROIMYRECT.x; j < col; w++, j++)
        {
            sum += pNETDImageData[h * width + w];
        }
    }
    netd = sum / (lowROIMYRECT.width * lowROIMYRECT.height);

    return 1000*netd;
}

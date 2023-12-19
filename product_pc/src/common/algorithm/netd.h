#ifndef NETD_H
#define NETD_H


typedef struct tagMYPOINT
{
    int x;
    int y;
}MYPOINT;

typedef struct tagMYRECT
{
    int x;
    int y;
    int width;
    int height;
}MYRECT;


//获取累积平均值图像
//pMeanImageData：输出累积平均值图像数据
//pImageData:输入Y8图像；
//width:图像宽
//height:图像高
//roiMYRECT:设定的矩形区域；
//返回值：程序状态量0/-1;
int GetMeanImage(float *pMeanImageData, unsigned char *pImageData, int width, int height, MYRECT roiMYRECT);


//获取视频图像数据
//pVedioImageData：输出视频图像数据
//pImageData:输入Y8图像；
//width:图像宽
//height:图像高
//返回值：保存的图像帧数据；
void GetVedioImage(unsigned char *pVedioImageData, unsigned char *pImageData, int width, int height, int num);


//获取图像标准差
//pStdImageData：输出图像标准差图像
//pVedioImageData:输入视频数据
//frameNum:输入Y8图像帧数；
//pMeanImageData:低温黑体累积均值图像数据
//width:图像宽
//height:图像高
//lowROIMYRECT:低温黑体矩形区域；
//返回值：程序状态量0/-1;；
int GetImageStd(double *pStdImageData, unsigned char *pVedioImageData,
                int frameNum, float *pMeanImageData, int width, int height, MYRECT lowROIMYRECT);

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
double GetImageNETD(double *pNETDImageData, double *pStdImageData,
                    float *pLowMeanImageData, float *pHighMeanImageData,
                    int frameNum, int width, int height,
                    MYRECT lowROIMYRECT, MYRECT highROIMYRECT);

#endif

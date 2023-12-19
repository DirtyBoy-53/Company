#ifndef IMAGECHECK_H
#define IMAGECHECK_H

#define IN
#define OUT
#define INOUT
#define PI 3.1415926

//高斯模板大小
#define GAUSSIAN_SIZE 9
//图像高度
//图像宽度
#define HEIGHT 512
#define WIDTH 640
#define WIN_W 10
#define WIN_H 10

typedef unsigned short Y16;// 定义Y16类型

class imagecheck
{
public:
    imagecheck();
    ~imagecheck();

    int Evaluate_HorStripe(double* value1, double* value2, double* value3, unsigned short *pus_src, int n_width, int n_height);
    //生成高斯算子
    int Gaussian(INOUT double *pdKernal, IN double sigma);
    //滤波器
     int Filter(IN double *pdKernal, IN Y16 *py16_SrcImg, OUT Y16 *py16_LowImg, OUT float *pfHighImg, OUT float *pfSrcImg);
    //Std计算
    double Std2(IN float *pfSrcImg);
	//时域STD
	double TimeStd(Y16 *py16_SrcVideo,int nFrameCount, int nFrameN = 50);
};

#endif // IMAGECHECK_H

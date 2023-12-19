#pragma once
#include <QObject>
#include <thread>
#include "opencv2/opencv.hpp"

#define WIDTH				640	//384				//图像宽
#define HEIGHT				512	//288				//图像高

struct MYPOINT
{
    int x;
    int y;
};
struct MYRECT
{
    int x;
    int y;
    int width;
    int height;
};


struct MTF_param
{
    float TC_x=0;
	float TC_y;
	float LC_x;
	float LC_y;
	float CC_x;
	float CC_y;
	float RC_x;
	float RC_y;
	float BC_x;
	float BC_y;
};

struct MTF_param_side
{
    float LT_x=0;
	float LT_y;
	float LB_x;
	float LB_y;
	float RT_x;
	float RT_y;
	float RB_x;
	float RB_y;
};



struct SharpStartPos{
    int x;
    int y;
    int w = 80;
    int h = 80;
};

struct  SharpPos{
    int pos;
    float TC_value;
    float BC_value;
    float CC_value;
    float LC_value;
    float RC_value;
    float avg_value;
    float MTF_C_X;
    float MTF_C_Y;
    double image_netd;
    double space_netd;
};

struct SharpPosSide{
    float LT_value=0;
    float LB_value;
    float RT_value;
    float RB_value;
};


class SRF
{
public:
	SRF();
	~SRF();

    int MTF_Cal(unsigned short* input_data, const std::vector<SharpStartPos> sharpPos, MTF_param& output_MTF);
    int MTF_Side_Cal(unsigned short* input_data, const std::vector<SharpStartPos> sharpPos, MTF_param_side& output_MTF);

    double GetImageSpaceSNR(double *pSpaceSNRData, double *pMeanImageData, double *pStdImageData, unsigned short *pImageData, int width, int height, MYRECT lowROIMYRECT, MYRECT highROIMYRECT);
    double GetImageNETD(double *pNETDImageData, double *pStdImageData, float *pLowMeanImageData, float *pHighMeanImageData, int frameNum, int width, int height, MYRECT lowROIMYRECT, MYRECT highROIMYRECT);
    int GetMeanImage(float *pMeanImageData, unsigned short *pImageData, int width, int height, MYRECT roiMYRECT);
    void GetVedioImage(unsigned short *pVedioImageData, unsigned short *pImageData, int width, int height, int num);
    int GetImageStd(double *pStdImageData, unsigned short *pVedioImageData, int frameNum, float *pMeanImageData, int width, int height, MYRECT lowROIMYRECT);


private:

	float SFR_ROI_Calculation(unsigned short* src_data, int roi_start_x, int roi_start_y, bool& is_vertical);

	void Get_roi_data(unsigned short* img, unsigned short* roi_data, int roi_start_x, int roi_start_y, int m_roi_w, int m_roi_h, int& cal_roi_w, int& cal_roi_h);

	void CentroidFind(unsigned short* data, int width, int height, float& k, float& b);

	void OverSampling(float* deSampling, unsigned short *Src, float slope, int n_width, int n_height);

	void DFT(float *data, int size);

	float Get_MTF(unsigned short* roi_data, int roi_w, int roi_h);

    void resetMTF(MTF_param& output_MTF);

};


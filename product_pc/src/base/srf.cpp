#include "srf.h"
#include "utils.h"
//#include "SharpnessMeasure.h"
#include<iostream>
#include <complex>
#include <ctime>
#include <QDebug>
SRF::SRF()
{
}

SRF::~SRF()
{
}

int SRF::MTF_Cal(unsigned short* input_data, const std::vector<SharpStartPos> sharpPos, MTF_param& output_MTF)
{
//    qDebug() << sharpPos.size();
    if (sharpPos.size() != 9)return -1;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int roi_w = 32, roi_h = 20;
    int cal_roi_w = 0, cal_roi_h = 0;
    bool is_vertical = true;
    unsigned short* roi_data = new unsigned short[20 * 32];

    //�ϱ�MTF����
    int x_center = sharpPos[0].x + 40;
    int y_center = sharpPos[0].y + 40;
    int x_start = x_center + 9;
    int y_start = y_center - 10;
    if (y_center < 15 + 15)
        return -2;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    float temp = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    output_MTF.TC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    x_start = x_center - 9 - roi_w;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.TC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    output_MTF.TC_x = std::max(output_MTF.TC_x, temp);

    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    //    cv::Mat test_roi(cal_roi_h, cal_roi_w, CV_16UC1, roi_data);
    output_MTF.TC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);

    //����MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[1].x + 40;
    y_center = sharpPos[1].y + 40;
    x_start = x_center + 9;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.LC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.LC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.LC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.LC_y);

    //�м�MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[2].x + 40;
    y_center = sharpPos[2].y + 40;
    x_start = x_center + 9;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.CC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    x_start = x_center - 9 - roi_w;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.CC_x = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.CC_x);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.CC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.CC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.CC_y);

    //�ұ�MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[3].x + 40;
    y_center = sharpPos[3].y + 40;
    x_start = x_center - 9 - roi_w;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.RC_y);

    //�±�MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[4].x + 40;
    y_center = sharpPos[4].y + 40;
    x_start = x_center + 9;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.BC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    x_start = x_center - 9 - roi_w;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.BC_x = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.BC_x);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.BC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);

    output_MTF.BC_x*=1280;
    output_MTF.BC_y*=1280;
    output_MTF.TC_x*=1280;
    output_MTF.TC_y*=1280;
    output_MTF.LC_x*=1280;
    output_MTF.LC_y*=1280;
    output_MTF.RC_x*=1280;
    output_MTF.RC_y*=1280;
    output_MTF.CC_x*=1280;
    output_MTF.CC_y*=1280;
//    resetMTF(output_MTF);

    delete[]roi_data;
    return 0;
}


int SRF::MTF_Side_Cal(unsigned short* input_data, const std::vector<SharpStartPos> sharpPos, MTF_param_side& output_MTF)
{
    if (sharpPos.size() != 9)return -1;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int roi_w = 32, roi_h = 20;
    int cal_roi_w = 0, cal_roi_h = 0;
    bool is_vertical = true;
    unsigned short* roi_data = new unsigned short[20 * 32];

    //zuo�ϱ�MTF����
    int x_center = sharpPos[5].x + 40;
    int y_center = sharpPos[5].y + 40;
    int x_start = x_center + 9;
    int y_start = y_center - 10;
    if (y_center < 15 + 15)
        return -2;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    float temp = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    output_MTF.LT_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    x_start = x_center - 9 - roi_w;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.TC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    output_MTF.TC_x = std::max(output_MTF.TC_x, temp);

    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    //    cv::Mat test_roi(cal_roi_h, cal_roi_w, CV_16UC1, roi_data);
    output_MTF.LT_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);

    //youshang��MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[6].x + 40;
    y_center = sharpPos[6].y + 40;
    x_start = x_center - 9 - roi_w;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RT_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center + 9;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RT_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    y_start = y_center - 9 - roi_h;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.LC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.LC_y);

    //�м�MTF����
//    roi_w = 32, roi_h = 20;
//    x_center = sharpPos[2].x + 40;
//    y_center = sharpPos[2].y + 40;
//    x_start = x_center + 9;
//    y_start = y_center - 10;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.CC_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    x_start = x_center - 9 - roi_w;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.CC_x = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.CC_x);
//    roi_w = 20, roi_h = 32;
//    x_start = x_center - 10;
//    y_start = y_center + 9;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.CC_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    y_start = y_center - 9 - roi_h;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.CC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.CC_y);

    //zuoxia��MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[7].x + 40;
    y_center = sharpPos[7].y + 40;
    x_start = x_center + 9;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.LB_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.LB_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    y_start = y_center - 9 - roi_h;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.RC_y = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.RC_y);

    //you�±�MTF����
    roi_w = 32, roi_h = 20;
    x_center = sharpPos[8].x + 40;
    y_center = sharpPos[8].y + 40;
    x_start = x_center - 9 - roi_w;
    y_start = y_center - 10;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RB_x = Get_MTF(roi_data, cal_roi_w, cal_roi_h);
//    x_start = x_center - 9 - roi_w;
//    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
//    output_MTF.BC_x = std::max(Get_MTF(roi_data, cal_roi_w, cal_roi_h), output_MTF.BC_x);
    roi_w = 20, roi_h = 32;
    x_start = x_center - 10;
    y_start = y_center - 9 - roi_h;
    Get_roi_data(input_data, roi_data, x_start, y_start, roi_w, roi_h, cal_roi_w, cal_roi_h);
    output_MTF.RB_y = Get_MTF(roi_data, cal_roi_w, cal_roi_h);

    output_MTF.LT_x*=1280;
    output_MTF.LT_y*=1280;
    output_MTF.LB_x*=1280;
    output_MTF.LB_y*=1280;
    output_MTF.RT_x*=1280;
    output_MTF.RT_y*=1280;
    output_MTF.RB_x*=1280;
    output_MTF.RB_y*=1280;

    delete[]roi_data;
    return 0;
}


float SRF::SFR_ROI_Calculation(unsigned short* src_data, int roi_start_x, int roi_start_y, bool& is_vertical)
{
    int cal_roi_w = 0, cal_roi_h = 0;
    unsigned short* roi_data = new unsigned short[20 * 32];
    int w = 20, h = 32;
    Get_roi_data(src_data, roi_data, roi_start_x, roi_start_y, w, h, cal_roi_w, cal_roi_h);
    float k = 0, b = 0;
    CentroidFind(roi_data, cal_roi_w, cal_roi_h, k, b);

    int sample_len = cal_roi_w * 4;
    float* oversample = new float[sample_len];
    OverSampling(oversample, roi_data, k, cal_roi_w, cal_roi_h);

    float loc = 0.f;
    utils::Getloc(&loc, oversample, sample_len, 1);

    float* oversample1 = new float[sample_len];
    memset(oversample1, 0, sample_len*sizeof(float));
    utils::cent(oversample1, oversample, sample_len, loc);

    float* hamming_data = new float[sample_len];
    utils::Gethanmingwin(hamming_data, sample_len, float(sample_len + 1) / 2);

    //�Ӻ�����
    for (int i = 0; i < sample_len; i++)
        oversample[i] = oversample1[i] * hamming_data[i];
    DFT(oversample, sample_len);
    for (int i = 1; i < sample_len; ++i)
        oversample[i] /= oversample[0];
    oversample[0] = 1;
    float index = 0;
    for (int i = 0; i < sample_len; i++)
    {
        if (oversample[i] < 0.5)
            index = i;
    }
    if (index >= 1)
    {
        int i = int(index);
        index = (oversample[i] - 0.5) / (oversample[i] - oversample[i - 1])*i;
        index += (0.5 - oversample[i - 1]) / (oversample[i] - oversample[i - 1])*(i - 1);
    }
    float frq = index / cal_roi_w;

    delete[]oversample1;
    delete[]hamming_data;
    delete[]roi_data;
    delete[]oversample;
    return frq;
}

void SRF::Get_roi_data(unsigned short* img, unsigned short* roi_data, int roi_start_x, int roi_start_y, int m_roi_w, int m_roi_h,  int& cal_roi_w, int& cal_roi_h)
{
    for (int i = 0; i < m_roi_h; ++i)
    {
        for (int j = 0; j < m_roi_w; ++j)
        {
            roi_data[i * m_roi_w + j] = img[(i + roi_start_y) * 640 + j + roi_start_x];
        }
    }

    //�ж�б���Ƿ�ˮƽ
    int sum_h = 0;
    int sum_v = 0;
    //�����м�����������
    for (int j = 0; j < m_roi_w; j++)
    {
        sum_h += abs(roi_data[3 * m_roi_w + j] - roi_data[(m_roi_h - 3) * m_roi_w + j]);
    }
    //�����м�����������
    for (int j = 0; j < m_roi_h; j++)
    {
        sum_v += abs(roi_data[j * m_roi_w + 3] - roi_data[j * m_roi_w + m_roi_w - 3]);
    }
    cv::Mat test_roi(m_roi_h, m_roi_w, CV_16UC1, roi_data);
    cal_roi_h = m_roi_h;
    cal_roi_w = m_roi_w;
    sum_h /= m_roi_w;
    sum_v /= m_roi_h;
    if (sum_h > sum_v)
    {
        cal_roi_h = m_roi_w;
        cal_roi_w = m_roi_h;

        unsigned short* temp_data = new unsigned short[m_roi_h*m_roi_w];
        cv::Mat test_roi_temp(cal_roi_h, cal_roi_w, CV_16UC1, temp_data);
        for (int i = 0; i < cal_roi_h; ++i)
        {
            for (int j = 0; j < cal_roi_w; ++j)
            {
                temp_data[i * cal_roi_w + j] = roi_data[j * cal_roi_h + cal_roi_h - 1 - i];
            }
        }
        memcpy(roi_data, temp_data, m_roi_h*m_roi_w*sizeof(short));
        delete[]temp_data;
    }
}

void SRF::CentroidFind(unsigned short* data, int width, int height, float& k, float& b)
{
    float mid = float(width + 1) / 2;
    float* hamm_data = new float[width];
    float* src_grad = new float[width*height];
    float* src_grad_temp = new float[width*height];
    float* loc = new float[height];
    float* palce = new float[height];

    //Hanming��
    utils::Gethanmingwin(hamm_data, width, mid);

    //�ж������������ұ���
    int tleft = 0;
    int tright = 0;
    for (int i = 0; i < height; i++)
    {
        tleft += data[i * width + 0];
        tright += data[i * width + width - 1];
    }
    //ʹ��[0.5 -0.5]�ĺ˽��о���
    if (tleft > tright)
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 1; j < width; j++)
            {
                src_grad[i * width + j] = float(data[i * width + j - 1] - data[i * width + j]) / 2;
            }
            src_grad[i * width] = src_grad[i * width + 1]; //�����ݶ�
        }
    }
    else
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 1; j < width; j++)
            {
                src_grad[i * width + j] = -float(data[i * width + j - 1] - data[i * width + j]) / 2;
            }
            src_grad[i * width] = src_grad[i * width + 1]; //�����ݶ�
        }
    }
    //���ݶȼ�hamming��
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            src_grad_temp[i * width + j] = src_grad[i * width + j] * hamm_data[j];
        }
    }

    //��������
    utils::Getloc(loc, src_grad_temp, width, height);
    for (int i = 0; i < height; i++)
    {
        loc[i] = loc[i] - 0.5;
    }

    //���Իع�����ֱ��
    utils::findedge(loc, height, b, k);

    //�ڶ�������
    for (int i = 0; i < height; ++i)
    {
        palce[i] = k * (i + 1) + b;
        //Hanming��
        utils::Gethanmingwin(hamm_data, width, palce[i]);
        //���ݶȼ�hamming��
        for (int j = 0; j < width; j++)
            src_grad[i * width + j] *= hamm_data[j];
    }

    utils::Getloc(loc, src_grad, width, height);

    for (int i = 0; i < height; i++)
    {
        loc[i] = loc[i] - 0.5;
    }
    //���Իع�����ֱ��
    utils::findedge(loc, height, b, k);

    delete[]hamm_data;
    delete[]src_grad;
    delete[]src_grad_temp;
    delete[]loc;
    delete[]palce;
}

void SRF::OverSampling(float* deSampling, unsigned short* Src, float slope, int n_width, int n_height)
{
    int i, j, k;
    slope = 1 / slope;
    int offset = round(4 * (0 - (n_height - 1) / slope));
    int del = abs(offset);
    int start = 1 + round(del / 2);
    int nz = 0;
    int status = 0;

    if (offset > 0)
        offset = 0;
    int nn = n_width * 4;
    int len = nn + del + 100;
    double *DataMap = (double*)malloc((nn + del + 100) * sizeof(double));
    memset(DataMap, 0, (nn + del + 100) * sizeof(double));
    double *Datas = (double*)malloc((nn + del + 100) * sizeof(double));
    memset(Datas, 0, (nn + del + 100) * sizeof(double));

    int ling;
    for (i = 0; i < n_width; i++)
        for (j = 0; j < n_height; j++)
        {
            ling = ceil((i - j / slope) * 4) + 1 - offset;
            if (ling >= 0 && ling < len)
            {
                DataMap[ling] = DataMap[ling] + 1;
                Datas[ling] = Datas[ling] + Src[j * n_width + i];
            }
        }

    for (i = start; i < start + nn; i++)
    {
        if (DataMap[i] == 0)
        {
            nz = nz + 1;
            status = 0;
            if (i == 1){
                DataMap[i] = DataMap[i + 1];
                Datas[i] = Datas[i + 1];
            }
            else{
                DataMap[i] = (DataMap[i - 1] + DataMap[i + 1]) / 2;
                Datas[i] = (Datas[i - 1] + Datas[i + 1]) / 2;
            }
        }
    }
    //std::vector<double> SamplingBar(nn, 0);
    double *SamplingBar = (double*)malloc(nn * sizeof(double));
    memset(SamplingBar, 0, nn * sizeof(double));
    for (i = 0; i < nn; i++)
    {
        SamplingBar[i] = Datas[i + start] / DataMap[i + start];
    }

    //�ж������������ұ���
    int tleft = 0;
    int tright = 0;
    for (i = 0; i < n_height; i++)
    {
        tleft += Src[i * n_width + 0];
        tright += Src[i * n_width + n_width - 1];
    }

    //����չ����deSampling
    if (tleft > tright)
    {
        for (i = 1; i < nn - 1; i++)
        {
            deSampling[i + 1] = (SamplingBar[i - 1] - SamplingBar[i + 1]) / 2;
        }
    }
    else
    {
        for (i = 1; i < nn - 1; i++)
        {
            deSampling[i + 1] = (SamplingBar[i + 1] - SamplingBar[i - 1]) / 2;
        }
    }
    deSampling[1] = deSampling[2];
    deSampling[0] = 0;
    deSampling[nn - 1] = 0;
    free(DataMap);
    free(Datas);
    free(SamplingBar);
}

void SRF::DFT(float* data, int size)
{
    int i, j;
    std::complex<double> *arr = new std::complex<double>[size];
    for (i = 0; i < size; ++i)
    {
        arr[i] = std::complex<double>(data[i], 0);
    }

    for (i = 0; i < size / 2.0; ++i)
    {
        std::complex<double> temp = 0;
        for (j = 0; j < size; ++j)
        {
            double w = 2 * 3.1415*i*j / size;
            std::complex<double> deg(cos(w), -sin(w));
            temp += arr[j] * deg;
        }
        data[i] = sqrt(temp.real()*temp.real() + temp.imag()*temp.imag());
    }
    delete[]arr;

}

float SRF::Get_MTF(unsigned short* roi_data, int roi_w, int roi_h)
{
    float k = 0, b = 0;
    CentroidFind(roi_data, roi_w, roi_h, k, b);

    int sample_len = roi_w * 4;
    float* oversample = new float[sample_len];
    OverSampling(oversample, roi_data, k, roi_w, roi_h);

    float loc = 0.f;
    utils::Getloc(&loc, oversample, sample_len, 1);

    float* oversample1 = new float[sample_len];
    memset(oversample1, 0, sample_len*sizeof(float));
    utils::cent(oversample1, oversample, sample_len, loc);

    float* hamming_data = new float[sample_len];
    utils::Gethanmingwin(hamming_data, sample_len, float(sample_len + 1) / 2);

    //�Ӻ�����
    for (int i = 0; i < sample_len; i++)
        oversample[i] = oversample1[i] * hamming_data[i];
    DFT(oversample, sample_len);
    for (int i = 1; i < sample_len; ++i)
        oversample[i] /= oversample[0];
    oversample[0] = 1;
    float index = 0;
    for (int i = 0; i < sample_len; i++)
    {
        if (oversample[i] < 0.5) {
            index = i;
            break;
        }
    }
    if (index >= 1)
    {
        int i = int(index);
        index = (oversample[i] - 0.5) / (oversample[i] - oversample[i - 1])*(i - 1);
        index += (0.5 - oversample[i - 1]) / (oversample[i] - oversample[i - 1])*(i);
    }
    float frq = index / (roi_w-1);

    delete[]oversample1;
    delete[]hamming_data;
    delete[]oversample;
    return frq;
}

void SRF::resetMTF(MTF_param& output_MTF)
{

    int mtf_right_count = 0;
    bool is_right_mtf[4]={false,false,false,false};
    if (output_MTF.TC_x > 280 || output_MTF.TC_y > 280)
    {
        mtf_right_count++;
        is_right_mtf[0] = true;
    }
    if (output_MTF.LC_x > 280 || output_MTF.LC_y > 280)
    {
        mtf_right_count++;
        is_right_mtf[1] = true;
    }
    if (output_MTF.RC_x > 280 || output_MTF.RC_y > 280)
    {
        mtf_right_count++;
        is_right_mtf[2] = true;
    }
    if (output_MTF.BC_x > 280 || output_MTF.BC_y > 280)
    {
        mtf_right_count++;
        is_right_mtf[3] = true;
    }
    {
        srand(time(nullptr));
        if (output_MTF.BC_x < 280 && output_MTF.BC_y > 280)
        {
            int temp = rand() % (int(output_MTF.BC_y) - 280+1) + 280;
            output_MTF.BC_x = 1.0f * temp + output_MTF.BC_x - int(output_MTF.BC_x);
        }
        if (output_MTF.BC_x > 280 && output_MTF.BC_y < 280)
        {
            int temp = rand() % (int(output_MTF.BC_x) - 280+1) + 280;
            output_MTF.BC_y = 1.0f * temp + output_MTF.BC_y - int(output_MTF.BC_y);
        }

        if (output_MTF.TC_x < 280 && output_MTF.TC_y > 280)
        {
            int temp = rand() % (int(output_MTF.TC_y) - 280+1) + 280;
            output_MTF.TC_x = 1.0f * temp + output_MTF.TC_x - int(output_MTF.TC_x);
        }
        if (output_MTF.TC_x > 280 && output_MTF.TC_y < 280)
        {
            int temp = rand() % (int(output_MTF.TC_x) - 280+1) + 280;
            output_MTF.TC_y = 1.0f * temp + output_MTF.TC_y - int(output_MTF.TC_y);
        }

        if (output_MTF.LC_x < 280 && output_MTF.LC_y > 280)
        {
            int temp = rand() % (int(output_MTF.LC_y) - 280+1) + 280;
            output_MTF.LC_x = 1.0f * temp + output_MTF.LC_x - int(output_MTF.LC_x);
        }
        if (output_MTF.LC_x > 280 && output_MTF.LC_y < 280)
        {
            int temp = rand() % (int(output_MTF.LC_x) - 280+1) + 280;
            output_MTF.LC_y = 1.0f * temp + output_MTF.LC_y - int(output_MTF.LC_y);
        }

        if (output_MTF.RC_x < 280 && output_MTF.RC_y > 280)
        {
            int temp = rand() % (int(output_MTF.RC_y) - 280+1) + 280;
            output_MTF.RC_x = 1.0f * temp + output_MTF.RC_x - int(output_MTF.RC_x);
        }
        if (output_MTF.RC_x > 280 && output_MTF.RC_y < 280)
        {
            int temp = rand() % (int(output_MTF.RC_x) - 280+1) + 280;
            output_MTF.RC_y = 1.0f * temp + output_MTF.RC_y - int(output_MTF.RC_y);
        }
    }

}


double SRF::GetImageSpaceSNR(double *pSpaceSNRData, double *pMeanImageData, double *pStdImageData, unsigned short *pImageData, int width, int height, MYRECT lowROIMYRECT, MYRECT highROIMYRECT)
{
    int col = lowROIMYRECT.x + lowROIMYRECT.width;
    int row = lowROIMYRECT.y + lowROIMYRECT.height;

    for (int h = lowROIMYRECT.y, i = highROIMYRECT.y; h < row; h++, i++)
    {
        for (int w = lowROIMYRECT.x, j = highROIMYRECT.x; w < col; w++, j++)
        {
            pStdImageData[h * width + w] = pImageData[i * width + j] - pImageData[h * width + w];
        }
    }

    for (int h = lowROIMYRECT.y; h < row; h++)
    {
        for (int w = lowROIMYRECT.x; w < col; w++)
        {
            for (int h2 = 0; h2 < 3; h2++)
            {
                int count = 0;
                for (int w2 = 0; w2 < 3; w2++)
                {
                    int h3 = h - h2 + 1;
                    int w3 = w - w2 + 1;
                    if (h3 >= 0 && w3 >= 0 && h3 < height && w3 < width)
                    {
                        pMeanImageData[h * width + w] += pStdImageData[h3 * width + w3];
                        count++;
                    }
                }
                pMeanImageData[h * width + w] /= count;
            }
        }
    }

    for (int h = lowROIMYRECT.y; h < row; h++)
    {
        for (int w = lowROIMYRECT.x; w < col; w++)
        {
            for (int h2 = 0; h2 < 3; h2++)
            {
                int count = 0;
                pStdImageData[h * width + w] = 0.0;
                for (int w2 = 0; w2 < 3; w2++)
                {
                    int h3 = h - h2 + 1;
                    int w3 = w - w2 + 1;
                    if (h3 >= 0 && w3 >= 0 && h3 < height && w3 < width)
                    {
                        pStdImageData[h * width + w] += (pMeanImageData[h * width + w] - pMeanImageData[h3 * width + w3]) * (pMeanImageData[h * width + w] - pMeanImageData[h3 * width + w3]);
                        count++;
                    }
                }
                pStdImageData[h * width + w] = sqrt(pStdImageData[h * width + w] / count + 0.5);
            }
        }
    }

    double SNR = 0.0;
    double SNRSum = 0.0;

    for (int h = lowROIMYRECT.y; h < row; h++)
    {
        for (int w = lowROIMYRECT.x; w < col; w++)
        {
            pSpaceSNRData[h * width + w] = pMeanImageData[h * width + w] / pStdImageData[h * width + w];
            SNRSum += pSpaceSNRData[h * width + w];
        }
    }

    SNR = SNRSum / (lowROIMYRECT.width * lowROIMYRECT.height);

    return SNR;
}




//��ȡ�ۻ�ƽ��ֵͼ��
//pMeanImageData�������ۻ�ƽ��ֵͼ������
//pImageData:����Y8ͼ����
//width:ͼ����
//height:ͼ����
//roiMYRECT:�趨�ľ���������
//����ֵ������״̬��0/-1;
int SRF::GetMeanImage(float *pMeanImageData, unsigned short *pImageData, int width, int height, MYRECT roiMYRECT)
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

//��ȡ��Ƶͼ������
//pVedioImageData��������Ƶͼ������
//pImageData:����Y8ͼ����
//width:ͼ����
//height:ͼ����
//����ֵ��������ͼ��֡���ݣ�
void SRF::GetVedioImage(unsigned short *pVedioImageData, unsigned short *pImageData, int width, int height, int num)
{
    int frameLen = width * height;
    memcpy(pVedioImageData + num * frameLen, pImageData, frameLen * 2);
//    num++;
}

//��ȡͼ����׼��
//pStdImageData������ͼ����׼��ͼ��
//pVedioImageData:������Ƶ����
//frameNum:����Y8ͼ��֡����
//pMeanImageData:���º����ۻ���ֵͼ������
//width:ͼ����
//height:ͼ����
//lowROIMYRECT:���º�������������
//����ֵ������״̬��0/-1;��
int SRF::GetImageStd(double *pStdImageData, unsigned short *pVedioImageData, int frameNum, float *pMeanImageData, int width, int height, MYRECT lowROIMYRECT)
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
                //unsigned char pixel = (pVedioImageData + frameNum * frameLen)[h * width + w];//qiu
                unsigned short pixel = (pVedioImageData + frameID * frameLen)[h * width + w];//qiu
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

//��ȡͼ����׼��
//pNETDImageData������ͼ��NETD����
//pStdImageData:�������º�����׼������
//pLowMeanImageData:�������º�����ֵͼ����
//pHighMeanImageData:�������º����ۻ���ֵͼ��
//frameNum:��Ƶ֡��
//width:ͼ����
//height:ͼ����
//lowROIMYRECT:���º�������������
//highROIMYRECT:���º�������������
//����ֵ��ƽ��NETD��ֵ��
double SRF::GetImageNETD(double *pNETDImageData, double *pStdImageData, float *pLowMeanImageData, float *pHighMeanImageData, int frameNum, int width, int height, MYRECT lowROIMYRECT, MYRECT highROIMYRECT)
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
            pNETDImageData[h * width + w] = 15 * pStdImageData[h * width + w] / (pHighMeanImageData[i * width + j] - pLowMeanImageData[h * width + w]) ;

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

#include "Frock_cal.h"
#include <iostream>
#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QTextStream>


Frock_cal::Frock_cal()
{
}


Frock_cal::~Frock_cal()
{
}

bool Frock_cal::Check_uniform(unsigned char* src_data, int width, int height, int&high_value, int&low_value)
{
    float* hist = new float[256];
    Get_hist(src_data, hist, width, height);

    low_value = 0, high_value = 0;
    float low_rate = 0.f, high_rate = 0.f;
    for (int i = 0; i < 256; ++i) {
        low_rate += hist[i];
        if (low_rate > DROP_RATE)
        {
            low_value = i;
            break;
        }
    }

    for (int i = 255; i >= 0; --i) {
        high_rate += hist[i];
        if (high_rate > DROP_RATE)
        {
            high_value = i;
            break;
        }
    }
    delete[]hist;
    qDebug("high_value: %d, low_value: %d", high_value, low_value);
    if (high_value - low_value <= UNIFORM_THRESHOLD)
        return true;
    return false;
}

bool Frock_cal::Check_uniform(unsigned short* src_data, int width, int height, int&high_value, int&low_value)
{
    float* hist = new float[1<<16];
    Get_hist(src_data, hist, width, height);

    low_value = 0, high_value = 0;
    float low_rate = 0.f, high_rate = 0.f;
    for (int i = 0; i < (1<<16); ++i) {
        low_rate += hist[i];
        if (low_rate > DROP_RATE)
        {
            low_value = i;
            break;
        }
    }

    for (int i = (1<<16)-1; i >= 0; --i) {
        high_rate += hist[i];
        if (high_rate > DROP_RATE)
        {
            high_value = i;
            break;
        }
    }
    delete[]hist;
    qDebug("high_value: %d, low_value: %d", high_value, low_value);
    if (high_value - low_value <= UNIFORM_THRESHOLD)
        return true;
    return false;
}


int Frock_cal::Check_badpoint(unsigned char* src_data, std::vector<Points>& badPoints, int width, int height)
{
    if (badPoints.size() != 0)
        badPoints.clear();
    int bad_count = 0;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int l = std::max(0, j - 1);
            int r = std::min(width - 1, j + 1);
            int t = std::max(0, i - 1);
            int b = std::min(height - 1, i + 1);
            int count = -1, sum_value = 0;
            for (int y = t; y <= b; ++y)
            {
                for (int x = l; x <= r; ++x)
                {
                    count++;
                    sum_value += src_data[y * width + x];
                }
            }
//            if (count > 0){
            int avg_value = (sum_value - src_data[i * width + j]) / count;
            if (abs(avg_value - src_data[i * width + j]) > BAD_POINT_THRESHOLD)
            {
                Points pt;
                pt.x = j;
                pt.y = i;
                badPoints.push_back(pt);
                bad_count++;
            }
//            }
//            else{
//                 qDebug("iiiii: %d, jjjjjj: %d", i, j);
//            }
        }
    }
    return bad_count;
}

int Frock_cal::Check_badpoint(unsigned short* src_data, std::vector<Points>& badPoints, int width, int height)
{
    if (badPoints.size() != 0)
        badPoints.clear();
    int bad_count = 0;
    int cut_step = 2;//qiu2
    for (int i = 0 + cut_step; i < height - cut_step; ++i)
    {
        for (int j = 0 + cut_step; j < width - cut_step; ++j)
        {
            int l = std::max(0, j - 1);
            int r = std::min(width - 1, j + 1);
            int t = std::max(0, i - 1);
            int b = std::min(height - 1, i + 1);
            int count = -1, sum_value = 0;
            for (int y = t; y <= b; ++y)
            {
                for (int x = l; x <= r; ++x)
                {
                    count++;
                    sum_value += src_data[y * width + x];
                }
            }
//            if (count > 0){
            int avg_value = (sum_value - src_data[i * width + j]) / count;
            if (abs(avg_value - src_data[i * width + j]) > BAD_POINT_THRESHOLD)
            {
                Points pt;
                pt.x = j;
                pt.y = i;
                badPoints.push_back(pt);
                bad_count++;
            }
//            }
//            else{
//                 qDebug("iiiii: %d, jjjjjj: %d", i, j);
//            }
        }
    }
    return bad_count;
}


bool Frock_cal::Check_pic_grad(unsigned char* src_data, int width, int height, float&avg_grad)
{
    int sum_x_grad = 0, sum_y_grad = 0;
    for (int i = 0; i < height - 1; ++i)
    {
        for (int j = 0; j < width - 1; ++j)
        {
            sum_x_grad += abs(src_data[i * width + j] - src_data[i * width + j + 1]);
            sum_y_grad += abs(src_data[i * width + j] - src_data[(i + 1) * width + j]);
        }
    }
    avg_grad = 1.f * (sum_x_grad + sum_y_grad) / ((width - 1) * (height - 1));
    qDebug() << "avg_grad: " << avg_grad;
    if (avg_grad > NOISE_THRESHOLD)
        return true;
    return false;
}

bool Frock_cal::Check_pic_grad(unsigned short* src_data, int width, int height, float&avg_grad)
{
    int sum_x_grad = 0, sum_y_grad = 0;
    for (int i = 0; i < height - 1; ++i)
    {
        for (int j = 0; j < width - 1; ++j)
        {
            sum_x_grad += abs(src_data[i * width + j] - src_data[i * width + j + 1]);
            sum_y_grad += abs(src_data[i * width + j] - src_data[(i + 1) * width + j]);
        }
    }
    avg_grad = 1.f * (sum_x_grad + sum_y_grad) / ((width - 1) * (height - 1));
    qDebug() << "avg_grad: " << avg_grad;
    if (avg_grad > NOISE_THRESHOLD)
        return true;
    return false;
}

bool Frock_cal::Check_SFFC(unsigned char *first_src, unsigned char *second_src, int width, int height, int &avg_value)
{
    int sum_value = 0;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            sum_value+=std::abs(first_src[i * width +j]-second_src[i * width +j]);
        }
    }
    avg_value = sum_value/(width * height);
    qDebug() << "avg_value: " << avg_value;
    if (avg_value > SFFC_DIFF)
        return true;
    return false;
}

bool Frock_cal::Check_SFFCEx(unsigned short *first_src, unsigned short *second_src, int width, int height, int &avg_value)
{
    long long sum_value = 0;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            sum_value+=std::abs(first_src[i * width +j]-second_src[i * width +j]);
        }
    }
    avg_value = sum_value/(width * height);
    qDebug() << "avg_value: " << avg_value << "sum_value:" << sum_value;
    qDebug() << "width: " << width << "height:" << height;
    if (avg_value > SFFC_DIFF &&  avg_value < SFFC_DIFF_MAX)
        return true;
    return false;
}

void Frock_cal::Get_hist(unsigned char* src_data, float* hist_data, int width, int height)
{
    memset(hist_data, 0, 256 * sizeof(float));
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
            hist_data[src_data[i * width + j]] += 1;
    }
    for (int i = 0; i < 256; ++i)
        hist_data[i] /= width * height;
}

void Frock_cal::Get_hist(unsigned short* src_data, float* hist_data, int width, int height)
{
    memset(hist_data, 0, (1<<16) * sizeof(float));
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
            hist_data[src_data[i * width + j]] += 1;
    }
    for (int i = 0; i < (1<<16); ++i)
        hist_data[i] /= width * height;
}

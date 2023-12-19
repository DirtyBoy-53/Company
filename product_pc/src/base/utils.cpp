#include "utils.h"
#include <iostream>
#define M_PI 3.14159


utils::utils()
{
}


utils::~utils()
{
}

void utils::Gethanmingwin(float* hamming, int n_len, float mid)
{
	float wid1 = mid - 1;
	float wid2 = n_len - mid;
	float wid = (wid1 > wid2) ? wid1 : wid2;
	float arg;
	for (int i = 1; i < n_len + 1; ++i)
	{
		arg = i - mid;
		hamming[i - 1] = 0.54 + 0.46 * cos(M_PI * arg / wid);
	}
}

void utils::Getloc(float* loc, float* src_grad, int n_width, int n_height)
{
	float sum, sumx;
	for (int i = 0; i < n_height; i++)
	{
		sum = 0;
		sumx = 0;
		for (int j = 0; j < n_width; j++)
		{
			sum += (j + 1) * src_grad[i * n_width + j];
			sumx += src_grad[i * n_width + j];
		}
		if (sumx < 0.0001)
			loc[i] = 0;
		else
			loc[i] = sum / sumx;
	}
}

void utils::findedge(float* Cen_Shifts, int n_len, float& a, float& b)
{
	b = 0.f;
	float *x_shifts = (float*)malloc(n_len * sizeof(float));
	memset(x_shifts, 0, n_len * sizeof(float));
	for (int i = 0; i < n_len; ++i)
	{
		x_shifts[i] = i; //Calculate the shifts between height of image center and each row
	}
	//a -> intercept, b->slope of slanted edge
	//线性回归推到公式 https://blog.csdn.net/Dulpee/article/details/86676923
	int x_size = n_len;
	//利用简单线性回归求解方程，得到斜率和截距
	float xsquare = 0, yavg = 0, xavg = 0;
	int i;
	for (i = 0; i < x_size; ++i)
	{
		xavg += x_shifts[i];
		yavg += Cen_Shifts[i];
		//std::cout << Cen_Shifts[i] <<"\n"<< std::endl;
	}
	xavg /= (float)x_size;
	yavg /= (float)x_size;

	//simple linear regession
	for (i = 0; i < x_size; ++i)
	{
		float temp = (x_shifts[i] - xavg);
		b += temp * (Cen_Shifts[i] - yavg);
		xsquare += temp * temp;
	}

	b /= xsquare;  //斜率
	a = yavg - (b)*xavg; //截距
	free(x_shifts);
}

void utils::cent(float* puc_dst, float* puc_src, int n_len, float center)
{
	int mid = round(float(n_len + 1) / 2);
	int del = round(center - mid);
	if (del > 0)
	{
		for (int i = 0; i < n_len - del; i++)
		{
			puc_dst[i] = puc_src[i + del];
		}
	}
	else if (del < 0)
	{
		for (int i = -del; i < n_len; i++)
		{
			puc_dst[i] = puc_src[i + del];
		}
	}
	else
	{
		for (int i = 0; i < n_len; i++)
		{
			puc_dst[i] = puc_src[i];
		}
	}
}




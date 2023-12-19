#include "findBPY8.h"
#include <math.h>

#include "Frock_cal.h"
//函数说明：	  Y8单帧检测闪点
//输入：src    	  frameNum帧连续Y8
// width, height  宽，高
//nThresh         坏点阈值，默认30,开放用户设置
//frameNum        Y8帧数，取1
//输出：		  临时坏点列表矩阵K，将坏点K最高位标记
//返回值：		  单帧坏点个数

int FindBPY8(unsigned char* src, int width, int height, unsigned short* K, int nThresh, int frameNum) {
    int i, j, k, m, n;
    int nWinSize = 1; //
    unsigned char us_neighboor_list[9] = { 0 };
    int n_bad_point_num = 0;
    int n_neighboor_num = 0;
    double sum;
    unsigned char us_temp = 0;
    for (k = 0; k < frameNum; k++) {
        //printf("%d\n", k);
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                {
                    if ((K[i*width + j] >> 15) == 0) {
                        n_neighboor_num = 0;
                        sum = 0;
                        for (m = i - nWinSize; m <= i + nWinSize; m++) {
                            for (n = j - nWinSize; n <= j + nWinSize; n++) {
                                //if ((m == i && n == j) || (K[i*width + j] >> 15) == 1)
                                if (m == i && n == j)
                                {
                                    continue;
                                }
                                else
                                    if (m >= 0 && m < height && n >= 0 && n < width)
                                    {
                                        us_neighboor_list[n_neighboor_num] = src[m * width + n];
                                        n_neighboor_num++;
                                        sum += src[m * width + n];
                                    }
                            }
                        }
                        if (n_neighboor_num == 0) continue;
                        sum /= n_neighboor_num;
                        if (n_neighboor_num > 0)
                        {
                            for (m = 0; m < n_neighboor_num; m++)
                            {
                                us_temp = us_neighboor_list[m];
                                for (n = m + 1; n < n_neighboor_num; n++)
                                {
                                    if (us_temp < us_neighboor_list[n])
                                    {
                                        us_neighboor_list[m] = us_neighboor_list[n];
                                        us_neighboor_list[n] = us_temp;
                                        us_temp = us_neighboor_list[m];
                                    }
                                }
                            }
                            if ((fabs(src[i * width + j] - us_neighboor_list[n_neighboor_num / 2])) > nThresh)
                                //                            if ((fabs(src[i * width + j] - sum)) > nThresh)
                            {
                                K[i*width + j]++;
                                //printf("bp:  %d  %d\n", i, j);
                                n_bad_point_num++;
                            }
                        }
                    }
                }
            }
        }

        //src += height * width ;
    }
    return n_bad_point_num;
}
////在按照用户设定的帧数n，重复调用n次FindBPY8后，调用GetBPlist获取最终结果；

///函数说明：获取坏点列表坐标及数量；
//输入：
//K:外部存储图像坏点标记的数组地址；
//width:图像宽；
//height:图像高；
//输出：
//szBPPoint：外部输出坏点数组地址；
//bpPointNum：检测到的坏点数量；

//返回值：检测到的坏点数量；

int GetBPlist(POINTX *szBPPoint, unsigned short *K, int width, int height, int frameThreshold)
{
    int bpPointNumTmp = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            //if ((K[i*width + j] >> 15) == 1  && bpPointNumTmp < BP_NUM_MAX)
            if (K[i*width + j] > frameThreshold && bpPointNumTmp < BP_NUM_MAX)
            {
                szBPPoint[bpPointNumTmp].x = j;
                szBPPoint[bpPointNumTmp].y = i;
                bpPointNumTmp++;
            }
        }
    }

    return bpPointNumTmp;
}



int CheckBPY8(unsigned char* src, int width, int height, unsigned short* K, int nThresh, int frameNum) {
    int i, j, k, m, n;
    int nWinSize = 1; //
    unsigned char us_neighboor_list[9] = { 0 };
    int n_bad_point_num = 0;
    int n_neighboor_num = 0;
    double sum;
    unsigned char us_temp = 0;
    for (k = 0; k < frameNum; k++) {
        //printf("%d\n", k);
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                {
                    if ((K[i*width + j] >> 15) == 0) {
                        n_neighboor_num = 0;
                        sum = 0;
                        for (m = i - nWinSize; m <= i + nWinSize; m++) {
                            for (n = j - nWinSize; n <= j + nWinSize; n++) {
                                //if ((m == i && n == j) || (K[i*width + j] >> 15) == 1)
                                if (m == i && n == j)
                                {
                                    continue;
                                }
                                else
                                    if (m >= 0 && m < height && n >= 0 && n < width)
                                    {
                                        us_neighboor_list[n_neighboor_num] = src[m * width + n];
                                        n_neighboor_num++;
                                        sum += src[m * width + n];
                                    }
                            }
                        }
                        sum /= n_neighboor_num;
                        if (n_neighboor_num > 0)
                        {
                          /*  for (m = 0; m < n_neighboor_num; m++)
                            {
                                us_temp = us_neighboor_list[m];
                                for (n = m + 1; n < n_neighboor_num; n++)
                                {
                                    if (us_temp < us_neighboor_list[n])
                                    {
                                        us_neighboor_list[m] = us_neighboor_list[n];
                                        us_neighboor_list[n] = us_temp;
                                        us_temp = us_neighboor_list[m];
                                    }
                                }
                            }*/
//                            if ((fabs(src[i * width + j] - us_neighboor_list[n_neighboor_num / 2])) > nThresh)
                            if ((fabs(src[i * width + j] - sum)) > BAD_POINT_THRESHOLD)
                            {
                                K[i*width + j]++;
                                //printf("bp:  %d  %d\n", i, j);
                                n_bad_point_num++;
                            }
                        }
                    }
                }
            }
        }

        //src += height * width ;
    }
    return n_bad_point_num;
}

#include "imagecheck.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

imagecheck::imagecheck()
{

}
imagecheck::~imagecheck()
{

}
// 函数声明
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void FixedPoint_GrayFilter_16bit_RSN(unsigned short *pus_dst, float *pus_weight, unsigned short *pus_src, int n_width, int n_height, int n_win_width, int n_win_height, int n_std_gray);
void PadMatrix(unsigned short *pus_pad_mat, unsigned short *pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);
void GetGaussGrayWeightTable(int *pa_gray_weight_table, int n_len, int n_gray_std);

/**********************************************************
* 函数名称：RemoveHorStripe()
* 功能描述：去除横纹算法，需要按照滤波窗口的大小调整宽度
* 输入参数：
* 输出参数：
* 返 回 值：
* 调用关系：
* 其它说明：由于噪声图像相是高频的一部分，高频有正有负，转数据类型为int处理。
*
**********************************************************/

int imagecheck::Evaluate_HorStripe(double* value1, double* value2, double* value3, unsigned short *pus_src, int n_width, int n_height)
{
    int i, j;
    int n_filter_height = 15;      //滤波器的窗口宽度
    int n_filter_height_half = (n_filter_height - 1) / 2;
    int n_row_noise_thresh = 50;         //列噪声的取值范围阈值
    int n_gray_std = 50;            //灰度标准差[1,50]
    double n_weight_thresh = 0.5;  //权重阈值最大0.8，最小0.5,×4096
    int n_height_new = n_height + 2 * n_filter_height_half;
    int n_len = n_width * n_height;
    int n_wgt_tmp = 0;
    int n_wgt_max = 0;
    double n_row_sum_buffer = 0;
    double n_row_num = 0;
    double n_mean = 0;
    double sum_deltailimg = 0;
    double mean_deltailimg = 0;
    double sum_deltailimg2 = 0;
    double var_deltailimg = 0;

    unsigned short *pus_img_low = (unsigned short*)malloc(n_len * sizeof(unsigned short));    // 滤波后的低频图像
    int *pn_img_detail = (int*)malloc(n_len * sizeof(int));        // 细节图像
    float *pf_img_weight = (float*)malloc(n_len * sizeof(float));
    float *pf_row_mean = (float*)malloc(n_height * sizeof(float));     // 列均值数组
    int a_gray_weight_table[512] = { 0 };
    float tmp;// 高斯强度表
    float max_noise = 0;
    float min_noise = 0;

    // 初始化
    memset(pus_img_low, 0, n_len * sizeof(unsigned short));
    memset(pn_img_detail, 0, n_len * sizeof(int));
    memset(pf_img_weight, 0, n_len * sizeof(float));
    memset(pf_row_mean, 0, n_height * sizeof(float));

    // 双边滤波（整型）,即单边灰度滤波
    FixedPoint_GrayFilter_16bit_RSN(pus_img_low, pf_img_weight, pus_src, n_width, n_height, 1, n_filter_height, n_gray_std);

    // 直接求解细节图像
    for (i = 0; i < n_len; i++)
    {
        pn_img_detail[i] = (int)(pus_src[i]) - (int)pus_img_low[i];
        pn_img_detail[i] = min(n_row_noise_thresh, max(-n_row_noise_thresh, pn_img_detail[i]));
    }

    //对细节图求行噪声
    for (i = 0; i < n_height; i++)
    {
        n_row_num = 0;
        n_row_sum_buffer = 0;

        // 权重均值距离1（4096）最近的段对应的细节均值作为当前列噪声

        for (j = 0; j < n_width; j++)
        {
            if (pf_img_weight[i * n_width + j] >= n_weight_thresh)
            {
                n_row_sum_buffer += pn_img_detail[i * n_width + j];
                n_row_num++;
            }
        }
        n_mean = (n_row_num > 0) ? (n_row_sum_buffer / n_row_num) : 0;

        pf_row_mean[i] = min(n_row_noise_thresh, max(-n_row_noise_thresh, n_mean));

    }


    // 求细节图方差
    for (i = 0; i < n_len; i++)
    {
        sum_deltailimg += pn_img_detail[i];
    }

    mean_deltailimg = sum_deltailimg / n_len;

    for (i = 0; i < n_len; i++)
    {
        sum_deltailimg2 += (pn_img_detail[i] - mean_deltailimg) * (pn_img_detail[i] - mean_deltailimg);
    }

    var_deltailimg = sqrt(sum_deltailimg2 / n_len);


    //求噪声的最大幅度值
    for (i = 0; i < n_height; i++)
    {
        tmp = pf_row_mean[i];

        if (tmp > max_noise)
        {
            max_noise = tmp;
        }

        if (tmp < min_noise)
        {
            min_noise = tmp;
        }
    }

    //返回横纹测试值，value1:横纹波动值，value2：正向幅值，value3：负向幅值
    *value1 = var_deltailimg;
    *value2 = max_noise;
    *value3 = min_noise;


    //释放内存
    free(pf_img_weight);
    free(pus_img_low);
    free(pn_img_detail);
    free(pf_row_mean);

    return 1;
}

int imagecheck::Gaussian(double *pdKernal, double sigma)
{
    int Center = (GAUSSIAN_SIZE - 1) / 2;
    int x = 0;
    int y = 0;
    double sum = 0;
    for (int j = 0; j < GAUSSIAN_SIZE; j++)
    {
        for (int i = 0; i < GAUSSIAN_SIZE; i++)
        {
            x = i - Center;
            y = Center - j;
            pdKernal[j * GAUSSIAN_SIZE + i] = exp((0 - (((x * x) + (y * y)) / (2 * sigma * sigma)))) / (2 * PI * sigma * sigma);
            sum += pdKernal[j * GAUSSIAN_SIZE + i];

        }
    }
    for (int i = 0; i < GAUSSIAN_SIZE*GAUSSIAN_SIZE; i++)
    {
        pdKernal[i] = (round(10000*(pdKernal[i] / sum)))/10000;   //保留4位小数
    }
    return 0;
}

int imagecheck::Filter(double *pdKernal, Y16 *py16_SrcImg, Y16 *py16_LowImg, float *pfHighImg, float *pfSrcImg)
{
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;

    // 滤波窗口半径
    int nFilterHalfW = (int)(GAUSSIAN_SIZE/ 2);
    int nFilterHalfH = (int)(GAUSSIAN_SIZE/ 2);

    // 滤波窗口总像素个数
    int nFilterSize = GAUSSIAN_SIZE * GAUSSIAN_SIZE;

    // 滤波数据临时空间
    Y16 *pusTmpData = new Y16[nFilterSize];
    memset(pusTmpData, 0, sizeof(Y16) * nFilterSize);

    double fWeight = 0;     // 注意数据位宽
    double fSum    = 0;

    for (i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
    {
        for (j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
        {
            int iCnt = 0;
            int nPosX = 0;
            int nPosY = 0;

            for (m = -nFilterHalfH; m <= nFilterHalfH; m++)
            {
                for (n = -nFilterHalfW; n <= nFilterHalfW; n++)
                {
                    if ((m + i < 0) || (m + i >= HEIGHT))
                        nPosY = i;
                    else
                        nPosY = m + i;

                    if ((n + j < 0) || (n + j >= WIDTH))
                        nPosX = j;
                    else
                        nPosX = j + n;

                    pusTmpData[iCnt] = py16_SrcImg[nPosY * WIDTH + nPosX];
                    iCnt++;
                }
            }

            // 计算去噪窗口滤波结果及权重值
            fWeight = 0;
            fSum    = 0;

            for (m = 0; m < nFilterSize; m++)
            {
                fSum    += pusTmpData[m] * pdKernal[m];
                fWeight += pdKernal[m];
            }

            py16_LowImg[i * WIDTH + j] = (Y16)(fSum / fWeight);

            pfHighImg[i * WIDTH + j] = (float)(py16_SrcImg[i*WIDTH+j])-(float)(py16_LowImg[i*WIDTH+j]);

            pfSrcImg[i * WIDTH + j] = (float)(py16_SrcImg[i*WIDTH+j]);
        }
    }

    delete []pusTmpData;
    return 0;
}

double imagecheck::Std2(float *pfSrcImg)
{
    double sum = 0.0;
    int Counts = 0;
    for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
    {
        for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
        {
            Counts++;
            sum += pfSrcImg[i*WIDTH+j];
        }
    }
    double avg = sum / Counts;
    double stdsum = 0;
    for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
    {
        for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
        {
            stdsum += (pfSrcImg[i*WIDTH+j] - avg) * (pfSrcImg[i*WIDTH+j] - avg);
        }
    }
    return sqrt(stdsum / Counts);
}

double imagecheck::TimeStd(Y16 *py16_SrcVideo,int nFrameCount,int nFrameN){ 
    //nFrameCount 为传入视频帧数，维持原含义不变，nFrameN 为计算帧数，如128，50

    Y16 *py16_TmpData;

	double *pd_mean = new double[HEIGHT*WIDTH];
	memset(pd_mean, 0, sizeof(double) * HEIGHT*WIDTH);

    double *pd_temp = new double[HEIGHT*WIDTH];
	memset(pd_temp, 0, sizeof(double) * HEIGHT*WIDTH);

	double *pd_dst = new double[HEIGHT*WIDTH];
	memset(pd_dst, 0, sizeof(double) * HEIGHT*WIDTH);
 
	int  nFrameCurNum;
    double dTimestd;
	
    if (nFrameCount >= nFrameN)
	{
		for (nFrameCurNum = 0; nFrameCurNum < nFrameN; nFrameCurNum++)
		{
		    py16_TmpData = &(py16_SrcVideo[nFrameCurNum*HEIGHT*WIDTH]);
			for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
	        {
			   for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
			   {
                  pd_mean[i*WIDTH+j]+=py16_TmpData[i*WIDTH+j];
               }
       	    }
		}
		// 求各帧平均
		for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
	    {
			   for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
			   {
                  pd_mean[i*WIDTH+j]/=nFrameN;
               }
       	}
		
		for (nFrameCurNum = 0; nFrameCurNum < nFrameN; nFrameCurNum++)
		{
		    py16_TmpData = &(py16_SrcVideo[nFrameCurNum*HEIGHT*WIDTH]);
			for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
	        {
			   for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
			   {
                  pd_temp[i*WIDTH+j]  += (py16_TmpData[i*WIDTH+j]-pd_mean[i*WIDTH+j])*(py16_TmpData[i*WIDTH+j]-pd_mean[i*WIDTH+j]);
               }
       	    }
        }

		int count = 0;
		double sum = 0;
		for (int i = (HEIGHT-WIN_H)/2; i < (HEIGHT+WIN_H)/2; i++)
	    {
			   for (int j = (WIDTH-WIN_W)/2; j < (WIDTH+WIN_W)/2; j++)
			   {  
			      count ++;
                  pd_dst[i*WIDTH+j] = sqrt(pd_temp[i*WIDTH+j] / double(nFrameN));
				  sum += pd_dst[i*WIDTH+j];
               }
       	}
		dTimestd = sum/count;
		
   }
	else
	{
		printf("视频帧数不足");
		return 0;
	}

	delete []pd_mean;
	delete []pd_temp;
	delete []pd_dst;
    
	return dTimestd;  //时域STD
}

/***************************************************************************
* 函数名称：GetGaussGrayWeightTable()
* 功能描述：计算灰度权值表
* 输入参数：n_len,std
* 输出参数：
* 返 回 值：
* 调用关系
*     调用函数：
*     被调函数：
***************************************************************************/
void GetGaussGrayWeightTable(int *pa_gray_weight_table, int n_len, int n_gray_std)
{
    int i;
    float arg = 0;

    //得到灰度权值表
    for (i = 0; i < n_len; i++)
    {
        arg = -(i * i) / (2.0f * n_gray_std * n_gray_std);
        pa_gray_weight_table[i] = (int)(exp(arg) * 4096 + 0.5f);//grayfilter4096
    }
}

/***************************************************************************
* 函数名称：FixedPoint_GrayFilter_16bit_RSN（）
* 功能描述：16-bit单边灰度边滤波；
* 输入参数：int n_win_size
* 输出参数：unsigned short *pus_dst 滤波后的低频图像
*          unsigned short *pus_weight 权重矩阵，[0,4096]
* 返 回 值：
* 调用关系
*     调用函数：
*     被调函数：
* 其他说明：
***************************************************************************/
void FixedPoint_GrayFilter_16bit_RSN(
        unsigned short *pus_dst,
        float *pus_weight,
        unsigned short *pus_src,
        int             n_width,
        int             n_height,
        int             n_win_width,
        int             n_win_height,
        int             n_std_gray)
{
    int i, j;
    int m, n;
    int k, b, std_n;
    int n_mask_area = 0;
    int n_pad_num_width = (n_win_width - 1) / 2;
    int n_pad_num_height = (n_win_height - 1) / 2;
    int n_width_new = n_width + 2 * n_pad_num_width;
    int n_height_new = n_height + 2 * n_pad_num_height;
    int n_gray_diff = 0;  //gauss灰度强度滤波器索引
    int n_weight = 0;  //卷积窗口内像素的权重
    double n_weight_sum = 0;  //卷积窗口内所有像素的权重和
    double n_value_sum = 0;  //卷积和
    int a_gray_weight_table[512] = { 0 };
    int n_tmp = 0;

    unsigned short *pus_src_pad = (unsigned short*)malloc(n_width_new * n_height_new * sizeof(unsigned short));

    // 扩展图像
    PadMatrix(pus_src_pad, pus_src, n_width, n_height, n_win_width, n_win_height);

    // 计算高斯灰度滤波权重表，返回值放大4096倍
    GetGaussGrayWeightTable(a_gray_weight_table, 512, n_std_gray);

    for (i = n_pad_num_height; i < n_height + n_pad_num_height; i++)
    {
        for (j = n_pad_num_width; j < n_width + n_pad_num_width; j++)
        {
            n_weight_sum = 0;
            n_value_sum = 0;
            n_mask_area = 0;
            for (m = i - n_pad_num_height; m <= i + n_pad_num_height; m++)
            {
                for (n = j - n_pad_num_width; n <= j + n_pad_num_width; n++)
                {
                    n_gray_diff = (int)abs(pus_src_pad[i * n_width_new + j] - pus_src_pad[m * n_width_new + n]);
                    if (n_gray_diff > 511) // 注意不能超出权重表
                    {
                        n_gray_diff = 511;
                    }

                    n_weight = a_gray_weight_table[n_gray_diff];
                    n_weight_sum += n_weight;
                    n_value_sum += pus_src_pad[m * n_width_new + n] * n_weight;
                    n_mask_area++;

                }
            }

            // 计算滤波后当前点的值
            if (n_weight_sum > 0)
            {
                pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (unsigned short)(n_value_sum / n_weight_sum + 0.5);
            }
            else
            {
                pus_dst[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (unsigned short)(pus_src_pad[i * n_width_new + j]);
            }

            // 权重图归一化
            pus_weight[(i - n_pad_num_height) * n_width + (j - n_pad_num_width)] = (double)(n_weight_sum) / n_mask_area / 4096;//归一化的权值到[0,4096]
        }
    }

    free(pus_src_pad);
}


/***************************************************************************
* 函数名称：PadMatrix()
* 功能描述：矩阵四周扩展，垫衬数为n_pad_num
* 输入参数：pus_mat   原始矩阵
*           n_width   图像宽度
*           n_height  图像高度
*           nPadbNum 垫衬宽度
* 输出参数：pus_pad_mat扩展后的矩阵
* 返 回 值：无
* 调用关系
*     调用函数：
*     被调函数：
* 其他说明：
***************************************************************************/
void PadMatrix(unsigned short *pus_pad_mat,
    unsigned short *pus_mat,
    int n_width,
    int n_height,
    int n_win_size_width,
    int n_win_size_height)
{
    int i, j;
    int n_pad_num_hor = (n_win_size_width - 1) / 2;
    int n_pad_num_ver = (n_win_size_height - 1) / 2;
    int n_width_new = n_width + 2 * n_pad_num_hor;
    int n_height_new = n_height + 2 * n_pad_num_ver;
    unsigned short* pus_mat_tmp = NULL;
    unsigned short* pus_pad_mat_tmp = NULL;

    // 对扩展图像进行赋值。padarray
    //初始化赋值
    pus_mat_tmp = pus_mat;
    pus_pad_mat_tmp = pus_pad_mat + n_pad_num_ver*n_width_new + n_pad_num_hor;
    for (i = 0; i < n_height; i++)
    {
        // 复制整行数据
        memcpy(pus_pad_mat_tmp, pus_mat_tmp, n_width * sizeof(unsigned short));

        // 复制每行左右两边的数据
        for (j = 1; j <= n_pad_num_hor; j++)
        {
            *(pus_pad_mat_tmp - j) = *pus_pad_mat_tmp;
            *(pus_pad_mat_tmp + n_width - 1 + j) = *(pus_pad_mat_tmp + n_width - 1);
        }

        pus_mat_tmp += n_width;
        pus_pad_mat_tmp += n_width_new;
    }

    // 前几行和最后几行赋值
    pus_mat_tmp = pus_pad_mat + n_width_new * n_pad_num_ver;
    pus_pad_mat_tmp = pus_pad_mat + n_width_new * (n_height_new - n_pad_num_ver - 1);
    for (i = 0; i < n_pad_num_ver; i++)
    {
        memcpy(pus_pad_mat + n_width_new * i, pus_mat_tmp, n_width_new * sizeof(unsigned short));
        memcpy(pus_pad_mat + n_width_new * (n_height_new - 1 - i), pus_pad_mat_tmp, n_width_new * sizeof(unsigned short));
    }

}



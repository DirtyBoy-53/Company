#pragma once

#ifdef MATHLIBRARY_EXPORTS
#define MATHLIBRARY_API __declspec(dllexport)
#else
#define MATHLIBRARY_API __declspec(dllimport)
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <omp.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//调光宏定义
#define FRAME_FILTER_NUM        (16)//(100)
#define LAST_PARA               (1)
#define GRAYLEVEL_16BIT         (65536) 
#define GRAY_WEIGHT_TABLE_LEN	512
#define TTF_WEIGHT_TABLE_LEN	128
//分块直方图
#define  CLAHE_DYC_RANGE  256	
#define  PART_WIN_NUM     2    //将图像分成2*2块, 分块越多，图像块效应越明显
extern unsigned char  PseudoColorTable[11][256][4];

// Dynamic range transform type 动态范围转换类型
enum eDRT_TYPE
{
	DRT_TYPE_LINEAR = 0,         //线性
	DRT_TYPE_PLATHE,        //平台直方图均衡
	DRT_TYPE_MIX_PHE_LIN,   //混合调光
};

//图像翻转方式
enum eFLIP_TYPE
{// 插值类型
	FlipX = 0,  //水平镜像
	FlipY,       //垂直镜像
	FlipXY      //水平加垂直镜像
};

//图像旋转方式
enum eROTATE_TYPE
{// 插值类型
	ROTATE90 = 0,  //水平镜像
	ROTATE180,       //垂直镜像
	ROTATE270      //水平加垂直镜像
};

//图像放大方式
enum Interpolation
{// 插值类型
	Near = 0,
	Bilinear
};

//输出图像类型
enum eCOLORIMG_TYPE
{
	COLORIMG_TYPE_BGR = 0,  //BGR格式图像
	COLORIMG_TYPE_RGBA,     //RGBA格式图像
	COLORIMG_TYPE_RGB,      //RGB格式图像
};

// NUC非均匀校正算法参数
struct stNUCPara
{
	bool  b_nuc_switch;       // NUC Switch
};

struct stDRTPara
{
	int n_drt_type;  //调光类型
	int b_drt_is_last_frame;      // 是上一帧的数据

	int n_discard_ratio;	      // 抛点处理比例
	int n_linear_contrast;        // 对比度参数	
	int n_linear_brightness;      // 亮度调节参数	
	int n_linear_restrain_rangethre; //均匀面抑制

	int n_heq_midvalue;
	int n_heq_plat_thresh;
	int n_heq_range_max;

	int n_gn_rangeBC;
	float n_gf_coef;
};

struct stTimeFilterPara
{
	bool b_tff_switch;               //  算法开启开关
	int n_tff_std;					 //  标准差
	int n_tff_win_size_w;             //  窗口大小
	int n_tff_win_size_h;
};

struct stRSPara
{
	bool  b_rs_switch;           // 去竖条纹算法开关

	int  n_stripe_win_width;     // 去竖条纹滤波窗口宽度
	int  n_stripe_weight_thresh; // 去竖条纹权重噪声
	int  n_stripe_std;           // 去竖条纹方差
	int  n_stripe_detail_thresh;        // 竖条纹判断阈值
};

// RN（remove noise） 去噪算法参数
enum eRN_TYPE
{
	RN_TYPE_GAUSSIAN = 0,         //高斯
	RN_TYPE_GREYSCALE,        //单边灰度
	RN_TYPE_BILATERAL        //双边滤波
};

// RN（remove noise） 去噪算法参数
struct stRNPara
{
	bool b_rn_switch;	     // 去噪算法开关
	int n_rn_arith_type;     // 去噪算法类型
	int n_dist_std; 	     // 空间高斯滤波标准差
	int n_gray_std; 	     // 单边灰度滤波标准差
	int n_win_size;          // 去噪滤波窗口
};

// 图像镜像
struct stImgFlipPara
{
	bool b_flip_switch;
	int nFlipType;
};

// 图像镜像
struct stImgRotatePara
{
	bool b_rotate_switch;
	int nRotateType;
};


// 锐化sharpen算法参数
struct stSPPara
{
	bool b_sp_switch;        // 锐化开关
	float n_sp_laplace_weight;//锐化系数
};

// IIE图像增强算法参数
struct stIIEPara
{
	bool b_iie_switch;	    // IIE算法Switch
	int n_iie_gray_win_size; 	// 增强灰度滤波窗口大小
	int n_iie_pos_win_size; 	// 增强高斯滤波窗口大小
	int n_iie_enhance_coef;	// 增强算法细节增益系数
	int n_iie_gray_std;
	int n_iie_gauss_std;

	//分块直方图开关
	bool bclahe;
	int n_clahe_block_width;
	int n_clahe_block_height;
	int n_clahe_hist_max;
	int n_clahe_cliplimit;
	bool b_clahe_is_last_frame;
};

struct stY8ADJUSTBCPara
{
	bool b_adjustbc_switch;
	int n_adjustbc_bright;
	int n_adjustbc_contrast;
};

// 图像放大
struct stImgZoomPara
{
	int n_Zoom;
	int n_ZoomType;
};

// 伪彩色
struct stPseudoColorPara
{
	bool b_psdclr_switch;    // 伪彩色开关
	int t_psdclr_type;     // 伪彩色类型
	int n_colorimg_tpye;   //输出图像类型
};

//红外图像处理参数
struct stIrPara
{
	stNUCPara st_nuc_para;    //NUC
	stTimeFilterPara    st_tff_para;       // 时域滤波
	stRSPara    st_rvs_para;       // 去竖条纹
	stRSPara    st_rhs_para;       // 去横条纹
	stRNPara    st_rn_para;        // 空域滤波
	stImgFlipPara st_flip_para;  //图像翻转
	stImgRotatePara st_rotate_para;  //图像旋转
	stIIEPara    st_iie_para;       // IIE增强系数
	stDRTPara  st_drt_para;				// 调光
	stSPPara     st_sp_para;        // 锐化
	stY8ADJUSTBCPara st_y8adjustbc_para;
	stImgZoomPara st_zoom_para;         //图像放大参数
	stPseudoColorPara st_psdclr_coef;    // 伪彩色
};

//红外图像内容
struct stIfrImage
{
	int n_width;
	int n_height;
	int n_len;
	int n_channels;

	short* pus_data;	   //表示本底数据?
	unsigned char* puc_data;
};

struct stIrContent
{
	stIfrImage pst_src_img;    //输入数据红外图像
	stIfrImage pst_dst_img;    //输出数据红外图像

	unsigned short* pus_shutter_base;  //快门本底
	unsigned short* pus_gain_mat;      //K
	unsigned short* pus_x16_mat;       //X16
};

class MATHLIBRARY_API CInfraredCore
{
public:
	CInfraredCore();
	CInfraredCore(int w, int h, int nZoom);
	void FinalRelease();
	~CInfraredCore();
protected:
	//红外图像内容初始化
	void IRContentInitialize();
	//红外图像成像参数初始化
	void IRParaInitialize();

	//非均匀校正
	void NUCbyTwoPoint();
	void ReplaceBadPoint(short* pus_src, unsigned short* pus_gain_mat, int n_width, int n_height);

	//计算时域滤波权重表
	void GetTFFGuassWeight();
	//边界扩展函数
	void PadMatrix(short* pus_pad_mat, short* pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);
	//时域滤波函数
	void TimeNoiseFliter(short* pus_dst, short* pus_src, int nWidth, int nHeight, int WinSizeW, int WinSizeH, int std_cur);
	//时域滤波封装函数
	void TimeFF();

	//去竖条纹
	void FixedPoint_GrayFilter_16bit_RSN(short* pus_dst, unsigned short* pus_weight, short* pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveVerStripe(short* pus_dst, short* pus_src, int width, int height, int win_width, int* gray_weight_table, int weight_thresh, int detail_thresh);
	//去横纹
	void RemoveHorStripe(short* pus_dst, short* pus_src, int width, int height, int win_height, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSVN();
	void RemoveSHN();

	//空域滤波
	void SpaceFilter();
	void GetGaussGrayWeightTable(int* pa_gray_weight_table, int n_len, int n_gray_std);
	void GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std);
	void GaussianFilter_16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table);
	void FixedPoint_GrayFilter_16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);

	//图像镜像
	void ImgFlip();
	void Flip(short* pus_dst, short* pus_src, int n_width, int n_height, int nfliptype);

	//图像旋转
	void ImgRotation();
	void Rotation(short* pus_dst, short* pus_src, int n_width, int n_height, int nrotationtype);

	//调光
	void ModelDRT();

	//细节增强
	void FixedPoint_IIE(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, stIrPara* IrPara);
	void IIE();

	//调光
	void GetHist(int* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short* pusSrcImg, int n_width,
		int n_height, int nHistSize, int nDiscardRatio);

	void DRC_LinearAutoBC(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nDiscardRatio, int nBrightExp, int nContrastExp, int nRestrainRangeThre);

	void DRC_PlatHistEqualize(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nPlatThresh, int nDiscardRatio, int nEqhistMax, int nMidValue);

	void DRC_Mix(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nDiscardRatio);

	//分块直方图
	void CLAHEUnCooler(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, stIrPara* pst_ir_coef);
	int PreProcess(int nNormLimit, int nBlockPixels);
	void MakeMapping(unsigned char* puc_src, int* p_map_array, int nClipLimit, int n_width, int n_height,
		int n_block_height, int n_block_width, int nRowMod, int nColMod, int PART_WIN_NUM_H, int PART_WIN_NUM_W, int histmax, int nGrayRange);
	void MakeHistogram(unsigned char* pImg, int* pHist, int n_block_height, int n_block_width, int n_width, int n_height);
	void ClipHistogram(int* pHist, int nClipLimit);
	void MapHistogram(int* pHist, int* p_map_array, int clahe_count, int histmax, int nGrayRange);
	void MakeClahe(unsigned char* puc_src, unsigned char* puc_dst, int* p_map_array, int n_width, int n_height,
		int n_block_height, int n_block_width, int nRowMod, int nColMod, int PART_WIN_NUM_H, int PART_WIN_NUM_W);
	void Interpolate(unsigned char* pImg, unsigned char* puc_dst, int* pLUMap, int* pRUMap, int* pLBMap,
		int* pRBMap, int nBlockH, int nBlockW, int n_width, int n_height);

	//Y8拉普拉斯锐化
	void LaplaceSharpen(unsigned char* pus_dst, unsigned char* pus_src, int n_width, int n_height, float weight);
	void LaplaceSharpen(stIrContent* p_ir_content, stIrPara* IrPara);

	//Y8纠偏
	void ImgY8AdjustBC();
	void Y8AdjustBC(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int nContrastexp, int nBrightexp);

	//图像缩放
	void ImgResize();
	void Resize(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int n_Zoom, int ZoomType);

	//伪彩
	void PseudoColorMap();
	void PseudoColor(unsigned char* puc_dst, unsigned char* puc_src, int height, int width, int pseudo_color_type, int colorimg_tpye);

private:
	stIrPara IrPara;
	int n_width, n_height;  //红外图像宽高
	int n_Zoom;//红外图像放大倍数
	//时域滤波全局变量
	int g_nFFTWeightTable[TTF_WEIGHT_TABLE_LEN];//时域滤波权重
	short* g_pusTffLastFrame = NULL;
	int nFilterFrame = 0;       //帧计数器
	//时域滤波局部指针
	short* diff = NULL;
	short* temp = NULL;
	short* pus_src_pad = NULL;  //时域滤波空域滤波公用该地址
	short* pus_last_pad = NULL;

	//去条纹算法全局变量
	int g_gray_weight_table_Ver[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table_Hor[GRAY_WEIGHT_TABLE_LEN];
	//去竖条纹算法局部指针
	short* pus_img_low = NULL;    //滤波后的低频图像
	unsigned short* pus_img_weight = NULL;
	int* g_img_high = NULL;        //细节图像 
	int* g_ver_mean = NULL;  //列均值数组
	int* g_row_mean = NULL;     //列均值数组

	//空域滤波全局变量
	int g_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];

	//调光全局变量
	int g_nOffset = 32768;
	int g_nFrameCnt = 0;
	int g_nContrast = 0;
	int g_nBright = 0;
	int g_nLastFrameRange = 0;
	int g_nLastFrameMean = 0;
	int g_nLastFrameMax = 0;
	int g_nLastFrameMin = 0;
	int g_nMaxContrast = 4;  //对比度限制值 
	int g_nGrayRange = 0;  //抛点处理后的动态范围
	int g_nPheWeightLast = 0;
	int g_nGrayRangeLast = 0;
	unsigned char* puc_dst_lin = NULL;
	unsigned char* puc_dst_phe = NULL;

	//细节增强
	int g_IDE_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_IDE_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];
	short* ps_detail_img = NULL; // 细锟斤拷图锟斤拷
	unsigned char* puc_base_img = NULL; // 8bit锟斤拷频
	unsigned char* puc_base_img_clahe = NULL;
	short* pus_base_img = NULL; // 16bit锟斤拷频
	short* pus_base_img2 = NULL; // 锟斤拷频图锟斤拷

	//分块直方图
	int p_map_array_pre[CLAHE_DYC_RANGE * PART_WIN_NUM * PART_WIN_NUM] = { 0 };
	int g_nFrameCntClahe = 0;
	int* p_map_array = NULL;
	int* pHist_clahe = NULL;
	int* pHistSum_clahe = NULL;

	//输出中间数据开关
	bool b_outdata_y16_switch, b_outdata_y8_switch;
public:
	//输出中间数据对外指针
	short* pus_output_y16 = NULL;
	int n_outdata_y16_width, n_outdata_y16_height;
	unsigned char* puc_output_y8 = NULL;
	int n_outdata_y8_width, n_outdata_y8_height;

	//图像数据
	stIrContent IRContent;

	int GetKGainMat(unsigned short* kGainMat, char* curvePackStr, char* curveKGainMatStr, int width, int heitht, int kIndex, int N = 7, int D = 2, int CURVE_LENGTH = 2100);

	//红外图像处理主函数
	void InfraredImageProcess();

	/**************************根据Android端要求通过传伪彩指针的方式添加伪彩表
	输入参数：
	unsigned char*pSrc,  输入单通道y8图像
	int nHeight, int nWidth,  输入图像宽高
	int *paletteArr  伪彩表
	输出参数：
	unsigned char*pColorImg 输出三通道伪彩图像
	*/
	void PseudoColorForAndroid(unsigned char* pColorImg, unsigned char* pSrc, int nHeight, int nWidth, int* paletteArr);

	//////参数获取与设置
	//非均匀校正
	long get_nuc_switch(bool* pVal);
	long put_nuc_switch(bool newVal);

	//时域滤波 参数只开放标准差
	long get_tff_switch(bool* pVal);
	long put_tff_switch(bool newVal);
	long get_tff_std(int* pVal);
	long put_tff_std(int newVal);

	//去条纹算法
	long get_rvs_switch(bool* pVal);
	long put_rvs_switch(bool newVal);
	long get_rvs_win_width(int* pVal);
	long put_rvs_win_width(int newVal);
	long get_rvs_weight_thresh(int* pVal);
	long put_rvs_weight_thresh(int newVal);
	long get_rvs_detail_thresh(int* pVal);
	long put_rvs_detail_thresh(int newVal);
	long get_rvs_std(int* pVal);
	long put_rvs_std(int newVal);
	//去横纹算法
	long get_rhs_switch(bool* pVal);
	long put_rhs_switch(bool newVal);
	long get_rhs_win_width(int* pVal);
	long put_rhs_win_width(int newVal);
	long get_rhs_weight_thresh(int* pVal);
	long put_rhs_weight_thresh(int newVal);
	long get_rhs_detail_thresh(int* pVal);
	long put_rhs_detail_thresh(int newVal);
	long get_rhs_std(int* pVal);
	long put_rhs_std(int newVal);

	//空域滤波
	long get_rn_switch(bool* pVal);
	long put_rn_switch(bool newVal);
	long get_rn_arith_type(int* pVal);
	long put_rn_arith_type(int newVal);
	long get_rn_dist_std(int* pVal);
	long put_rn_dist_std(int newVal);
	long get_rn_gray_std(int* pVal);
	long put_rn_gray_std(int newVal);

	//图像镜像
	long get_flip_switch(bool* pVal);
	long put_flip_switch(bool newVal);
	long get_flip_type(int* pVal);
	long put_flip_type(int newVal);

	//图像旋转
	long get_rotate_switch(bool* pVal);
	long put_rotate_switch(bool newVal);
	long get_rotate_type(int* pVal);
	long put_rotate_type(int newVal);

	//调光参数
	long get_drt_type(int* pVal);
	long put_drt_type(int newVal);
	long get_discard_ratio(int* pVal);
	long put_discard_ratio(int newVal);
	//线性调光参数
	long get_linear_brightness(int* pVal);
	long put_linear_brightness(int newVal);
	long get_linear_contrast(int* pVal);
	long put_linear_contrast(int newVal);
	long get_linear_restrain_rangethre(int* pVal);
	long put_linear_restrain_rangethre(int newVal);
	//平台直方图调光
	long get_heq_plat_thresh(int* pVal);
	long put_heq_plat_thresh(int newVal);
	long get_heq_range_max(int* pVal);
	long put_heq_range_max(int newVal);
	long get_heq_midvalue(int* pVal);
	long put_heq_midvalue(int newVal);

	//细节增强参数
	long get_iie_switch(bool* pVal);
	long put_iie_switch(bool newVal);
	long get_iie_enhance_coef(int* pVal);
	long put_iie_enhance_coef(int newVal);
	long get_iie_gray_std(int* pVal);
	long put_iie_gray_std(int newVal);
	long get_iie_pos_std(int* pVal);
	long put_iie_pos_std(int newVal);

	//分块直方图接口函数
	long get_clahe_switch(bool* pVal);
	long put_clahe_switch(bool newVal);
	long get_clahe_cliplimit(int* pVal);
	long put_clahe_cliplimit(int newVal);
	long get_clahe_block_width(int* pVal);
	long put_clahe_block_width(int newVal);
	long get_clahe_block_height(int* pVal);
	long put_clahe_block_height(int newVal);
	long get_clahe_is_last_frame(bool* pVal);
	long put_clahe_is_last_frame(bool newVal);

	//锐化
	long get_sp_switch(bool* pVal);
	long put_sp_switch(bool newVal);
	long get_sp_laplace_weight_y16(float* pVal);
	long put_sp_laplace_weight_y16(float newVal);

	//Y8纠偏
	long get_y8adjustbc_switch(bool* pVal);
	long put_y8adjustbc_switch(bool newVal);
	long get_adjustbc_contrast(int* pVal);
	long put_adjustbc_contrast(int newVal);
	long get_adjustbc_bright(int* pVal);
	long put_adjustbc_bright(int newVal);

	//图像放大参数设置
	long get_ImgZoom(int* pVal);
	long put_ImgZoom(int newVal);
	long get_ImgZoomType(int* pVal);
	long put_ImgZoomType(int newVal);

	//伪彩
	long get_psdclr_switch(bool* pVal);
	long put_psdclr_switch(bool newVal);
	long get_psdclr_type(int* pVal);
	long put_psdclr_type(int newVal);
	long get_colorimg_tpye(int* pVal);
	long put_colorimg_tpye(int newVal);

	//输出中间数据开关
	long get_output_y8_switch(bool* pVal);
	long put_output_y8_switch(bool newVal);
	long get_output_y16_switch(bool* pVal);
	long put_output_y16_switch(bool newVal);
};

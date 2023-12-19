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

//����궨��
#define FRAME_FILTER_NUM        (16)//(100)
#define LAST_PARA               (1)
#define GRAYLEVEL_16BIT         (65536) 
#define GRAY_WEIGHT_TABLE_LEN	512
#define TTF_WEIGHT_TABLE_LEN	128
//�ֿ�ֱ��ͼ
#define  CLAHE_DYC_RANGE  256	
#define  PART_WIN_NUM     2    //��ͼ��ֳ�2*2��, �ֿ�Խ�࣬ͼ���ЧӦԽ����
extern unsigned char  PseudoColorTable[11][256][4];

// Dynamic range transform type ��̬��Χת������
enum eDRT_TYPE
{
	DRT_TYPE_LINEAR = 0,         //����
	DRT_TYPE_PLATHE,        //ƽֱ̨��ͼ����
	DRT_TYPE_MIX_PHE_LIN,   //��ϵ���
};

//ͼ��ת��ʽ
enum eFLIP_TYPE
{// ��ֵ����
	FlipX = 0,  //ˮƽ����
	FlipY,       //��ֱ����
	FlipXY      //ˮƽ�Ӵ�ֱ����
};

//ͼ����ת��ʽ
enum eROTATE_TYPE
{// ��ֵ����
	ROTATE90 = 0,  //ˮƽ����
	ROTATE180,       //��ֱ����
	ROTATE270      //ˮƽ�Ӵ�ֱ����
};

//ͼ��Ŵ�ʽ
enum Interpolation
{// ��ֵ����
	Near = 0,
	Bilinear
};

//���ͼ������
enum eCOLORIMG_TYPE
{
	COLORIMG_TYPE_BGR = 0,  //BGR��ʽͼ��
	COLORIMG_TYPE_RGBA,     //RGBA��ʽͼ��
	COLORIMG_TYPE_RGB,      //RGB��ʽͼ��
};

// NUC�Ǿ���У���㷨����
struct stNUCPara
{
	bool  b_nuc_switch;       // NUC Switch
};

struct stDRTPara
{
	int n_drt_type;  //��������
	int b_drt_is_last_frame;      // ����һ֡������

	int n_discard_ratio;	      // �׵㴦�����
	int n_linear_contrast;        // �ԱȶȲ���	
	int n_linear_brightness;      // ���ȵ��ڲ���	
	int n_linear_restrain_rangethre; //����������

	int n_heq_midvalue;
	int n_heq_plat_thresh;
	int n_heq_range_max;

	int n_gn_rangeBC;
	float n_gf_coef;
};

struct stTimeFilterPara
{
	bool b_tff_switch;               //  �㷨��������
	int n_tff_std;					 //  ��׼��
	int n_tff_win_size_w;             //  ���ڴ�С
	int n_tff_win_size_h;
};

struct stRSPara
{
	bool  b_rs_switch;           // ȥ�������㷨����

	int  n_stripe_win_width;     // ȥ�������˲����ڿ��
	int  n_stripe_weight_thresh; // ȥ������Ȩ������
	int  n_stripe_std;           // ȥ�����Ʒ���
	int  n_stripe_detail_thresh;        // �������ж���ֵ
};

// RN��remove noise�� ȥ���㷨����
enum eRN_TYPE
{
	RN_TYPE_GAUSSIAN = 0,         //��˹
	RN_TYPE_GREYSCALE,        //���߻Ҷ�
	RN_TYPE_BILATERAL        //˫���˲�
};

// RN��remove noise�� ȥ���㷨����
struct stRNPara
{
	bool b_rn_switch;	     // ȥ���㷨����
	int n_rn_arith_type;     // ȥ���㷨����
	int n_dist_std; 	     // �ռ��˹�˲���׼��
	int n_gray_std; 	     // ���߻Ҷ��˲���׼��
	int n_win_size;          // ȥ���˲�����
};

// ͼ����
struct stImgFlipPara
{
	bool b_flip_switch;
	int nFlipType;
};

// ͼ����
struct stImgRotatePara
{
	bool b_rotate_switch;
	int nRotateType;
};


// ��sharpen�㷨����
struct stSPPara
{
	bool b_sp_switch;        // �񻯿���
	float n_sp_laplace_weight;//��ϵ��
};

// IIEͼ����ǿ�㷨����
struct stIIEPara
{
	bool b_iie_switch;	    // IIE�㷨Switch
	int n_iie_gray_win_size; 	// ��ǿ�Ҷ��˲����ڴ�С
	int n_iie_pos_win_size; 	// ��ǿ��˹�˲����ڴ�С
	int n_iie_enhance_coef;	// ��ǿ�㷨ϸ������ϵ��
	int n_iie_gray_std;
	int n_iie_gauss_std;

	//�ֿ�ֱ��ͼ����
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

// ͼ��Ŵ�
struct stImgZoomPara
{
	int n_Zoom;
	int n_ZoomType;
};

// α��ɫ
struct stPseudoColorPara
{
	bool b_psdclr_switch;    // α��ɫ����
	int t_psdclr_type;     // α��ɫ����
	int n_colorimg_tpye;   //���ͼ������
};

//����ͼ�������
struct stIrPara
{
	stNUCPara st_nuc_para;    //NUC
	stTimeFilterPara    st_tff_para;       // ʱ���˲�
	stRSPara    st_rvs_para;       // ȥ������
	stRSPara    st_rhs_para;       // ȥ������
	stRNPara    st_rn_para;        // �����˲�
	stImgFlipPara st_flip_para;  //ͼ��ת
	stImgRotatePara st_rotate_para;  //ͼ����ת
	stIIEPara    st_iie_para;       // IIE��ǿϵ��
	stDRTPara  st_drt_para;				// ����
	stSPPara     st_sp_para;        // ��
	stY8ADJUSTBCPara st_y8adjustbc_para;
	stImgZoomPara st_zoom_para;         //ͼ��Ŵ����
	stPseudoColorPara st_psdclr_coef;    // α��ɫ
};

//����ͼ������
struct stIfrImage
{
	int n_width;
	int n_height;
	int n_len;
	int n_channels;

	short* pus_data;	   //��ʾ��������?
	unsigned char* puc_data;
};

struct stIrContent
{
	stIfrImage pst_src_img;    //�������ݺ���ͼ��
	stIfrImage pst_dst_img;    //������ݺ���ͼ��

	unsigned short* pus_shutter_base;  //���ű���
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
	//����ͼ�����ݳ�ʼ��
	void IRContentInitialize();
	//����ͼ����������ʼ��
	void IRParaInitialize();

	//�Ǿ���У��
	void NUCbyTwoPoint();
	void ReplaceBadPoint(short* pus_src, unsigned short* pus_gain_mat, int n_width, int n_height);

	//����ʱ���˲�Ȩ�ر�
	void GetTFFGuassWeight();
	//�߽���չ����
	void PadMatrix(short* pus_pad_mat, short* pus_mat, int n_width, int n_height, int n_win_size_width, int n_win_size_height);
	//ʱ���˲�����
	void TimeNoiseFliter(short* pus_dst, short* pus_src, int nWidth, int nHeight, int WinSizeW, int WinSizeH, int std_cur);
	//ʱ���˲���װ����
	void TimeFF();

	//ȥ������
	void FixedPoint_GrayFilter_16bit_RSN(short* pus_dst, unsigned short* pus_weight, short* pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);
	void RemoveVerStripe(short* pus_dst, short* pus_src, int width, int height, int win_width, int* gray_weight_table, int weight_thresh, int detail_thresh);
	//ȥ����
	void RemoveHorStripe(short* pus_dst, short* pus_src, int width, int height, int win_height, int* gray_weight_table, int weight_thresh, int detail_thresh);
	void RemoveSVN();
	void RemoveSHN();

	//�����˲�
	void SpaceFilter();
	void GetGaussGrayWeightTable(int* pa_gray_weight_table, int n_len, int n_gray_std);
	void GetGuassPosWeightTable(int* a_pos_weight_table, int n_win_wid, int n_win_hei, int std);
	void GaussianFilter_16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int n_win_wid, int* pos_weight_table);
	void FixedPoint_GrayFilter_16bit(short* pus_dst, short* pus_src, int n_width, int n_height, int* gray_weight_table, int n_win_width, int n_win_height);

	//ͼ����
	void ImgFlip();
	void Flip(short* pus_dst, short* pus_src, int n_width, int n_height, int nfliptype);

	//ͼ����ת
	void ImgRotation();
	void Rotation(short* pus_dst, short* pus_src, int n_width, int n_height, int nrotationtype);

	//����
	void ModelDRT();

	//ϸ����ǿ
	void FixedPoint_IIE(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, stIrPara* IrPara);
	void IIE();

	//����
	void GetHist(int* pHist, int* pnMaxOri, int* pnMinOri, int* pnMaxDisc, int* pnMinDisc, int* pGrayMean, short* pusSrcImg, int n_width,
		int n_height, int nHistSize, int nDiscardRatio);

	void DRC_LinearAutoBC(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nDiscardRatio, int nBrightExp, int nContrastExp, int nRestrainRangeThre);

	void DRC_PlatHistEqualize(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nPlatThresh, int nDiscardRatio, int nEqhistMax, int nMidValue);

	void DRC_Mix(unsigned char* puc_dst, short* pus_src, int n_width, int n_height, int nMixContrastExp, int nMixBrightExp, int nRestrainRangeThre, int n_mixrange, int nMidValue, int nPlatThresh, int nDiscardRatio);

	//�ֿ�ֱ��ͼ
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

	//Y8������˹��
	void LaplaceSharpen(unsigned char* pus_dst, unsigned char* pus_src, int n_width, int n_height, float weight);
	void LaplaceSharpen(stIrContent* p_ir_content, stIrPara* IrPara);

	//Y8��ƫ
	void ImgY8AdjustBC();
	void Y8AdjustBC(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int nContrastexp, int nBrightexp);

	//ͼ������
	void ImgResize();
	void Resize(unsigned char* puc_dst, unsigned char* puc_src, int n_width, int n_height, int n_Zoom, int ZoomType);

	//α��
	void PseudoColorMap();
	void PseudoColor(unsigned char* puc_dst, unsigned char* puc_src, int height, int width, int pseudo_color_type, int colorimg_tpye);

private:
	stIrPara IrPara;
	int n_width, n_height;  //����ͼ����
	int n_Zoom;//����ͼ��Ŵ���
	//ʱ���˲�ȫ�ֱ���
	int g_nFFTWeightTable[TTF_WEIGHT_TABLE_LEN];//ʱ���˲�Ȩ��
	short* g_pusTffLastFrame = NULL;
	int nFilterFrame = 0;       //֡������
	//ʱ���˲��ֲ�ָ��
	short* diff = NULL;
	short* temp = NULL;
	short* pus_src_pad = NULL;  //ʱ���˲������˲����øõ�ַ
	short* pus_last_pad = NULL;

	//ȥ�����㷨ȫ�ֱ���
	int g_gray_weight_table_Ver[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table_Hor[GRAY_WEIGHT_TABLE_LEN];
	//ȥ�������㷨�ֲ�ָ��
	short* pus_img_low = NULL;    //�˲���ĵ�Ƶͼ��
	unsigned short* pus_img_weight = NULL;
	int* g_img_high = NULL;        //ϸ��ͼ�� 
	int* g_ver_mean = NULL;  //�о�ֵ����
	int* g_row_mean = NULL;     //�о�ֵ����

	//�����˲�ȫ�ֱ���
	int g_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];

	//����ȫ�ֱ���
	int g_nOffset = 32768;
	int g_nFrameCnt = 0;
	int g_nContrast = 0;
	int g_nBright = 0;
	int g_nLastFrameRange = 0;
	int g_nLastFrameMean = 0;
	int g_nLastFrameMax = 0;
	int g_nLastFrameMin = 0;
	int g_nMaxContrast = 4;  //�Աȶ�����ֵ 
	int g_nGrayRange = 0;  //�׵㴦���Ķ�̬��Χ
	int g_nPheWeightLast = 0;
	int g_nGrayRangeLast = 0;
	unsigned char* puc_dst_lin = NULL;
	unsigned char* puc_dst_phe = NULL;

	//ϸ����ǿ
	int g_IDE_pos_weight_table[GRAY_WEIGHT_TABLE_LEN];
	int g_IDE_gray_weight_table[GRAY_WEIGHT_TABLE_LEN];
	short* ps_detail_img = NULL; // ϸ��ͼ��
	unsigned char* puc_base_img = NULL; // 8bit��Ƶ
	unsigned char* puc_base_img_clahe = NULL;
	short* pus_base_img = NULL; // 16bit��Ƶ
	short* pus_base_img2 = NULL; // ��Ƶͼ��

	//�ֿ�ֱ��ͼ
	int p_map_array_pre[CLAHE_DYC_RANGE * PART_WIN_NUM * PART_WIN_NUM] = { 0 };
	int g_nFrameCntClahe = 0;
	int* p_map_array = NULL;
	int* pHist_clahe = NULL;
	int* pHistSum_clahe = NULL;

	//����м����ݿ���
	bool b_outdata_y16_switch, b_outdata_y8_switch;
public:
	//����м����ݶ���ָ��
	short* pus_output_y16 = NULL;
	int n_outdata_y16_width, n_outdata_y16_height;
	unsigned char* puc_output_y8 = NULL;
	int n_outdata_y8_width, n_outdata_y8_height;

	//ͼ������
	stIrContent IRContent;

	int GetKGainMat(unsigned short* kGainMat, char* curvePackStr, char* curveKGainMatStr, int width, int heitht, int kIndex, int N = 7, int D = 2, int CURVE_LENGTH = 2100);

	//����ͼ����������
	void InfraredImageProcess();

	/**************************����Android��Ҫ��ͨ����α��ָ��ķ�ʽ���α�ʱ�
	���������
	unsigned char*pSrc,  ���뵥ͨ��y8ͼ��
	int nHeight, int nWidth,  ����ͼ����
	int *paletteArr  α�ʱ�
	���������
	unsigned char*pColorImg �����ͨ��α��ͼ��
	*/
	void PseudoColorForAndroid(unsigned char* pColorImg, unsigned char* pSrc, int nHeight, int nWidth, int* paletteArr);

	//////������ȡ������
	//�Ǿ���У��
	long get_nuc_switch(bool* pVal);
	long put_nuc_switch(bool newVal);

	//ʱ���˲� ����ֻ���ű�׼��
	long get_tff_switch(bool* pVal);
	long put_tff_switch(bool newVal);
	long get_tff_std(int* pVal);
	long put_tff_std(int newVal);

	//ȥ�����㷨
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
	//ȥ�����㷨
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

	//�����˲�
	long get_rn_switch(bool* pVal);
	long put_rn_switch(bool newVal);
	long get_rn_arith_type(int* pVal);
	long put_rn_arith_type(int newVal);
	long get_rn_dist_std(int* pVal);
	long put_rn_dist_std(int newVal);
	long get_rn_gray_std(int* pVal);
	long put_rn_gray_std(int newVal);

	//ͼ����
	long get_flip_switch(bool* pVal);
	long put_flip_switch(bool newVal);
	long get_flip_type(int* pVal);
	long put_flip_type(int newVal);

	//ͼ����ת
	long get_rotate_switch(bool* pVal);
	long put_rotate_switch(bool newVal);
	long get_rotate_type(int* pVal);
	long put_rotate_type(int newVal);

	//�������
	long get_drt_type(int* pVal);
	long put_drt_type(int newVal);
	long get_discard_ratio(int* pVal);
	long put_discard_ratio(int newVal);
	//���Ե������
	long get_linear_brightness(int* pVal);
	long put_linear_brightness(int newVal);
	long get_linear_contrast(int* pVal);
	long put_linear_contrast(int newVal);
	long get_linear_restrain_rangethre(int* pVal);
	long put_linear_restrain_rangethre(int newVal);
	//ƽֱ̨��ͼ����
	long get_heq_plat_thresh(int* pVal);
	long put_heq_plat_thresh(int newVal);
	long get_heq_range_max(int* pVal);
	long put_heq_range_max(int newVal);
	long get_heq_midvalue(int* pVal);
	long put_heq_midvalue(int newVal);

	//ϸ����ǿ����
	long get_iie_switch(bool* pVal);
	long put_iie_switch(bool newVal);
	long get_iie_enhance_coef(int* pVal);
	long put_iie_enhance_coef(int newVal);
	long get_iie_gray_std(int* pVal);
	long put_iie_gray_std(int newVal);
	long get_iie_pos_std(int* pVal);
	long put_iie_pos_std(int newVal);

	//�ֿ�ֱ��ͼ�ӿں���
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

	//��
	long get_sp_switch(bool* pVal);
	long put_sp_switch(bool newVal);
	long get_sp_laplace_weight_y16(float* pVal);
	long put_sp_laplace_weight_y16(float newVal);

	//Y8��ƫ
	long get_y8adjustbc_switch(bool* pVal);
	long put_y8adjustbc_switch(bool newVal);
	long get_adjustbc_contrast(int* pVal);
	long put_adjustbc_contrast(int newVal);
	long get_adjustbc_bright(int* pVal);
	long put_adjustbc_bright(int newVal);

	//ͼ��Ŵ��������
	long get_ImgZoom(int* pVal);
	long put_ImgZoom(int newVal);
	long get_ImgZoomType(int* pVal);
	long put_ImgZoomType(int newVal);

	//α��
	long get_psdclr_switch(bool* pVal);
	long put_psdclr_switch(bool newVal);
	long get_psdclr_type(int* pVal);
	long put_psdclr_type(int newVal);
	long get_colorimg_tpye(int* pVal);
	long put_colorimg_tpye(int newVal);

	//����м����ݿ���
	long get_output_y8_switch(bool* pVal);
	long put_output_y8_switch(bool newVal);
	long get_output_y16_switch(bool* pVal);
	long put_output_y16_switch(bool newVal);
};

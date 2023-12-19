#pragma once

#include <iostream>
#include <fstream> 
#include <vector>
#define DllExport __declspec(dllexport)

typedef struct IntrinsicParam {
	//Intrinsic matrix K
	//K=[fx,0, cx
	//   0, fy,cy
	//    0, 0, 1]
	float fx;
	float cx;
	float fy;
	float cy;
	//distortion param distCoff
	//distCoff=[k1,k2,p1,p2,k3]
	float distCoff[5];

	//ZM05B 
	float H;
	float alpha;
	float h;

	float rms;
}intrisic_param_t;


typedef struct IRPoint
{
	float x;
	float y;
}ir_point_m;

class DllExport IRCalibIntrinsic
{
public:
	static IRCalibIntrinsic* Instance();

	//set calib project type
	//params:std::string setProductType  :input ZM05B or XD01A   
	//return value 
	//       true:success
	//       false:input empty
	bool IRCalibIntrinsic::setTypeValue(std::string setProductType);

	//set calib rms_limt value 
	//params:std::string setProductType  :input rms_limt value   
	//return value 
	//       true:success
	//       false:input value error
	bool IRCalibIntrinsic::setRmsLimt(float rms_limt1);

	///getFourPoint 
	//params: unsigned char * input_img  :input image data
	//        ir_point_m& centerPoint: circle center point,
	//return value
	//       0:success
	//       -1:image empty
	//       -2:motion blur image
	//       -3:image error
	//       -4:图像未拍全
	int IRCalibIntrinsic::getCenterPoint(unsigned char * input_img, ir_point_m& centerPoint);

	///irCalibIntrinsicAuto 
	//params: std::vector<std::string> imagePathList  :input all image absolute path 
	//        intrisic_param_t& m_intrisic_param: m_intrisic_param include fx,cx,fy,cy,distcoff,H,alpha
	//return value
	//       0:success
	//       -1:input image num <2
	//       -2:detect corner effective image num <2
	//       -3:motion blur image
	//       -4:image error 全黑图像
	//       -5:image error 相邻两张图像一致
	//       -6:rms >errLimt
	int IRCalibIntrinsic::irCalibIntrinsicAuto(std::vector<std::string> imagePathList, intrisic_param_t& m_intrisic_param);
	~IRCalibIntrinsic();
private:
	int IRCalibIntrinsic::irCalibIntrinsic(std::string productID, int numIm);
	int squareWidth;
	int squareHeight;
	double squareSize;
	bool flagReadRes = false;
	bool flagSaveRes = true;
	bool flagImgFlip = false;
	std::string cpreName1Str, fmtIm1Str;
	std::string productType;

    float rms_limt;//rms

	IRCalibIntrinsic();
	IRCalibIntrinsic(const IRCalibIntrinsic&);
	IRCalibIntrinsic& operator=(const IRCalibIntrinsic&);
};

#pragma once

#include <iostream>
#include <vector>

#define DllExport __declspec(dllexport)

typedef enum tagErrorcode
{
	SUCCESS = 0,  //执行成功
	FAILED
};
struct IRPoint
{
	double x;
	double y;
};

class DllExport IRCalib
{
public:
	float installHeight;//安装高度

	static IRCalib* Instance();

	bool IRCalib::setValue(int setSquareWidth, int setSquareHeight, int setSquareSize, bool setFlagImgFlip, std::string setFmtIm1Str, float m_intrinsic_param[14]);
	int IRCalib::irCalib(std::string productID, int numIm);
	int IRCalib::saveIntrinsicParam(std::vector<float>& m_intrinsic_param,int len);
	
	//计算外参安装高度
	int IRCalib::calcInstallHeight(std::string productId, int numIm, float h0,float& H);
	
	int IRCalib::calcElurAngle(std::string productId, int numIm);//暂不调用

	int IRCalib::calcTargetPoint(double L0, std::vector<struct IRPoint>& targetPointList);
	void IRCalib::DistortPoints(struct IRPoint& undistPoint, struct IRPoint& distPoint);

	//vector len=3
	int IRCalib::setPitchAngle(double L0, double pitchAngle, std::vector<struct IRPoint>& targetPointPitchList);
	int IRCalib::setYawAngle(double L0, double yawAngle, double pitchAngle, std::vector<struct IRPoint>& targetPointYawList);
	int IRCalib::setRollAngle(double L0, double yawAngle, double pitchAngle, std::vector<struct IRPoint>& targetPointRollList);

	int IRCalib::saveExternalParam(std::string productId, double pitchAngle, double yawAngle, double rollAngle);
	//int IRCalib::setYawAngle(double rollAngle, double d, std::vector<cv::Point2f>& targetPointYawList);
	//int IRCalib::setPitchAngle(double pitchAngle, cv::Point2f& targetPoint_Pitch);
	~IRCalib();
private:
	int squareWidth;//靶标长方形
	int squareHeight;//靶标
	double squareSize;//靶标边长(mm)
	bool flagReadRes = false;
	bool flagSaveRes = true;
	bool flagImgFlip = false;
	std::string cpreName1Str, fmtIm1Str;

	float pitch;
	float yaw;
	float roll;
	
	float h;

	IRCalib();
	IRCalib(const IRCalib&);
	IRCalib& operator=(const IRCalib&);
};
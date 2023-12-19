#pragma once

#define FIRST_THRESHOLD 80
#define SECOND_THRESHOLD 50
#define DIRT_CHECK_THRESHOLD 20
#define DIRT_SECOND_CHECK_THRESHOLD 10
#define DIRT_AROUND_CHECK_THRESHOLD 5

#define FRQ_LOW_THRESHOLD 6.0
#define FRQ_HIGH_THRESHOLD 11.0

#define CHECK_START_LEN 5 
#define CHECK_END_LEN 15
#define SUM_LEN 10
#define CHECK_THRESHOLD 10

#include <vector>

#include "halconcpp.h"

using namespace HalconCpp;

struct badLineData
{
	int line = 0;
	int startIndex;
	int endIndex;
};

struct tRect
{
	int x;
	int y;
	int w;
	int h;
};

class imgCalY8
{
public:

	void checkVerBadLine(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height);//检测竖线，输入yuv数据的Y8
	void checkHorBadLine(unsigned char* src, std::vector<badLineData>&horIndex, int width, int height);//检测横线，输入yuv数据的Y8
	void checkDirt(unsigned char* src, std::vector<tRect>&outRect, int width, int height);//检测灰尘，输入Y16的转成Y8灰度图
	void checkDirtNew(unsigned char* src, std::vector<tRect>&outRect, int width, int height);//检测灰尘，输入Y8灰度图
	
	void checkDirtNew(unsigned short* src, std::vector<tRect>&outRect, int width, int height);//检测灰尘，输入Y8灰度图

	void checkDirtHalcon(unsigned char* src, std::vector<tRect>&outRect, int width, int height);//新添加

	
private:

	void scale_image_range(HObject ho_Image, HObject *ho_ImageScaled, HTuple hv_Min,
		HTuple hv_Max);//新添加


	void avgFilter(unsigned char* src, unsigned char* dst, int width, int height);
	void avgFilter(unsigned short* src, unsigned short* dst, int width, int height);
	void segThreshold(unsigned char* src, unsigned char* dst, int width, int height);
	void segThreshold_Time(unsigned char* src, unsigned char* dst, int width, int height);
	void segThreshold(unsigned short* src, unsigned char* dst, int width, int height);
	void secondSegThreshold(unsigned char* orginSrc, unsigned char* src, std::vector<tRect>& outRect, int width, int height);
	void secondSegThreshold(unsigned short* orginSrc, unsigned char* src, std::vector<tRect>& outRect, int width, int height);

	bool isDirtCheck(unsigned char* orginSrc, unsigned short* src, tRect rect, int id, int width, int height);
	bool isDirtCheck(unsigned short* orginSrc, unsigned short* src, tRect rect, int id, int width, int height);

private:
	void gaussFilter(unsigned char* src, unsigned char* dst, int width, int height);
	void sharpFilter(unsigned char* orginSrc, unsigned char* filterSrc, unsigned char* dst, int width, int height);

	void midFilter(unsigned char* src, unsigned char* dst, int kernelSize, int width, int height);
	void gradFilter(unsigned char* src, unsigned char* dst, int width, int height);

	void maxFilter(unsigned char* src, unsigned char* dst, int width, int height);

	void verMidFilter(unsigned char* src, unsigned char* dst, int filterLen, int width, int height);

	void hovMidFilter(unsigned char* src, unsigned char* dst, int filterLen, int width, int height);

	void verGradCheckLow(unsigned char* src, unsigned char* dst, int width, int height);
	void checkVerBadLineLow(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height);
	void verGradCheckHigh(unsigned char* src, unsigned char* dst, int width, int height);
	void checkVerBadLineHigh(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height);
	
	void horGradCheckLow(unsigned char* src, unsigned char* dst, int width, int height);
	void checkHorBadLineLow(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height);
	void horGradCheckHigh(unsigned char* src, unsigned char* dst, int width, int height);
	void checkHorBadLineHigh(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height);



	unsigned char midData(unsigned char*src, int left, int right, int top, int bottom, int width);
	unsigned char midDataSingle(unsigned char*src, int len);

	void getThresholdData(unsigned char* src, unsigned char* dst, int width, int height);

};


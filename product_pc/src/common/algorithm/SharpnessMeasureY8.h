#pragma once
#include <string>

struct sSharpnessValue
{
	float SharpnessLU;
	float SharpnessRU;
	float SharpnessCenter;
	float SharpnessLD;
	float SharpnessRD;
};

/*设备清晰度测量函数
SharpnessMeasure函数可同时测量图像5个区域(分别为左上，右上，中间，左下和右下)内的清晰度。

unsigned char* nSrc,输入Y16数据。
int nWidth, int nHeight, Y16数据宽高。
int x, int y，计算清晰度矩形x，y坐标；
int nRoiWidth, int nRoiHeight， 计算清晰度区域宽高，默认nRoiWidth = 25，nRoiHeight = 15。
sSharpnessValue* sSV,返回图像5个区域内清晰度（五个重合区域）。
*/
void SharpnessMeasure(sSharpnessValue* sSV, unsigned char* nSrc, int nWidth, int nHeight, int x, int y, int nRoiWidth, int nRoiHeight);
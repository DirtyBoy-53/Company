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

/*�豸�����Ȳ�������
SharpnessMeasure������ͬʱ����ͼ��5������(�ֱ�Ϊ���ϣ����ϣ��м䣬���º�����)�ڵ������ȡ�

unsigned char* nSrc,����Y16���ݡ�
int nWidth, int nHeight, Y16���ݿ�ߡ�
int x, int y�����������Ⱦ���x��y���ꣻ
int nRoiWidth, int nRoiHeight�� ���������������ߣ�Ĭ��nRoiWidth = 25��nRoiHeight = 15��
sSharpnessValue* sSV,����ͼ��5�������������ȣ�����غ����򣩡�
*/
void SharpnessMeasure(sSharpnessValue* sSV, unsigned char* nSrc, int nWidth, int nHeight, int x, int y, int nRoiWidth, int nRoiHeight);
#ifndef FINDBPY8_H
#define FINDBPY8_H
#include "common_struct.h"

#define WIDTH 640
#define HEIGHT 512
#define BP_NUM_MAX 1024


int FindBPY8(unsigned char* src, int width, int height, unsigned short* K, int nThresh, int frameNum);

int GetBPlist(POINTX *szBPPoint, unsigned short *K, int width, int height, int frameThreshold);

int CheckBPY8(unsigned char* src, int width, int height, unsigned short* K, int nThresh, int frameNum);

#endif // FINDBPY8_H

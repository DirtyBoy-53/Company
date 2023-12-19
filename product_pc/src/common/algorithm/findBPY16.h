#ifndef FINDBPY16_H
#define FINDBPY16_H

#include "common_struct.h"
#define WIDTH 640
#define HEIGHT 512
#define BP_NUM_MAX 1024


int FindBPY16(unsigned short* src, int width, int height, unsigned short* K, int nThresh, int frameNum);

int GetBPlistY16(POINTX *szBPPoint, unsigned short *K, int width, int height, int frameThreshold);

int CheckBPY16(unsigned short* src, int width, int height, unsigned short* K, int nThresh, int frameNum);

#endif // FINDBPY16_H

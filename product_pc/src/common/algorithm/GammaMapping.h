#pragma once
#include <stdio.h>

#define ClipU8(x) ((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))
#define PSEUDONUM (41)

extern const unsigned char  PseudoRGB[PSEUDONUM][256][4];

/*RGB白热图像加Gamma映射
unsigned char* nDst,  加Gamma校正后的图像，图像格式为RGBRGBRGB....
unsigned char* nSrc,  RGB格式白热红外图像，图像格式为RGBRGBRGB....
int nWidth, int nHeight, 红外图像宽高
unsigned char GammaGrade Gamma等级 0—1.0  1—1.1  2—1.2  3—1.3  4—1.4  5—1.5
*/
void GammaMapping(unsigned char* nDst, unsigned char* nSrc, int nWidth, int nHeight, unsigned char GammaGrade);

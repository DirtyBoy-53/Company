#ifndef __CHECKBOX_EE_H__
#define __CHECKBOX_EE_H__
#include <vector>

typedef struct {
	int x;
	int y;
	int width;
	int height;
} MyRectangle;

//bool detectColorBox(const unsigned char* uyvyData, int width, int height, int stride);
std::vector<MyRectangle> findColorRectangles(const unsigned char* uyvyData, int width, int height, int stride, int colorThreshold);

#endif

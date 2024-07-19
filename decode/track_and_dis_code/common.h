#pragma once
#include "opencv2/opencv.hpp"

using namespace cv;

struct Object
{
	Rect_<float> rect;
	int label;
	float prob;
};
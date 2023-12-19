#include "imgCalY8.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

void imgCalY8::checkVerBadLine(unsigned char* src, std::vector<badLineData>& verIndex, int width, int height)
{
	verIndex.clear();
	unsigned char* dst = new unsigned char[width * height];
	verGradCheckLow(src, dst, width, height);
	checkVerBadLineLow(dst, verIndex, width, height);
	verGradCheckHigh(src, dst, width, height);
	checkVerBadLineHigh(dst, verIndex, width, height);
	delete[]dst;
}

void imgCalY8::checkHorBadLine(unsigned char* src, std::vector<badLineData>& horIndex, int width, int height)
{
	horIndex.clear();
	unsigned char* dst = new unsigned char[width * height];
	horGradCheckLow(src, dst, width, height);
	checkHorBadLineLow(dst, horIndex, width, height);
	horGradCheckHigh(src, dst, width, height);
	checkHorBadLineHigh(dst, horIndex, width, height);
	delete[]dst;
}

void imgCalY8::checkDirt(unsigned char* src, std::vector<tRect>& outRect, int width, int height)
{
	outRect.clear();
	cv::Mat outImg(height, width, CV_8UC1);
	cv::Mat outImgTmp(height, width, CV_8UC1);
	midFilter(src, outImg.data, 5, width, height);
	gradFilter(outImg.data, outImgTmp.data, width, height);
	maxFilter(outImgTmp.data, outImg.data, width, height);
	cv::Mat out, stats, centroids;
	int number = cv::connectedComponentsWithStats(outImg, out, stats, centroids, 8, CV_16U);
	int outCount = 0;
	for (int i = 0; i < number; ++i)
	{
		int x = stats.at<int>(i, CC_STAT_LEFT);
		int y = stats.at<int>(i, CC_STAT_TOP);
		int w_ = stats.at<int>(i, CC_STAT_WIDTH);
		int h_ = stats.at<int>(i, CC_STAT_HEIGHT);
		int area = stats.at<int>(i, CC_STAT_AREA);
		if (w_ > 10 && w_< 320 && h_ > 10 && h_<256)
		{
			tRect tr;
			tr.h = h_;
			tr.w = w_;
			tr.x = x;
			tr.y = y;
			outCount++;
			outRect.push_back(tr);
		}
	}
	if (outCount > 10)
		outRect.clear();
}

void imgCalY8::checkDirtNew(unsigned char* src, std::vector<tRect>& outRect, int width, int height)
{
	uchar* dst1 = new uchar[width * height];
	uchar* dst2 = new uchar[width * height];
	Mat out_y_8_img1(height, width, CV_8UC1, dst1);
	Mat out_y_8_img2(height, width, CV_8UC1, dst2);
	avgFilter(src, dst2, width, height);
//	getThresholdData(dst2, dst1, width, height);
//	avgFilter(dst1, dst2, width, height);
	segThreshold(dst2, dst1, width, height);
	secondSegThreshold(dst2, dst1, outRect, width, height);

	segThreshold_Time(dst2, dst1, width, height);
	std::vector<tRect> tempRect;
	secondSegThreshold(dst2, dst1, tempRect, width, height);
	for (int i = 0; i < tempRect.size(); ++i)
	{
		int tL = tempRect[i].x;
		int tT = tempRect[i].y;
		int tR = tempRect[i].x + tempRect[i].w;
		int tB = tempRect[i].y + tempRect[i].h;
		bool isCross = false;
		for (int j = 0; j < outRect.size(); ++j)
		{
			int oL = outRect[j].x;
			int oT = outRect[j].y;
			int oR = outRect[j].x + outRect[j].w;
			int oB = outRect[j].y + outRect[j].h;

			int x_min = max(tL, oL);
			int x_max = min(tR, oR);
			int y_min = max(tT, oT);
			int y_max = max(tB, oB);
			if (x_max-x_min>0&&y_max-y_min>0)
			{
				isCross = true;
				break;
				
			}
		}
//		if (!isCross)
//			outRect.push_back(tempRect[i]);

	}

	delete[]dst1;
	delete[]dst2;
}

void imgCalY8::checkDirtNew(unsigned short* src, std::vector<tRect>& outRect, int width, int height)
{
	uchar* dst1 = new uchar[width * height];
	ushort* dst2 = new ushort[width * height];
	Mat out_y_8_img1(height, width, CV_16UC1, dst1);
	Mat out_y_8_img2(height, width, CV_16UC1, dst2);
	avgFilter(src, dst2, width, height);

	//	avgFilter(dst1, dst2, width, height);
	segThreshold(dst2, dst1, width, height);
	secondSegThreshold(dst2, dst1, outRect, width, height);
	delete[]dst1;
	delete[]dst2;

}

void imgCalY8::checkDirtHalcon(unsigned char* src, std::vector<tRect>& outRect, int width, int height)
{

	// Local iconic variables
	HObject  ho_Image, ho_GaussFilter1, ho_GaussFilter2;
	HObject  ho_Filter, ho_ImageFFT1, ho_ImageFFT;
	HObject  ho_ImageConvol, ho_ImageFiltered, ho_ImageScaled;
	HObject  ho_Region, ho_ConnectedRegions, ho_SelectedRegions;
	HObject  ho_Region_, ho_ConnectedRegions_, ho_SelectedRegions_;
	HObject  ho_ObjectSelected, ho_ObjectSelected_, ho_RegionUnion;

	// Local control variables
	HTuple  hv_path, hv_Width, hv_Height, hv_Sigma1;
	HTuple  hv_Sigma2, hv_Area, hv_Row, hv_Column, hv_Row1;
	HTuple  hv_Column1, hv_Row2, hv_Column2, hv_Number, hv_Area_;
	HTuple  hv_Row_, hv_Column_, hv_Row_1, hv_Column_1, hv_Row_2;
	HTuple  hv_Column_2, hv_Number_, hv_i, hv_j, hv_Area_1;
	HTuple  hv_Area_2, hv_Area_3, hv_Row_3, hv_Column_3;
	ho_Image = HObject();
	//	Mat srcImg = imread(maPath, IMREAD_GRAYSCALE);
	GenImage1(&ho_Image, "byte", width, height, (Hlong)src);	GetImageSize(ho_Image, &hv_Width, &hv_Height);
	hv_Sigma1 = FRQ_HIGH_THRESHOLD;
	hv_Sigma2 = FRQ_LOW_THRESHOLD;
	GenGaussFilter(&ho_GaussFilter1, hv_Sigma1, hv_Sigma1, 0.0, "none", "rft", hv_Width,
		hv_Height);
	GenGaussFilter(&ho_GaussFilter2, hv_Sigma2, hv_Sigma2, 0.0, "none", "rft", hv_Width,
		hv_Height);
	SubImage(ho_GaussFilter1, ho_GaussFilter2, &ho_Filter, 1, 0);

	FftImage(ho_Image, &ho_ImageFFT1);
	RftGeneric(ho_Image, &ho_ImageFFT, "to_freq", "none", "complex", hv_Width);
	ConvolFft(ho_ImageFFT, ho_Filter, &ho_ImageConvol);
	RftGeneric(ho_ImageConvol, &ho_ImageFiltered, "from_freq", "n", "real", hv_Width);
	scale_image_range(ho_ImageFiltered, &ho_ImageScaled, 0, 255);



	//write_image (ImageScaled, 'png', -1, 'F:/')
	Threshold(ho_ImageScaled, &ho_Region, 1, 255);
	Connection(ho_Region, &ho_ConnectedRegions);
	SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, (HTuple("circularity").Append("area")),
		"and", (HTuple(0.6).Append(80)), (HTuple(1).Append(1200)));
	AreaCenter(ho_SelectedRegions, &hv_Area, &hv_Row, &hv_Column);
	SmallestRectangle1(ho_SelectedRegions, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
	CountObj(ho_SelectedRegions, &hv_Number);

	Threshold(ho_ImageScaled, &ho_Region_, 0.1, 255);
	Connection(ho_Region_, &ho_ConnectedRegions_);
	SelectShape(ho_ConnectedRegions_, &ho_SelectedRegions_, (HTuple("circularity").Append("area")),
		"and", (HTuple(0.6).Append(80)), (HTuple(1).Append(1200)));
	AreaCenter(ho_SelectedRegions_, &hv_Area_, &hv_Row_, &hv_Column_);
	SmallestRectangle1(ho_SelectedRegions_, &hv_Row_1, &hv_Column_1, &hv_Row_2, &hv_Column_2);
	CountObj(ho_SelectedRegions_, &hv_Number_);

	int num = int(hv_Number);
	int num_ = int(hv_Number_);
	outRect.clear();

	HTuple end_val34 = hv_Number;
	HTuple step_val34 = 1;
	for (hv_i = 1; hv_i.Continue(end_val34, step_val34); hv_i += step_val34)
	{
			HTuple end_val35 = hv_Number_;
			HTuple step_val35 = 1;
			bool isTarget = false;
			for (hv_j = 1; hv_j.Continue(end_val35, step_val35); hv_j += step_val35)
			{
				SelectObj(ho_SelectedRegions, &ho_ObjectSelected, hv_i);
				SelectObj(ho_SelectedRegions_, &ho_ObjectSelected_, hv_j);
				Union2(ho_ObjectSelected, ho_ObjectSelected_, &ho_RegionUnion);
				AreaCenter(ho_ObjectSelected, &hv_Area_1, &hv_Row_1, &hv_Column_1);
				AreaCenter(ho_ObjectSelected_, &hv_Area_2, &hv_Row_2, &hv_Column_2);
				AreaCenter(ho_RegionUnion, &hv_Area_3, &hv_Row_3, &hv_Column_3);
				if (int (hv_Area_2) == int(hv_Area_3))
				{
					isTarget = true;
					break;
				}
			}
			if(isTarget)
			{
				tRect rect;
				rect.x = int(hv_Column1[hv_i-1]);
				rect.y = int(hv_Row1[hv_i-1]);
				rect.w = int(hv_Column2[hv_i-1]) - int(hv_Column1[hv_i-1]);
				rect.h = int(hv_Row2[hv_i-1]) - int(hv_Row1[hv_i-1]);
				outRect.push_back(rect);
			}
		
	}

/*
	for (int i = 0; i < num; ++i)
	{
		tRect rect;
		rect.x = int(hv_Column1[i]);
		rect.y = int(hv_Row1[i]);
		rect.w = int(hv_Column2[i]) - int(hv_Column1[i]);
		rect.h = int(hv_Row2[i]) - int(hv_Row1[i]);
		bool isTarget = false;
		for (int j = 0; j < num_; ++j)
		{
			tRect rect_;
			rect_.x = int(hv_Column_1[j]);
			rect_.y = int(hv_Row_1[j]);
			rect_.w = int(hv_Column_2[j]) - int(hv_Column_1[j]);
			rect_.h = int(hv_Row_2[j]) - int(hv_Row_1[j]);
			if (rect.x >= rect_.x && rect.y >= rect_.y && int(hv_Column2[i]) <= int(hv_Column_2[j]) && hv_Row2[i] <= hv_Row_2[j])
			{
				isTarget = true;
				break;
			}
		}
		if (isTarget)
			outRect.push_back(rect);
	}
	*/
}

void imgCalY8::scale_image_range(HObject ho_Image, HObject* ho_ImageScaled, HTuple hv_Min, HTuple hv_Max)
{	// Local iconic variables
	HObject  ho_ImageSelected, ho_SelectedChannel;
	HObject  ho_LowerRegion, ho_UpperRegion, ho_ImageSelectedScaled;

	// Local control variables
	HTuple  hv_LowerLimit, hv_UpperLimit, hv_Mult;
	HTuple  hv_Add, hv_NumImages, hv_ImageIndex, hv_Channels;
	HTuple  hv_ChannelIndex, hv_MinGray, hv_MaxGray, hv_Range;

	//Convenience procedure to scale the gray values of the
	//input image Image from the interval [Min,Max]
	//to the interval [0,255] (default).
	//Gray values < 0 or > 255 (after scaling) are clipped.
	//
	//If the image shall be scaled to an interval different from [0,255],
	//this can be achieved by passing tuples with 2 values [From, To]
	//as Min and Max.
	//Example:
	//scale_image_range(Image:ImageScaled:[100,50],[200,250])
	//maps the gray values of Image from the interval [100,200] to [50,250].
	//All other gray values will be clipped.
	//
	//input parameters:
	//Image: the input image
	//Min: the minimum gray value which will be mapped to 0
	//     If a tuple with two values is given, the first value will
	//     be mapped to the second value.
	//Max: The maximum gray value which will be mapped to 255
	//     If a tuple with two values is given, the first value will
	//     be mapped to the second value.
	//
	//Output parameter:
	//ImageScale: the resulting scaled image.
	//
	if (0 != ((hv_Min.TupleLength()) == 2))
	{
		hv_LowerLimit = ((const HTuple&)hv_Min)[1];
		hv_Min = ((const HTuple&)hv_Min)[0];
	}
	else
	{
		hv_LowerLimit = 0.0;
	}
	if (0 != ((hv_Max.TupleLength()) == 2))
	{
		hv_UpperLimit = ((const HTuple&)hv_Max)[1];
		hv_Max = ((const HTuple&)hv_Max)[0];
	}
	else
	{
		hv_UpperLimit = 255.0;
	}
	//
	//Calculate scaling parameters.
	hv_Mult = ((hv_UpperLimit - hv_LowerLimit).TupleReal()) / (hv_Max - hv_Min);
	hv_Add = ((-hv_Mult)*hv_Min) + hv_LowerLimit;
	//
	//Scale image.
	ScaleImage(ho_Image, &ho_Image, hv_Mult, hv_Add);
	//
	//Clip gray values if necessary.
	//This must be done for each image and channel separately.
	GenEmptyObj(&(*ho_ImageScaled));
	CountObj(ho_Image, &hv_NumImages);
	{
		HTuple end_val49 = hv_NumImages;
		HTuple step_val49 = 1;
		for (hv_ImageIndex = 1; hv_ImageIndex.Continue(end_val49, step_val49); hv_ImageIndex += step_val49)
		{
			SelectObj(ho_Image, &ho_ImageSelected, hv_ImageIndex);
			CountChannels(ho_ImageSelected, &hv_Channels);
			{
				HTuple end_val52 = hv_Channels;
				HTuple step_val52 = 1;
				for (hv_ChannelIndex = 1; hv_ChannelIndex.Continue(end_val52, step_val52); hv_ChannelIndex += step_val52)
				{
					AccessChannel(ho_ImageSelected, &ho_SelectedChannel, hv_ChannelIndex);
					MinMaxGray(ho_SelectedChannel, ho_SelectedChannel, 0, &hv_MinGray, &hv_MaxGray,
						&hv_Range);
					Threshold(ho_SelectedChannel, &ho_LowerRegion, (hv_MinGray.TupleConcat(hv_LowerLimit)).TupleMin(),
						hv_LowerLimit);
					Threshold(ho_SelectedChannel, &ho_UpperRegion, hv_UpperLimit, (hv_UpperLimit.TupleConcat(hv_MaxGray)).TupleMax());
					PaintRegion(ho_LowerRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_LowerLimit,
						"fill");
					PaintRegion(ho_UpperRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_UpperLimit,
						"fill");
					if (0 != (hv_ChannelIndex == 1))
					{
						CopyObj(ho_SelectedChannel, &ho_ImageSelectedScaled, 1, 1);
					}
					else
					{
						AppendChannel(ho_ImageSelectedScaled, ho_SelectedChannel, &ho_ImageSelectedScaled
							);
					}
				}
			}
			ConcatObj((*ho_ImageScaled), ho_ImageSelectedScaled, &(*ho_ImageScaled));
		}
	}
	return;

}

void imgCalY8::avgFilter(unsigned char* src, unsigned char* dst, int width, int height)
{
	memcpy(dst, src, width * height);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int sumValue = 0;
			int inT = max(0, i - 2);
			int inB = min(height - 1, i + 2);
			int inL = max(0, j - 2);
			int inR = min(width - 1, j + 2);
			for (int m = inT; m <= inB; ++m)
				for (int n = inL; n <= inR; n++)
				{
					sumValue += src[m*width + n];
				}
			dst[i * width + j] = sumValue / ((inB - inT + 1)*(inR - inL + 1));
		}
	}
}

void imgCalY8::avgFilter(unsigned short* src, unsigned short* dst, int width, int height)
{
	memcpy(dst, src, width * height * sizeof(short));
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int sumValue = 0;
			int inT = max(0, i - 2);
			int inB = min(height - 1, i + 2);
			int inL = max(0, j - 2);
			int inR = min(width - 1, j + 2);
			for (int m = inT; m <= inB; ++m)
				for (int n = inL; n <= inR; n++)
				{
					sumValue += src[m*width + n];
				}
			dst[i * width + j] = sumValue / ((inB - inT + 1)*(inR - inL + 1));
		}
	}
}

void imgCalY8::segThreshold(unsigned char* src, unsigned char* dst, int width, int height)
{
	int inLen = 2;
	int outLen = 49;
	memset(dst, 0, width * height);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int inT = max(0, i - inLen);
			int inB = min(height - 1, i + inLen);
			int inL = max(0, j - inLen);
			int inR = min(width - 1, j + inLen);

			int outT = max(0, i - outLen);
			int outB = min(height - 1, i + outLen);
			int outL = max(0, j - outLen);
			int outR = min(width - 1, j + outLen);

			int inValue = 0, outValue = 0;
			for (int y = inT; y <= inB; ++y)
			{
				for (int x = inL; x <= inR; ++x)
				{
					inValue += src[y * width + x];
				}
			}
			for (int y = outT; y <= outB; ++y)
			{
				for (int x = outL; x <= outR; ++x)
				{
					outValue += src[y * width + x];
				}
			}
			inValue /= ((inB - inT + 1)*(inR - inL+1));
			outValue /= ((outB - outT+1)*(outR - outL+1));

			if (outValue - inValue > DIRT_CHECK_THRESHOLD)
				dst[i * width + j] = 100;
			else if (inValue - outValue > DIRT_CHECK_THRESHOLD)
				dst[i * width + j] = 255;

		}
	}
}

void imgCalY8::segThreshold_Time(unsigned char* src, unsigned char* dst, int width, int height)
{
	int inLen = 2;
	int outLen = 49;
	memset(dst, 0, width * height);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int inT = max(0, i - inLen);
			int inB = min(height - 1, i + inLen);
			int inL = max(0, j - inLen);
			int inR = min(width - 1, j + inLen);

			int outT = max(0, i - outLen);
			int outB = min(height - 1, i + outLen);
			int outL = max(0, j - outLen);
			int outR = min(width - 1, j + outLen);

			int inValue = 0, outValue = 0;
			for (int y = inT; y <= inB; ++y)
			{
				for (int x = inL; x <= inR; ++x)
				{
					inValue += src[y * width + x];
				}
			}
			for (int y = outT; y <= outB; ++y)
			{
				for (int x = outL; x <= outR; ++x)
				{
					outValue += src[y * width + x];
				}
			}
			inValue /= ((inB - inT + 1)*(inR - inL + 1));
			outValue /= ((outB - outT + 1)*(outR - outL + 1));

			if (outValue - inValue > DIRT_SECOND_CHECK_THRESHOLD)
				dst[i * width + j] = 100;
			else if (inValue - outValue > DIRT_SECOND_CHECK_THRESHOLD)
				dst[i * width + j] = 255;

		}
	}
}

void imgCalY8::segThreshold(unsigned short* src, unsigned char* dst, int width, int height)
{
	int inLen = 2;
	int outLen = 49;
	memset(dst, 0, width * height);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int inT = max(0, i - inLen);
			int inB = min(height - 1, i + inLen);
			int inL = max(0, j - inLen);
			int inR = min(width - 1, j + inLen);

			int outT = max(0, i - outLen);
			int outB = min(height - 1, i + outLen);
			int outL = max(0, j - outLen);
			int outR = min(width - 1, j + outLen);

			int inValue = 0, outValue = 0;
			for (int y = inT; y <= inB; ++y)
			{
				for (int x = inL; x <= inR; ++x)
				{
					inValue += src[y * width + x];
				}
			}
			for (int y = outT; y <= outB; ++y)
			{
				for (int x = outL; x <= outR; ++x)
				{
					outValue += src[y * width + x];
				}
			}
			inValue /= ((inB - inT + 1)*(inR - inL + 1));
			outValue /= ((outB - outT + 1)*(outR - outL + 1));

			if (outValue - inValue > DIRT_CHECK_THRESHOLD)
				dst[i * width + j] = 100;
			else if (inValue - outValue > DIRT_CHECK_THRESHOLD)
				dst[i * width + j] = 255;

		}
	}
}

void imgCalY8::secondSegThreshold(unsigned char* orginSrc, unsigned char* src, std::vector<tRect>& outRect, int width, int height)
{
	outRect.clear();
	cv::Mat srcImg(height, width, CV_8UC1, src);
	cv::Mat out, stats, centroids;
	int number = cv::connectedComponentsWithStats(srcImg, out, stats, centroids, 8, CV_16U);
	int outCount = 0;
//	std::vector<int>id;
	tRect rect;
	for (int i = 0; i < number; ++i)
	{
		int x = stats.at<int>(i, CC_STAT_LEFT);
		int y = stats.at<int>(i, CC_STAT_TOP);
		int w_ = stats.at<int>(i, CC_STAT_WIDTH);
		int h_ = stats.at<int>(i, CC_STAT_HEIGHT);
		int area = stats.at<int>(i, CC_STAT_AREA);

		if (w_ > 10 && w_< 320 && h_ > 10 && h_<256 )
		{
			int count = 0;
			if (x <= 1)count++;
			if (x + w_ >= width - 1)count++;
			if (y <= 1)count++;
			if (y + h_ >= height - 1)count++;
			if (count >= 2)
				continue;
			rect.x = x;
			rect.y = y;
			rect.w = w_;
			rect.h = h_;
			int c_x = centroids.at<double>(i, 0);
			int c_y = centroids.at<double>(i, 1);
			int id_ = out.at<ushort>(c_y, c_x);
//			id.push_back(out.at<ushort>(c_y, c_x));
			if(isDirtCheck(orginSrc, (unsigned short*)out.data, rect, id_, width, height))
			{
				outRect.push_back(rect);
				outCount++;
			}
		}
	}
}

void imgCalY8::secondSegThreshold(unsigned short* orginSrc, unsigned char* src, std::vector<tRect>& outRect, int width, int height)
{
	outRect.clear();
	cv::Mat srcImg(height, width, CV_8UC1, src);
	cv::Mat out, stats, centroids;
	int number = cv::connectedComponentsWithStats(srcImg, out, stats, centroids, 8, CV_16U);
	int outCount = 0;
	//	std::vector<int>id;
	tRect rect;
	for (int i = 0; i < number; ++i)
	{
		int x = stats.at<int>(i, CC_STAT_LEFT);
		int y = stats.at<int>(i, CC_STAT_TOP);
		int w_ = stats.at<int>(i, CC_STAT_WIDTH);
		int h_ = stats.at<int>(i, CC_STAT_HEIGHT);
		int area = stats.at<int>(i, CC_STAT_AREA);

		if (w_ > 10 && w_< 320 && h_ > 10 && h_<256)
		{
			int count = 0;
			if (x <= 1)count++;
			if (x + w_ >= width - 1)count++;
			if (y <= 1)count++;
			if (y + h_ >= height - 1)count++;
			if (count >= 2)
				continue;
			rect.x = x;
			rect.y = y;
			rect.w = w_;
			rect.h = h_;
			int c_x = centroids.at<double>(i, 0);
			int c_y = centroids.at<double>(i, 1);
			int id_ = out.at<ushort>(c_y, c_x);
			//			id.push_back(out.at<ushort>(c_y, c_x));
			if (isDirtCheck(orginSrc, (unsigned short*)out.data, rect, id_, width, height))
			{
				outRect.push_back(rect);
				outCount++;
			}
		}
	}

}

bool imgCalY8::isDirtCheck(unsigned char* orginSrc, unsigned short* src, tRect rect, int id, int width, int height)
{
	int mLeft = rect.x - 50;
	int mRight = rect.x + 50 + rect.w;
	int mTop = rect.y - 50;
	int mBotton = rect.y + 50 + rect.h;
	if (mLeft >= 0 && mRight < width&&mTop >= 0 && mBotton < height)
		return true;
	int inValue = 0, outValue = 0;
	int inCount = 0, outCount = 0;
	int outLeft = rect.x - 5;
	outLeft = std::max(0, outLeft);
	int outRight= rect.x + 5 + rect.w;
	outRight = std::min(width-1, outRight);
	int outTop = rect.y - 5;
	outTop = std::max(0, outTop);
	int outBotton = rect.y + 5 + rect.h;
	outBotton = std::min(height-1, outBotton);
	for (int i = outTop; i <= outBotton; ++i)
	{
		for (int j = outLeft; j <= outRight; ++j)
		{
			if(i<rect.y || i> rect.h+rect.y || j< rect.x|| j> rect.x+rect.w)
			{
				outValue += orginSrc[i * width + j];
				outCount++;
			}
			else if(src[i * width + j] == id)
			{
				inValue += orginSrc[i * width + j];
				inCount++;
			}
		}
	}
	if (inCount == 0 || outCount == 0)return false;
	inValue /= inCount;
	outValue /= outCount;
	if (abs(inValue - outValue) > DIRT_AROUND_CHECK_THRESHOLD)
		return true;
	return false;
}

bool imgCalY8::isDirtCheck(unsigned short* orginSrc, unsigned short* src, tRect rect, int id, int width, int height)
{
	int mLeft = rect.x - 50;
	int mRight = rect.x + 50 + rect.w;
	int mTop = rect.y - 50;
	int mBotton = rect.y + 50 + rect.h;
	if (mLeft >= 0 && mRight < width&&mTop >= 0 && mBotton < height)
		return true;
	int inValue = 0, outValue = 0;
	int inCount = 0, outCount = 0;
	int outLeft = rect.x - 5;
	outLeft = std::max(0, outLeft);
	int outRight = rect.x + 5 + rect.w;
	outRight = std::min(width - 1, outRight);
	int outTop = rect.y - 5;
	outTop = std::max(0, outTop);
	int outBotton = rect.y + 5 + rect.h;
	outBotton = std::min(height - 1, outBotton);
	for (int i = outTop; i <= outBotton; ++i)
	{
		for (int j = outLeft; j <= outRight; ++j)
		{
			if (i<rect.y || i> rect.h + rect.y || j< rect.x || j> rect.x + rect.w)
			{
				outValue += orginSrc[i * width + j];
				outCount++;
			}
			else if (src[i * width + j] == id)
			{
				inValue += orginSrc[i * width + j];
				inCount++;
			}
		}
	}
	if (inCount == 0 || outCount == 0)return false;
	inValue /= inCount;
	outValue /= outCount;
	if (abs(inValue - outValue) > DIRT_AROUND_CHECK_THRESHOLD)
		return true;
	return false;

}

void imgCalY8::gaussFilter(unsigned char* src, unsigned char* dst, int width, int height)
{
	float kernel[9];
	kernel[0] = kernel[2] = kernel[6] = kernel[8] = 0.0947416f;
	kernel[1] = kernel[3] = kernel[5] = kernel[7] = 0.118318f;
	kernel[4] = 0.147761f;
	memcpy(dst, src, width * height * sizeof(char));

	for (int i = 1; i < height - 1; ++i)
	{
		for (int j = 1; j < width - 1; ++j)
		{
			float sum = 0.f;
			int index = 0;
			for (int m = i - 1; m <= i + 1; ++m)
				for (int n = j - 1; n <= j + 1; ++n)
					sum += kernel[index++] * src[m * width + n];
			dst[i * width + j] = sum;
		}
	}
}

void imgCalY8::sharpFilter(unsigned char* orginSrc, unsigned char* filterSrc, unsigned char* dst, int width, int height)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int tmp = abs(orginSrc[i * width + j] - filterSrc[i * width + j]);
			dst[i * width + j] = tmp;
		}
	}
}

void imgCalY8::midFilter(unsigned char* src, unsigned char* dst, int kernelSize, int width, int height)
{
	int halfSize = kernelSize / 2;
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int left, right, top, bottom;
			left = std::max(0, j - halfSize);
			right = std::min(width - 1, j + halfSize);
			top = std::max(0, i - halfSize);
			bottom = std::min(height - 1, i + halfSize);
			dst[i * width + j] = midData(src, left, right, top, bottom, width);

//			std::cout << "i: " << i << " j: " << j << std::endl;
		}
	}
}

void imgCalY8::gradFilter(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 0; i < height - 2; ++i)
	{
		for (int j = 0; j < width - 2; ++j)
		{
			int xData = abs(src[i * width + j] - src[i * width + j + 2]);
			int yData = abs(src[i * width + j] - src[(i + 2) * width + j]);
			int tmpData = sqrt(xData * xData + yData * yData);
			tmpData = std::min(tmpData, 255);
			tmpData = tmpData > DIRT_CHECK_THRESHOLD ? 255 : 0;
			dst[i * width + j] = tmpData;
		}
	}
}

void imgCalY8::maxFilter(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width *  height);
	for (int i = 1; i < height - 1; ++i)
	{
		for (int j = 1; j < width - 1; ++j)
		{
			int maxValue = 0;
			for (int m = i - 1; m <= i + 1; ++m)
			{
				for (int n = j - 1; n <= j + 1; ++n)
				{
					if (maxValue < src[m * width + n])
						maxValue = src[m * width + n];
				}
			}
			dst[i * width + j] = maxValue;
		}
	}
}

void imgCalY8::verMidFilter(unsigned char* src, unsigned char* dst, int filterLen, int width, int height)
{
	int halfLen = filterLen / 2;
	memcpy(dst, src, width * height);
	unsigned char* tmpData = new unsigned char[filterLen];
	for (int i = halfLen; i < height - halfLen; ++i)
	{
		for (int j = halfLen; j < width - halfLen; ++j)
		{
			for (int z = 0; z < filterLen; ++z)
			{
				tmpData[z] = src[(i + z - halfLen) * width + j];
			}
			dst[i * width + j] = midDataSingle(tmpData, filterLen); 
		}
	}
	delete[]tmpData;
}

void imgCalY8::hovMidFilter(unsigned char* src, unsigned char* dst, int filterLen, int width, int height)
{
	int halfLen = filterLen / 2;
	memcpy(dst, src, width * height);
	unsigned char* tmpData = new unsigned char[filterLen];
	for (int i = halfLen; i < height - halfLen; ++i)
	{
		for (int j = halfLen; j < width - halfLen; ++j)
		{
			for (int z = 0; z < filterLen; ++z)
			{
				tmpData[z] = src[i * width + (j + z - halfLen)];
			}
			dst[i * width + j] = midDataSingle(tmpData, filterLen);
		}
	}
	delete[]tmpData;
}

void imgCalY8::verGradCheckLow(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 2; i < height - 2; i++)
	{
		for (int j = 2; j < width - 2; ++j)
		{
			int checkValue1 = src[width * i + j] - src[width * (i) - 1 + j];
			int checkValue2 = src[width * i + j] - src[width * (i) + 1 + j];
			if (abs(checkValue1) > SECOND_THRESHOLD || abs(checkValue2) > SECOND_THRESHOLD)
			{
				if (abs(checkValue1) >= abs(checkValue2))
				{
					if (checkValue1 > 0)
						dst[i * width + j] = 255;
					else
						dst[i * width + j] = 100;
				}
				else
					if (checkValue2 > 0)
						dst[i * width + j] = 255;
					else
						dst[i * width + j] = 100;
			}

		}
	}
}

void imgCalY8::checkVerBadLineLow(unsigned char* src, std::vector<badLineData>&verIndex, int width, int height)
{
	for (int j = 2; j < width - 2; ++j)
	{
		int lowCount = 0;
		int highCount = 0;
		badLineData lowLine, highLine;
		bool lowStart = false, highStart = false;
		for (int i = 2; i < height - 2; ++i)
		{
			unsigned char value = src[i * width + j];
			if (value == 100)
			{
				if (!lowStart)
				{
					lowStart = true;
					lowLine.line = j;
					lowLine.startIndex = i;
				}
				else
					lowLine.endIndex = i;
				lowCount++;
			}
			else if (value == 255) 
			{
				if (!highStart)
				{
					highStart = true;
					highLine.line = j;
					highLine.startIndex = i;
				}
				else
					highLine.endIndex = i;
				highCount++;
			}
		}
//		std::cout << "j: " << j << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;

		if (lowCount > highCount)
		{
			if (lowCount > height / 3 && highCount < height / 20)
			{
				verIndex.push_back(lowLine);
				std::cout << "j: " << j << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;
			}
		}
		else
		{
			if (highCount > height / 3 && lowCount < height / 20)
			{
				verIndex.push_back(highLine);
				std::cout << "j: " << j << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;
			}
		}
		
	}
}

void imgCalY8::verGradCheckHigh(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 2; i < height - 2; i++)
	{
		for (int j = 2; j < width - 2; ++j)
		{
			unsigned char avg_value = (src[width * (i)-1 + j] + src[width * (i)+1 + j]) / 2;
			if (avg_value - src[width * i + j] > FIRST_THRESHOLD)
				dst[i * width + j] = 100;
			else if (src[width * i + j] - avg_value > FIRST_THRESHOLD)
				dst[i * width + j] = 255;
		}
	}
}

void imgCalY8::checkVerBadLineHigh(unsigned char* src, std::vector<badLineData>& verIndex, int width, int height)
{
//	verIndex.clear();
	Mat srcImg(height, width, CV_8UC1, src);
	for (int j = 2; j < width - 2; ++j)
	{
		int lowCount = 0;
		int highCount = 0;
		badLineData lowLine, highLine;
		bool lowStart = false, highStart = false;
		for (int i = 2; i < height - 2; ++i)
		{
			unsigned char value = src[i * width + j];
			if (value == 100) 
			{
				if (!lowStart)
				{
					lowStart = true;
					lowLine.startIndex = i;
					lowLine.line = j;
				}
				else
					lowLine.endIndex = i;
				lowCount++;
			}
			else if (value == 255)
			{
				if (!highStart)
				{
					highStart = true;
					highLine.startIndex = i;
					highLine.line = j;
				}
				else
					highLine.endIndex = i;
				highCount++;
			}
		}
				std::cout << "j: " << j << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;

		if (lowCount > highCount)
		{
			if (lowCount > 50 && lowCount > (lowLine.endIndex-lowLine.startIndex) * 0.05)
			{
				verIndex.push_back(lowLine);
			}
		}
		else
		{
			if (highCount > 50 && highCount > (highLine.endIndex - highLine.startIndex) * 0.05)
			{
				verIndex.push_back(highLine);
			}
		}

	}
}

void imgCalY8::horGradCheckLow(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 2; i < height - 2; i++)
	{
		for (int j = 2; j < width - 2; ++j)
		{
			int checkValue1 = src[width * i + j] - src[width * (i-1) + j];
			int checkValue2 = src[width * i + j] - src[width * (i+1) + j];
			if (abs(checkValue1) > SECOND_THRESHOLD || abs(checkValue2) > SECOND_THRESHOLD)
			{
				if (abs(checkValue1) >= abs(checkValue2))
				{
					if (checkValue1 > 0)
						dst[i * width + j] = 255;
					else
						dst[i * width + j] = 100;
				}
				else
					if (checkValue2 > 0)
						dst[i * width + j] = 255;
					else
						dst[i * width + j] = 100;
			}
		}
	}
}

void imgCalY8::checkHorBadLineLow(unsigned char* src, std::vector<badLineData>& verIndex, int width, int height)
{
	for (int i = 2; i < height - 2; ++i)
	{
		int lowCount = 0;
		int highCount = 0;
		badLineData lowLine, highLine;
		bool lowStart = false, highStart = false;
		for (int j = 2; j < width - 2; ++j)
		{
			unsigned char value = src[i * width + j];
			if (value == 100)
			{
				if (!lowStart)
				{
					lowStart = true;
					lowLine.line = i;
					lowLine.startIndex = j;
				}
				else
					lowLine.endIndex = j;
				lowCount++;
			}
			else if (value == 255)
			{
				if (!highStart)
				{
					highStart = true;
					highLine.line = i;
					highLine.startIndex = j;
				}
				else
					highLine.endIndex = j;
				highCount++;
			}
		}
		//		std::cout << "j: " << j << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;

		if (lowCount > highCount)
		{
			if (lowCount > width / 3 && highCount < width / 20)
			{
				verIndex.push_back(lowLine);
				std::cout << "i: " << i << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;
			}
		}
		else
		{
			if (highCount > height / 3 && lowCount < height / 20)
			{
				verIndex.push_back(highLine);
				std::cout << "i: " << i << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;
			}
		}
	}
}

void imgCalY8::horGradCheckHigh(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 2; i < height - 2; i++)
	{
		for (int j = 2; j < width - 2; ++j)
		{
			unsigned char avg_value = (src[width * (i-1) + j] + src[width * (i+1) + j]) / 2;
			if (avg_value - src[width * i + j] > FIRST_THRESHOLD)
				dst[i * width + j] = 100;
			else if (src[width * i + j] - avg_value > FIRST_THRESHOLD)
				dst[i * width + j] = 255;
		}
	}

}

void imgCalY8::checkHorBadLineHigh(unsigned char* src, std::vector<badLineData>& verIndex, int width, int height)
{
	for (int i = 2; i < height - 2; ++i)
	{
		int lowCount = 0;
		int highCount = 0;
		badLineData lowLine, highLine;
		bool lowStart = false, highStart = false;
		for (int j = 2; j < width - 2; ++j)
		{
			unsigned char value = src[i * width + j];
			if (value == 100)
			{
				if (!lowStart)
				{
					lowStart = true;
					lowLine.startIndex = j;
					lowLine.line = i;
				}
				else
					lowLine.endIndex = j;
				lowCount++;
			}
			else if (value == 255)
			{
				if (!highStart)
				{
					highStart = true;
					highLine.startIndex = j;
					highLine.line = i;
				}
				else
					highLine.endIndex = j;
				highCount++;
			}
		}
		std::cout << "i: " << i << " lowCount: " << lowCount << " highCount: " << highCount << std::endl;

		if (lowCount > highCount)
		{
			if (lowCount > 50 && lowCount > (lowLine.endIndex - lowLine.startIndex) * 0.2)
			{
				verIndex.push_back(lowLine);
			}
		}
		else
		{
			if (highCount > 50 && highCount > (highLine.endIndex - highLine.startIndex) * 0.2)
			{
				verIndex.push_back(highLine);
			}
		}

	}
}

unsigned char imgCalY8::midData(unsigned char* src, int left, int right, int top, int bottom, int width)
{
	unsigned char ret_ = 0;
	int count = (right - left + 1) * (bottom - top + 1);
	int mid = count / 2;
	unsigned char *tmpData = new unsigned char[count];
	for (int i = top; i <= bottom; ++i)
	{
		memcpy(tmpData + (i - top) * (right - left + 1), src + i * width + left, right - left + 1);
	}
	for (int i = 0; i < count - 1; ++i)
	{
		for (int j = count - 1; j > i; --j)
		{
			if (tmpData[j] > tmpData[j - 1])
			{
				unsigned char t = tmpData[j];
				tmpData[j] = tmpData[j - 1];
				tmpData[j - 1] = t;
			}
		}
		if (i == mid)
		{
			ret_ = tmpData[i];
			break;
		}
	}
	if (tmpData)
		delete[]tmpData;
	return ret_;
}

unsigned char imgCalY8::midDataSingle(unsigned char* src, int len)
{
	unsigned char out = 0;
	int half = len / 2;
	for (int i = 0; i < len - 1; ++i)
	{
		for (int j = len - 1; j > i; --j)
		{
			if (src[j] > src[j - 1])
			{
				unsigned char t = src[j];
				src[j] = src[j - 1];
				src[j - 1] = t;
			}
		}
		if (i == half)
		{
			out = src[i];
			break;
		}
	}
	return out;
}

void imgCalY8::getThresholdData(unsigned char* src, unsigned char* dst, int width, int height)
{
	memset(dst, 0, width * height);
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			float leftSum = 0, rightSum = 0, topSum = 0, bottomSum = 0;
			for (int m = CHECK_START_LEN; m <= CHECK_END_LEN; ++m)
			{
				if (j - m - SUM_LEN >= 0)
				{
					float tempL = 0;
					for (int k = j - m - SUM_LEN; k < j - m; ++k)
						tempL += src[k + i * width];
					tempL /= SUM_LEN;
					leftSum = std::max(tempL, leftSum);
				}
				if (j + m + SUM_LEN < width)
				{
					float tempR = 0;
					for (int k = j + m; k < j + m + SUM_LEN; ++k)
						tempR += src[k + i * width];
					tempR /= SUM_LEN;
					rightSum = std::max(tempR, rightSum);
				}

				if (i - m - SUM_LEN >= 0)
				{
					float tempT = 0;
					for (int k = i - m - SUM_LEN; k < i - m; ++k)
						tempT += src[j + k * width];
					tempT /= SUM_LEN;
					topSum = std::max(tempT, topSum);
				}
				if (i + m + SUM_LEN < height)
				{
					float tempB = 0;
					for (int k = i + m; k < i + m + SUM_LEN; ++k)
						tempB += src[j + k * width];
					tempB /= SUM_LEN;
					bottomSum = std::max(tempB, bottomSum);
				}

				int value = src[j + i * width];
				if (abs(value - leftSum) >= CHECK_THRESHOLD || abs(value - rightSum) >= CHECK_THRESHOLD||
					abs(value - topSum) >= CHECK_THRESHOLD || abs(value - bottomSum) >= CHECK_THRESHOLD)
				{
					dst[width * i + j] = 255;
				}
			}
		}
	}
}
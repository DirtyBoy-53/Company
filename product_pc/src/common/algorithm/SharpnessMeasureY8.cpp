#include "SharpnessMeasureY8.h"

void SharpnessMeasure(sSharpnessValue* sSV,unsigned char* nSrc, int nWidth, int nHeight, int x, int y, int nRoiWidth, int nRoiHeight)
{
	if ((nRoiHeight + y > nHeight) || (nRoiWidth + x > nWidth) || nRoiHeight < 3 || nRoiWidth < 3)
	{
		return;
	}

	int i, j,w,h,nPixelNum;
	float nSharpValue;
	int nRoiLen = nRoiHeight * nRoiWidth;
	unsigned char* Roi = (unsigned char *)malloc(nRoiLen * sizeof(unsigned char));
	memset(Roi, 0, nRoiLen * sizeof(unsigned char));

	//左上区域
	for(i = 0;i<nRoiHeight;i++)
	{
		for (j = 0; j < nRoiWidth; j++)
		{
			Roi[i * nRoiWidth + j] = nSrc[(x + i) * nWidth + y + j];
		}
	}

	nPixelNum = 0;
	nSharpValue = 0;
	for (i = 1; i<nRoiHeight-1; i++)
		for (j = 1; j < nRoiWidth-1; j++)
		{
			nPixelNum++;
			for(h = -1;h <= 1;h++)
				for (w = -1; w <= 1; w++)
				{
					nSharpValue += abs(Roi[i*nRoiWidth + j] - Roi[(i + h)*nRoiWidth + j + w]);
				}
		}
	sSV->SharpnessLU = nSharpValue / (nPixelNum * 8);
	 
	//右上区域
	for(i = 0;i<nRoiHeight;i++)
	{
		for (j = 0; j < nRoiWidth; j++)
		{
			Roi[i * nRoiWidth + j] = nSrc[(x + i) * nWidth + y + j];
		}
	}

	nPixelNum = 0;
	nSharpValue = 0;
	for (i = 1; i<nRoiHeight - 1; i++)
		for (j = 1; j < nRoiWidth - 1; j++)
		{
			nPixelNum++;
			for (h = -1; h <= 1; h++)
				for (w = -1; w <= 1; w++)
				{
					nSharpValue += abs(Roi[i*nRoiWidth + j] - Roi[(i + h)*nRoiWidth + j + w]);
				}
		}
	sSV->SharpnessRU = nSharpValue / (nPixelNum * 8);

	//中心区域
	for(i = 0;i<nRoiHeight;i++)
	{
		for (j = 0; j < nRoiWidth; j++)
		{
			Roi[i * nRoiWidth + j] = nSrc[(x + i) * nWidth + y + j];
		}
	}

	nPixelNum = 0;
	nSharpValue = 0;
	for (i = 1; i<nRoiHeight - 1; i++)
		for (j = 1; j < nRoiWidth - 1; j++)
		{
			nPixelNum++;
			for (h = -1; h <= 1; h++)
				for (w = -1; w <= 1; w++)
				{
					nSharpValue += abs(Roi[i*nRoiWidth + j] - Roi[(i + h)*nRoiWidth + j + w]);
				}
		}
	sSV->SharpnessCenter = nSharpValue / (nPixelNum * 8);

	//左下区域
	for(i = 0;i<nRoiHeight;i++)
	{
		for (j = 0; j < nRoiWidth; j++)
		{
			Roi[i * nRoiWidth + j] = nSrc[(x + i) * nWidth + y + j];
		}
	}
	nPixelNum = 0;
	nSharpValue = 0;
	for (i = 1; i<nRoiHeight - 1; i++)
		for (j = 1; j < nRoiWidth - 1; j++)
		{
			nPixelNum++;
			for (h = -1; h <= 1; h++)
				for (w = -1; w <= 1; w++)
				{
					nSharpValue += abs(Roi[i*nRoiWidth + j] - Roi[(i + h)*nRoiWidth + j + w]);
				}
		}
	sSV->SharpnessLD = nSharpValue / (nPixelNum * 8);

	//右下区域
	for(i = 0;i<nRoiHeight;i++)
	{
		for (j = 0; j < nRoiWidth; j++)
		{
			Roi[i * nRoiWidth + j] = nSrc[(x + i) * nWidth + y + j];
		}
	}
	nPixelNum = 0;
	nSharpValue = 0;
	for (i = 1; i<nRoiHeight - 1; i++)
		for (j = 1; j < nRoiWidth - 1; j++)
		{
			nPixelNum++;
			for (h = -1; h <= 1; h++)
				for (w = -1; w <= 1; w++)
				{
					nSharpValue += abs(Roi[i*nRoiWidth + j] - Roi[(i + h)*nRoiWidth + j + w]);
				}
		}
	sSV->SharpnessRD = nSharpValue / (nPixelNum * 8);

	free(Roi);
	Roi = NULL;
}




#include "checkbox.h"


static bool isColor(const unsigned char* uyvyData, int width, int height, int stride, int x, int y, int colorThreshold) {

	// Check if the point(x,y) is within the image boundaries
	if (x < 0 || x >= width || y < 0 || y >= height)
		return false;

	// Calculate the size of a single UYVY pixel
	int pixelSize = 2;

	// Define the threshold values for gray color
	const unsigned char grayUVValue = 0x80;

	// Calculate the index of the current pixel in the UYVY data
	int index = y * stride + x * pixelSize;

	// Get the U, Y, V values of the current pixel
	unsigned char uvValue = uyvyData[index];
	unsigned char yValue = uyvyData[index + 1];

	// Calculate the color difference between U and V components
	int colorDifference = uvValue - grayUVValue;
	colorDifference = colorDifference > 0 ? colorDifference : -colorDifference;

	// Check if the color difference exceeds the threshold
	if (colorDifference > colorThreshold)
		return true; // Color box detected
	else
		return false; // No color box detected
}


std::vector<MyRectangle> findColorRectangles(const unsigned char* uyvyData, int width, int height, int stride, int colorThreshold) {

    std::vector<MyRectangle> rectangles;
	std::vector<bool> isRectangle(width*height, false);

	// Iterate over each pixel in the image
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// Check if the point within a rectangle
			if (isRectangle.at(y*width+x) == true) continue;

			// Check if the color difference exceeds the threshold
			if (isColor(uyvyData, width, height, stride, x, y, colorThreshold)) {
				// Check if the current pixel is the top-left corner of a rectangle
				if (x + 1 < width && y + 1 < height) {
					// Check if the next pixel in the same row also exceeds the threshold
					if (isColor(uyvyData, width, height, stride, x + 1, y, colorThreshold)) {
						// Check if the next pixel in the same column also exceeds the threshold
						if (isColor(uyvyData, width, height, stride, x, y + 1, colorThreshold)) {
							// Calculate the width and height of the rectangle
							int rectangleWidth = 2;
							int rectangleHeight = 2;

							// Expand the rectangle horizontally
							while (isColor(uyvyData, width, height, stride, x + rectangleWidth, y, colorThreshold)) {
								rectangleWidth++;
							}

							// Expand the rectangle vertically
							while (isColor(uyvyData, width, height, stride, x, y + rectangleHeight, colorThreshold)) {
								rectangleHeight++;
							}

							// Check the center of the rectangle is void
							int centerX = x + rectangleWidth / 2;
							int centerY = y + rectangleHeight / 2;

							// Create a rectangle object and add it to the vector
							if (!isColor(uyvyData, width, height, stride, centerX, centerY, colorThreshold))//¿ò±ÕºÏ£¬ÉáÆú
							{
                                MyRectangle rectangle;
								rectangle.x = x;
								rectangle.y = y;
								rectangle.width = rectangleWidth;
								rectangle.height = rectangleHeight;
								rectangles.push_back(rectangle);

								for (auto j = 0; j < rectangleHeight; ++j)
									for (auto i = 0; i < rectangleWidth; ++i)
										isRectangle.at((y+j)*width + (x+i)) = true;

							}

						}
					}
				}
			}
		}
	}

	return rectangles;
}


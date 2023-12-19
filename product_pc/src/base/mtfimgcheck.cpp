#include "mtfimgcheck.h"
#include <string.h>

mtfImgCheck::mtfImgCheck()
{

}

void mtfImgCheck::Get_Y16_hist(unsigned short *input_data,  unsigned short &max_value, unsigned short &min_value, int width, int height)
{
    int *hist_data = new int[1 << 16];
    memset(hist_data, 0, sizeof(int) * (1 << 16));
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            hist_data[input_data[i * width + j]] += 1;
        }
    }
    int sum_count = 0;
    for (int i = (1 << 16) - 1; i > 0; i--)
    {
        sum_count += hist_data[i];
        if (sum_count > 1000)
        {
            max_value = i;
            break;
        }
    }
    sum_count = 0;
    for (int i = 0; i < (1 << 16); ++i)
    {
        sum_count += hist_data[i];
        if (sum_count > 10000)
        {
            min_value = i;
            break;
        }
    }
    delete[]hist_data;
}

void mtfImgCheck::Get_target_count(unsigned short *input_data, int *target_count, unsigned short max_value, unsigned short min_value, int width, int height)
{
    unsigned char* target_value = new unsigned char[width * height];
    memset(target_count, 0, sizeof(int) * 5);
    memset(target_value, 0, sizeof(char) * width * height);
    unsigned short threshold_value = (max_value + min_value) / 2;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            target_value[i * width + j] = input_data[i * width + j] > threshold_value ? 1 : 0;
        }
    }
    int start_xs[5] = { 245,30,245,470,255 };
    int start_ys[5] = { 15,180,180,180,350 };
    int w = 150, h = 150;

    for (int i = 0; i < 5; ++i)
    {
        for (int y = start_ys[i]; y < start_ys[i] + h; ++y)
        {
            for (int x = start_xs[i]; x < start_xs[i] + w; ++x)
            {
                if (target_value[x + y * width] > 0)
                    target_count[i]++;
            }
        }
    }

    delete[]target_value;
}

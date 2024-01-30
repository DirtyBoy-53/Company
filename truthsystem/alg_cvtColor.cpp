#include "alg_cvtColor.h"
#include "stdint.h"

#define OMP_NUM_THREADS 8
namespace alg_cv {

#define LIMIT_VAL_HIGH(val, max) (val) > (max) ? (max) : (val)
#define LIMIT_VAL_LOW(val, min) (val) < (min) ? (min) : (val)

#define ALG_CV_YUV2RGBCVT_SHIFT 20
#define ALG_CV_YUV2RGBCVT_CY 1220542  // 1.164*1024*1024
#define ALG_CV_YUV2RGBCVT_CVR 1673527 // 1.596*1024*1024
#define ALG_CV_YUV2RGBCVT_CVG -852492 // -0.813*1024*1024
#define ALG_CV_YUV2RGBCVT_CUG -409993 // -0.391*1024*1024
#define ALG_CV_YUV2RGBCVT_CUB 2116026 // 2.018*1024*1024

color_space_e image_format( int data_type )
{
    switch(data_type)
    {
        case 0    :  return ALG_CV_YUV2RGBCVT_DEFAULT;
        case 0x18 :  return ALG_CV_YUV2RGBCVT_UVY2;
        case 0x19 :  return ALG_CV_YUV2RGBCVT_VUY2;
        case 0x1A :  return ALG_CV_YUV2RGBCVT_Y2UV;
        case 0x1B :  return ALG_CV_YUV2RGBCVT_Y2VU;
        case 0x1C :  return ALG_CV_YUV2RGBCVT_UYVY;
        case 0x1D :  return ALG_CV_YUV2RGBCVT_VYUY;
        case 0x1E :  return ALG_CV_YUV2RGBCVT_YUYV;
        case 0x1F :  return ALG_CV_YUV2RGBCVT_YVYU;
        case 0x2B :  return ALG_CV_Bayer2RGB_RAW10;
        case 0x2C :  return ALG_CV_Bayer2RGB_RAW12;
        default : return ALG_CV_YUV2RGBCVT_ERR;
    }
}

void yuv422_rgb888_converter (unsigned char* img_src, unsigned char* img_dst, uint32_t index, uint32_t pos, int* yuv_idx)
{
    int y1 = int(img_src[4*index + yuv_idx[0]] - 16) * ALG_CV_YUV2RGBCVT_CY;
    int y2 = int(img_src[4*index + yuv_idx[1]] - 16) * ALG_CV_YUV2RGBCVT_CY;
    int u =  int(img_src[4*index + yuv_idx[2]]) - 128;
    int v =  int(img_src[4*index + yuv_idx[3]]) - 128;

    int y10 = LIMIT_VAL_LOW(y1, 0);
    int y20 = LIMIT_VAL_LOW(y2, 0);
    int ruv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CVR * v, 0);
    int guv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CVG * v + ALG_CV_YUV2RGBCVT_CUG * u, 0);
    int buv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CUB * u, 0);

    img_dst[pos + 0] = LIMIT_VAL_HIGH((y10 + ruv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    img_dst[pos + 1] = LIMIT_VAL_HIGH((y10 + guv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    img_dst[pos + 2] = LIMIT_VAL_HIGH((y10 + buv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    img_dst[pos + 3] = LIMIT_VAL_HIGH((y20 + ruv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    img_dst[pos + 4] = LIMIT_VAL_HIGH((y20 + guv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    img_dst[pos + 5] = LIMIT_VAL_HIGH((y20 + buv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
}

void set_yuv_index (int* yuv_idx, color_space_e color_space)
{
    if (!yuv_idx)
    {
        return;
    }

    if (color_space == ALG_CV_YUV2RGBCVT_DEFAULT)
    {
        yuv_idx[0] = 0;
        yuv_idx[1] = 2;
        yuv_idx[2] = 1;
        yuv_idx[3] = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_UYVY)
    {
        yuv_idx[0] = 1;
        yuv_idx[1] = 3;
        yuv_idx[2] = 0;
        yuv_idx[3] = 2;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_VYUY)
    {
        yuv_idx[0] = 1;
        yuv_idx[1] = 3;
        yuv_idx[2] = 2;
        yuv_idx[3] = 0;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_YUYV)
    {
        yuv_idx[0] = 0;
        yuv_idx[1] = 2;
        yuv_idx[2] = 1;
        yuv_idx[3] = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_YVYU)
    {
        yuv_idx[0] = 0;
        yuv_idx[1] = 2;
        yuv_idx[2] = 3;
        yuv_idx[3] = 1;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_UVY2)
    {
        yuv_idx[0] = 3;
        yuv_idx[1] = 1;
        yuv_idx[2] = 0;
        yuv_idx[3] = 2;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_VUY2)
    {
        yuv_idx[0] = 3;
        yuv_idx[1] = 1;
        yuv_idx[2] = 2;
        yuv_idx[3] = 0;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_Y2UV)
    {
        yuv_idx[0] = 2;
        yuv_idx[1] = 0;
        yuv_idx[2] = 1;
        yuv_idx[3] = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_Y2VU)
    {
        yuv_idx[0] = 2;
        yuv_idx[1] = 0;
        yuv_idx[2] = 3;
        yuv_idx[3] = 1;
    }
}

void cvt_yuv422_rgb888_loop (unsigned char* img_src, unsigned char* img_dst, int section, uint32_t data_size_div, int* yuv_idx)
{
    uint32_t pos = 6*data_size_div*section;
    for(uint32_t index = section*data_size_div; index < (section+1)*data_size_div; index++, pos+=6)
    {
        yuv422_rgb888_converter(img_src, img_dst, index, pos, yuv_idx);
    }
}

void cvt_yuv422_rgb888_impl (unsigned char* img_src, unsigned char* img_dst, const uint32_t data_size, color_space_e color_space)
{
    int yuv_idx[4];
    set_yuv_index(yuv_idx, color_space);
#if defined(WITH_OPENMP)
    omp_set_num_threads(OMP_NUM_THREADS);
    const uint32_t data_size_div = data_size/2/OMP_NUM_THREADS;

    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 1, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 2, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 3, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 4, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 5, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 6, data_size_div, yuv_idx);
            }
            #pragma omp section
            {
                cvt_yuv422_rgb888_loop(img_src, img_dst, 7, data_size_div, yuv_idx);
            }
        }
    }
#else
    const uint32_t data_size_div = data_size/2;
    cvt_yuv422_rgb888_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
#endif
}

void alg_sdk_cvtColor(void* src, void* dst, int w, int h, color_space_e color_space)
{
    if (!src || !dst)
    {
        return;
    }
    unsigned char* img_src = (unsigned char*)src;
    unsigned char* img_dst = (unsigned char*)dst;
    const uint32_t data_size = w * h;

    switch(color_space)
    {
        case ALG_CV_YUV2RGBCVT_DEFAULT:
        case ALG_CV_YUV2RGBCVT_UVY2: case ALG_CV_YUV2RGBCVT_VUY2:
        case ALG_CV_YUV2RGBCVT_Y2UV: case ALG_CV_YUV2RGBCVT_Y2VU:
        case ALG_CV_YUV2RGBCVT_UYVY: case ALG_CV_YUV2RGBCVT_VYUY:
        case ALG_CV_YUV2RGBCVT_YUYV: case ALG_CV_YUV2RGBCVT_YVYU:
            cvt_yuv422_rgb888_impl(img_src, img_dst, data_size, color_space);
        default:
            break;
    }
}
}


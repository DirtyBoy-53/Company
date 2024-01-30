/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _ALG_CVTCOLOR_H_
#define _ALG_CVTCOLOR_H_

#if defined(WITH_OPENMP)
#include "omp.h"
#endif

namespace alg_cv {
    enum color_space_e {
        ALG_CV_YUV2RGBCVT_ERR = 0xFF,
        ALG_CV_YUV2RGBCVT_DEFAULT = 0x0,
        ALG_CV_YUV2RGBCVT_UVY2 = 0x18,
        ALG_CV_YUV2RGBCVT_VUY2 = 0x19,
        ALG_CV_YUV2RGBCVT_Y2UV = 0x1A,
        ALG_CV_YUV2RGBCVT_Y2VU = 0x1B,
        ALG_CV_YUV2RGBCVT_UYVY = 0x1C,
        ALG_CV_YUV2RGBCVT_VYUY = 0x1D,
        ALG_CV_YUV2RGBCVT_YUYV = 0x1E,
        ALG_CV_YUV2RGBCVT_YVYU = 0x1F,
        ALG_CV_Bayer2RGB_RAW10 = 0x2B,
        ALG_CV_Bayer2RGB_RAW12 = 0x2C,
    };
    void alg_sdk_cvtColor(void* src, void* dst, int w, int h, color_space_e color_space);
    color_space_e image_format(int data_type);
}


#endif

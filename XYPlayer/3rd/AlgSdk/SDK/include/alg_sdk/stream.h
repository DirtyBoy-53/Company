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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MINGW32) && defined(BUILD_EXPORT)
    #ifdef ALG_SDK_EXPORT
        #define ALG_SDK_API __declspec(dllexport)
    #else
        #define ALG_SDK_API __declspec(dllimport)
    #endif // ALG_SDK_EXPORT
#else
    #define ALG_SDK_API extern
#endif // MINGW32
#define ALG_STREAM_USAGE_STRING  "Stream options: \n" 								\
		  "  protocol=display         type of supported protocol, arguments are :\n"		    \
		  "                             * display                  (Display with OpenGL)\n"		\
		  "                             * v4l2                     (V4L2 camera)\n"		        \
		  "                             * rtp                      (RTP stream)\n"				\
		  "                             * rtmp                     (RTMP stream)\n"				\
		  "                             * file                     (save to video file)\n"	    \
		  "                             * record                   (display and save to file)\n"\
		  "  parameters usuages : \n\n"                                                           \
		  "  v4l2_device=/dev/video0  v4l2 device name, for example :\n"			            \
		  "                             * /dev/video*              (Virtual v4l2 device name)\n"\
		  "  codec_type=h264          supported codec type, for example :\n"		            \
		  "                             * raw = raw data (no codec)\n"						    \
		  "                             * h264 = h264 codec (need CUDA)\n"						\
		  "                             * h265 = h265 codec (need CUDA)\n"                      \
		  "  host_ip=127.0.0.1        rtp host ip address, for example :\n"		                \
		  "                             * 127.0.0.1\n"                                          \
		  "  port_num=5000            rtp port number, for example :\n"		   		            \
		  "                             * 5000\n"                                               \
		  "  file_location=.          video file save location, for example :\n"		        \
		  "                             * .  (current folder)\n"                                \
		  "  rtmp_addr=.              rtmp address, for example :\n"					        \
		  "                             * rtmp://127.0.0.1:1935/live/\n\n"                         
 
#define ALG_SDK_STREAM_STRING_SIZE 256
typedef struct _GstElement GstElement;
typedef struct _GstBus GstBus;
typedef struct _GstCaps GstCaps;

typedef struct alg_sdk_gst_stream
{
    char launch_str[ALG_SDK_STREAM_STRING_SIZE * 4];
    char protocol[ALG_SDK_STREAM_STRING_SIZE];
    char host_ip[ALG_SDK_STREAM_STRING_SIZE];
    char port_num[ALG_SDK_STREAM_STRING_SIZE];
    char codec_type[ALG_SDK_STREAM_STRING_SIZE];
    char cap_str[ALG_SDK_STREAM_STRING_SIZE];
    char v4l2_device[ALG_SDK_STREAM_STRING_SIZE];
    char encoder_h264[ALG_SDK_STREAM_STRING_SIZE];
    char encoder_h265[ALG_SDK_STREAM_STRING_SIZE];
    char file_location[ALG_SDK_STREAM_STRING_SIZE];
    char decoder_h264[ALG_SDK_STREAM_STRING_SIZE];
    char decoder_h265[ALG_SDK_STREAM_STRING_SIZE];
    char rtmp_addr[ALG_SDK_STREAM_STRING_SIZE];

    int width;
    int height;
    int frame_rate;
    int format;
    int ch_id;
    int rtmp_port;
    int have_sei;
    int gpu_id;

    uint32_t frame_index;
    uint32_t last_frame_index;
    uint64_t timestamp;

    bool need_data;
    bool need_sei_data;
    bool mStreaming;  

    GstBus *mBus;
    GstElement *mAppSrc;
    GstElement *mPipeline;
    GstCaps *mBufferCaps;
    GstElement *mAppSEISrc;

    void *mImageRender;
} alg_sdk_gst_stream_t;

ALG_SDK_API int alg_sdk_init_streamer(int argn, char** arg);
ALG_SDK_API int alg_sdk_push2stream(const void* msg, const int ch_id);
ALG_SDK_API int alg_sdk_start_streamer();
ALG_SDK_API int alg_sdk_stop_streamer();
const char* alg_sdk_stream_usage();
ALG_SDK_API int alg_sdk_stream_on_start (const int ch_id);
ALG_SDK_API int alg_sdk_stream_on_stop (const int ch_id);
ALG_SDK_API int alg_sdk_stream_parse_args(int argn, char **arg, void *_streamer, int ch_id);
ALG_SDK_API int alg_sdk_stream_init_pipeline(void *_streamer);
ALG_SDK_API int alg_sdk_stream_deinit_pipeline(void *_streamer);
ALG_SDK_API bool alg_sdk_open_streamer(void *_streamer);
ALG_SDK_API void alg_sdk_close_streamer(void *_streamer);
ALG_SDK_API int alg_sdk_stream_encode_image(void *buffer, int width, int height, void *_streamer, int input_format, uint32_t fr, uint64_t ts);
ALG_SDK_API bool alg_sdk_stream_gstreamer_init();

#ifdef __cplusplus
}
#endif
#endif

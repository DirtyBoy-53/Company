#ifndef XYZJVIDEO_H
#define XYZJVIDEO_H

#include "YThread.h"
#include "VideoPlayer.h"

#include <opencv2/opencv.hpp>


#include <atomic>
QT_FORWARD_DECLARE_CLASS(VideoRecord)
class XYZJVideo : public VideoPlayer, public YThread
{
public:
    explicit XYZJVideo();
    ~XYZJVideo();

    virtual int start() {
        quit = 0;
        return YThread::start();
    }
    virtual int stop() {
        quit = 1;
        runFlag = false;
        return YThread::stop();
    }
    virtual int pause() {return YThread::pause();}
    virtual int resume() {return YThread::resume();}

    virtual int seek(int64_t ms);
    void addData(XYZJFramePtr frame);

// VideoPlayer interface
private:
    virtual bool doPrepare();
    virtual void doTask();
    virtual bool doFinish();

    bool open();
    bool close();

    void startRecord();
    void stopRecord();

    void shot(YFramePtr yframe);
    bool frameParse(const XYZJFramePtr frame, YFramePtr yframe);
    bool YuvParse(const XYZJFramePtr frame, YFramePtr yframe);
public:
    int64_t block_starttime;
    int64_t block_timeout;
    int     quit;

    void write(XYZJFramePtr frame);
    void write(YFrame *yframe);
    void write(QImage &img, QString path);
private:
    static std::atomic_flag s_ffmpeg_init;

//    AVDictionary*       fmt_opts;
//    AVDictionary*       codec_opts;

//    AVFormatContext*    fmt_ctx;
//    AVCodecContext*     codec_ctx;

//    AVPacket* packet;
//    AVFrame* frame;

    XYZJFramePtr frame;
    YFrame       rgbFrame;
    uint64_t recvCnt{0};

    int video_stream_index;
    int audio_stream_index;
    int subtitle_stream_index;

    int video_time_base_num;
    int video_time_base_den;

    // for scale
//    AVPixelFormat   src_pix_fmt;
//    AVPixelFormat   dst_pix_fmt;
//    SwsContext*     sws_ctx;
    uint8_t*        data[4];
    int             linesize[4];
    YFrame          hframe;

    uint8_t*           y16{nullptr};
    bool            runFlag{true};

    VideoRecord*    videoHandle{nullptr};

};

#endif // PCIEVIDEO_H

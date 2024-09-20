#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include "Media.h"
#include "YFrame.h"
#include "GQueue.h"
#include "YStruct.h"
#include "YFunction.h"
#include <QDebug>
#include <atomic>
enum player_event_e {
    PLAYER_OPEN_FAILED,
    PLAYER_OPENED,
    PLAYER_EOF,
    PLAYER_CLOSED,
    PLAYER_ERROR,
};
typedef int (*player_event_cb)(player_event_e e, void* userdata);

class VideoPlayer
{
public:
    VideoPlayer() {

        width = 0;
        height = 0;
        duration = 0;
        start_time = 0;
        eof = 0;
        error = 0;
        event_cb = nullptr;
    }

    virtual ~VideoPlayer() {}

    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int pause() = 0;
    virtual int resume() = 0;
    virtual void startRecord() = 0;
    virtual void stopRecord() = 0;
    virtual void addData(XYZJFramePtr frame) = 0;


    virtual int seek(int64_t ms) {

        return 0;
    }

    bool isRecord(){
        return isSaveVideo;
    }
    void set_media(Media& media) {
        this->media = media;
    }

    void set_decode_mode(int mode) {
        decode_mode = mode;
    }
    void screenShot(){
        setShot(true);
    }

    void setShot(bool state) {
        isShot = state;
    }
    // FrameStats get_frame_stats() {
    //     return frame_buf.frame_stats;
    // }

    // FrameInfo get_frame_info() {
    //     return frame_buf.frame_info;
    // }

    // void set_frame_cache(int cache) {
    //     frame_buf.setCache(cache);
    // }

    // void clear_frame_cache() {
    //     frame_buf.clear();
    // }
    size_t get_frame_size(){
        return yframe_buf.size();
    }
    int push_frame(YFramePtr& pFrame) {
        // return frame_buf.push(pFrame);
        // qDebug() << "push_frame curSize:" << yframe_buf.size();
        yframe_buf.push(pFrame);
    }

    bool pop_frame(YFramePtr& pFrame) {
        // return frame_buf.pop(pFrame);
        // qDebug() << "pop_frame curSize:" <<yframe_buf.size();
        if(yframe_buf.size() > 100){
            for(auto i = 0;i < 99;i++){
                yframe_buf.wait_and_pop_fortime(pFrame);
            }
            yprint(QString("Cache pressure too hight[size=%1],discarding 99 data items.").arg(yframe_buf.size()), PRINT_RED);
        }
        return yframe_buf.wait_and_pop_fortime(pFrame);
    }

    void set_event_callback(player_event_cb cb, void* userdata) {
        event_cb = cb;
        event_cb_userdata = userdata;
    }

    void event_callback(player_event_e e) {
        if (event_cb) {
            event_cb(e, event_cb_userdata);
        }
    }

public:
    Media               media;
    int                 fps{0};
    int                 decode_mode;
    int                 real_decode_mode;

    int32_t     width;
    int32_t     height;
    int64_t     duration;   // ms
    int64_t     start_time; // ms
    int         eof;
    int         error;
    std::atomic<bool>   isShot{false};
protected:
    GQueue<XYZJFramePtr>        queue;
    // YFrameBuf                   frame_buf;
    GQueue<YFramePtr>           yframe_buf;
    player_event_cb             event_cb;
    void*                       event_cb_userdata;
    bool        isSaveVideo{false};
};


#endif // VIDEOPALYER_H

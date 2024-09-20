#ifndef YFRAME_H
#define YFRAME_H

#include <deque>
#include <mutex>
#include <memory>
#include "YBuf.h"

class YFrame {
public:
    YBuf buf;
    YBuf param;
    int w{0};
    int h{0};
    int depth{0};
    int type{0};
    int ySize{0};
    int uSize{0};
    int vSize{0};
    int fps{0};
    uint64_t ts{0};
    int64_t useridx{-1};
    void* userdata{nullptr};

    bool isNull() {
        return w == 0 || h == 0 || buf.isNull();
    }
    void setYuvSize(const int &y,const int &u,const int &v){
        ySize = y;
        uSize = u;
        vSize = v;
    }
    char* getY(){
        return buf.base;
    }
    char* getU(){
        return (buf.base+ySize);
    }
    char* getV(){
        return (buf.base + ySize + uSize);
    }
    char* data(){
        return buf.base;
    }
    void copy(const YFrame& rhs) {
        w = rhs.w;
        h = rhs.h;
        depth = rhs.depth;
        type = rhs.type;
        ySize = rhs.ySize;
        uSize = rhs.uSize;
        vSize = rhs.vSize;
        fps = rhs.fps;
        ts = rhs.ts;
        useridx = rhs.useridx;
        userdata = rhs.userdata;
        buf.copy(rhs.buf.base, rhs.buf.len);
        param.copy(rhs.param.base, rhs.param.len);
    }

    // YFrame(size_t newSize){
    //     buf.resize(newSize);
    // }

    ~YFrame(){
        if(!isNull()){
            buf.cleanup();
            param.cleanup();
        }
    }
    void clear(){
        w={0};
        h={0};
        depth={0};
        type={0};
        ySize={0};
        uSize={0};
        vSize={0};
        fps={0};
        ts={0};
        useridx={-1};
        userdata={nullptr};
        buf.cleanup();
        param.cleanup();
    }

};
typedef std::shared_ptr<YFrame> YFramePtr;

typedef struct frame_info_s {
    int w;
    int h;
    int type;
    int depth;
} FrameInfo;

typedef struct frame_stats_s {
    int push_cnt;
    int pop_cnt;

    int push_ok_cnt;
    int pop_ok_cnt;

    frame_stats_s() {
        push_cnt = pop_cnt = push_ok_cnt = pop_ok_cnt = 0;
    }
} FrameStats;

#define DEFAULT_FRAME_CACHENUM  50

class YFrameBuf : public YRingBuf {
 public:
    enum CacheFullPolicy {
        SQUEEZE,
        DISCARD,
    } policy;

    YFrameBuf() : YRingBuf() {
        cache_num = DEFAULT_FRAME_CACHENUM;
        policy = SQUEEZE;
    }

    void setCache(int num) {cache_num = num;}
    void setPolicy(CacheFullPolicy policy) {this->policy = policy;}

    int push(YFrame* pFrame);
    int pop(YFrame* pFrame);
    void clear();

    int         cache_num;
    FrameStats  frame_stats;
    FrameInfo   frame_info;
    std::deque<YFrame> frames;
    std::mutex         mutex;
};
#endif // YFRAME_H

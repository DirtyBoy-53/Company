#ifndef YSTRUCT_H
#define YSTRUCT_H

#include <memory>
#include <exception>
#include <string.h>
#include <exception>
#include <QColor>
#include <QString>

#include "YFrame.h"
#include "YDefine.h"

class CYStructException : public std::exception
{
public:
    CYStructException(const std::string &msg) : m_msg(msg.c_str()) {}
    const char* what() const throw() { return m_msg.c_str(); }

private:
    std::string m_msg;
};
// 视频默认  帧头   + 算法数据 + YUV视频 + Y16视频 + 参数行  + 可见光视频
//         128帧头 + nvsLen + yuvLen + y16Len + paramLen + vlLen
struct FrameInfo_S{
    uint64_t uHeadFlag{0x80808080efefefef};//0xefefefef80808080
    int32_t uTotalLen{0};
    int64_t uTimeTm{0};
    int16_t uNvsLen{0};
    int32_t uY16Len{0};
    int32_t uYuvLen{0};
    int32_t uParamLen{0};
    uint8_t uFormat{0}; //1 yuv422 3 yuv16 5 y16+yuv 6 yuv420  7 x16  0xffff 原始数据
    int8_t uScale{0};//1 正常  2 2倍 4 4倍
    int8_t vlFormat{0};//1 yuv422p y+u+v 2 yuv420 3 h264
    int32_t vlLen{0};
    int16_t vlWidth{0};
    int16_t vlHeight{0};
    int16_t width{0};
    int16_t height{0};
    int32_t imuLen{0};
    uint8_t ext[40]{0};//环境CollectInfo
    int endian() {
        if(uHeadFlag == 0xefefefef80808080) {
            return 0;
        } else if(uHeadFlag == 0x80808080efefefef) {
            return 1;
        } else {
            return -1;
        }
    }
    FrameInfo_S(){
        uHeadFlag = 0x80808080efefefef;
        uTotalLen = 0;uTimeTm = 0;uNvsLen = 0;uY16Len = 0;
        uYuvLen = 0;uParamLen = 0;uFormat = 0;uScale = 0;vlFormat = 0;vlLen = 0;vlWidth = 0;
        vlHeight = 0;width = 0;height = 0;imuLen = 0;memset(ext,0,sizeof(uint8_t)*40);
    }
};
#include "ylog.h"
struct XYZJFrame {
    FrameInfo_S head;
    size_t      size{ 0 };
    size_t      paramOffset{ 0 };
    char*    data{ nullptr };
    XYZJFrame(){}
    XYZJFrame(size_t newSize){
        if(data == nullptr && size == 0){
            data = (char*)malloc(newSize);
            // YLog::Logger->info("XYZJFrame data:{}", data);
            if(data) size = newSize;
            else throw CYStructException("XYZJFrame memory allocation failed.");
        }
    }
    bool resize(size_t newSize){
        if (data == nullptr || newSize > size) {
            data = (char*)realloc(data, newSize);
        }
        if(data == nullptr) {
            throw CYStructException("XYZJFrame resize memory reallocation failed.");
            return false;//may not be executed.
        }
        size = newSize;
        return true;
    }
    ~XYZJFrame(){
        if(data){
            free(data); data = nullptr;
        }
    }
};
typedef std::shared_ptr<XYZJFrame> XYZJFramePtr;

// 读取寄存器返回
struct readReg_S{
    uint8_t value[1024]{0};
    size_t len{0};
    uint8_t channel;
    bool ret{false};
};

typedef struct aspect_ratio_s {
    aspect_ratio_e type;
    int w, h;
} aspect_ratio_t;

//水印信息
struct CustomWatermarkInfo {
    QString id;
    int x;
    int y;
    int fontSize;
    QColor color;
    QString value;

    QString toString(){
        return QString("id:%1 x:%2 y:%3 size:%4 color:%5").arg(id).arg(x).arg(y).arg(fontSize).arg(color.name());
    }
};

#endif // YSTRUCT_H

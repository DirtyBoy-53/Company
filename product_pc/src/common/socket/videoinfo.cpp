#include "videoinfo.h"

VideoInfo::VideoInfo()
{
    for(int i=0; i<4; i++) {
        VideoFormatInfo info;
        m_info.push_back(info);
    }
}

VideoInfo::~VideoInfo()
{

}

VideoInfo* VideoInfo::getInstance()
{
    static VideoInfo instance;
    return &instance;
}

void VideoInfo::getVideoInfo(const int handle, VideoFormatInfo& info)
{
    if(handle >= 4) return;
    info = m_info[handle];
}

void VideoInfo::setVideoInfo(const int handle, const VideoFormatInfo info)
{
    if(handle >= 4) return;
    m_info[handle] = info;
}

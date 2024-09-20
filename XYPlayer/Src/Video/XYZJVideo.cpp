#include "XYZJVideo.h"
#include <algorithm>
#include "YDefine.h"
#include "VideoMap.h"
#include "AlgSdkManager.h"
#include "confile.h"
#include "VideoRecord.h"
#include <fstream>
#include <QDebug>
#include "YuvToImg.h"
XYZJVideo::XYZJVideo()
{
    y16 = new uint8_t[VIDEO_FRAME_MAX_SIZE];
}

XYZJVideo::~XYZJVideo()
{
    if(y16) delete y16;
    y16 = nullptr;
    rgbFrame.buf.cleanup();
}

int XYZJVideo::seek(int64_t ms)
{

    return 0;
}

void XYZJVideo::addData(XYZJFramePtr frame)
{
    if(m_status == RUNNING){
        // qDebug() << "addData";
        queue.push(frame);
    }
}

bool XYZJVideo::doPrepare()
{
    int ret = open();
    if (ret != 0) {
        if (!quit) {
            error = ret;
            event_callback(PLAYER_OPEN_FAILED);
        }
        return false;
    }
    else {
        event_callback(PLAYER_OPENED);
    }
    return true;
}

//等待PCIE驱动传入数据（智能指针队列）
//解析数据后传入YFrame
void XYZJVideo::doTask()
{
    static bool writeFlag{false};
    if(media.type == MEDIA_TYPE_PCIE){
        while (queue.wait_and_pop_fortime(frame)) {
            recvCnt++;
            if((recvCnt % 1000) == 0){
                qDebug() << media.toString() << " RecvIdx:" << recvCnt << " queueSize:" << queue.size() << " frameBufSize:" << yframe_buf.size();
            }

            if(videoHandle && isRecord()) videoHandle->addFrame(frame);


            YFramePtr yframe = std::make_shared<YFrame>();
            if(frameParse(frame, yframe)){
                push_frame(yframe);

                if(isShot){
                    shot(yframe);
                    setShot(false);
                }

                if(!writeFlag){
                    writeFlag = true;

                    // write(yframe.get());
                }


            }
        }
    }
}

void XYZJVideo::write(XYZJFramePtr frame){
    QString filepath = g_exec_dir_path + "/yuv.raw";
    std::ofstream out;
    out.open(filepath.toStdString(), std::ios::out);
    out.write(frame->data, frame->size);
}

void XYZJVideo::write(YFrame *yframe){
    static YFrame frame;
    frame.copy(*yframe);
    QString filepath = g_exec_dir_path + "/blue-yuv.bin";
    std::ofstream out;
    out.open(filepath.toStdString(), std::ios::out | std::ios::app);
    // out.write(frame.buf.base, frame.buf.size());
    out.write(frame.getY(), frame.ySize);
    out.write(frame.getU(), frame.uSize);
    out.write(frame.getV(), frame.vSize);
}

void XYZJVideo::write(QImage &img, QString path)
{

    if(!img.save(path)){
        qDebug() << "write bmp failed";
    }

}


bool XYZJVideo::doFinish()
{
    bool ret{false};

    return ret;
}

//打开本地文件类型
//打开打开艾利光采集通道
//网络（暂不支持）
bool XYZJVideo::open()
{
    bool ret{false};
    ConsoleControl con;
    switch (media.type)
    {
    
    case MEDIA_TYPE_PCIE:
        //发送PCIE配置文件
        break;
    case MEDIA_TYPE_FILE: break;
    case MEDIA_TYPE_NETWORK:  break;
    default: break;
    }
    return ret;
}

//关闭本地文件类型
//关闭打开艾利光采集通道
//网络（暂不支持）
bool XYZJVideo::close()
{

    return true;
}

void XYZJVideo::startRecord()
{
    if(isSaveVideo) return;

    if(!videoHandle){
        delete videoHandle;
        videoHandle = nullptr;
    }

    videoHandle = new VideoRecord();
    auto videoPath = g_config->get<std::string>("videoSavePath", "root", (g_exec_dir_path + "/record_video/").toStdString());
    videoHandle->setPath(QString::fromStdString(videoPath));
    isSaveVideo = true;
    videoHandle->startRun();
    videoHandle->setMedia(media);
    videoHandle->start();
}

void XYZJVideo::stopRecord()
{
    if(!isSaveVideo) return;
    isSaveVideo = false;
    if(videoHandle){
        videoHandle->stop();
        videoHandle->quit();
        videoHandle->wait(500);
        videoHandle->close();
    }
}

void XYZJVideo::shot(YFramePtr yframe)
{
    try{
        YuvToImg toImg;
        QImage img;
        int size = yframe->w * yframe->h * 3;
        auto picPath = g_config->get<std::string>("videoSavePath", "root", (g_exec_dir_path + "/screen_shot/").toStdString());
        QString savePath =  QString::fromStdString(picPath) + "/Window" + QString::number(media.id) + "/screen_shot/";
        std::string path = g_config->get<std::string>("shotPath", "video", savePath.toStdString());
        savePath = QString::fromStdString(path);
        QString curTime = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz");
        QString fileForamt = QString("%1_%2.%3").arg(media.id).arg(curTime).arg("bmp");
        savePath += fileForamt;
        if(!confirmFile(savePath)) return;


        rgbFrame.buf.resize(size);
        if(toImg.yuv422pToRgb((uint8_t*)rgbFrame.data(), (uint8_t*)yframe->data(), yframe->w, yframe->h)){
            img = toImg.rgbToQImage((uint8_t*)rgbFrame.data(), yframe->w, yframe->h);
            write(img, savePath);
            yprint("shot path:"+savePath, PRINT_GRAY);
        }
    }catch(...){
        yprint("截图失败");
    }
}

#include <QDebug>
bool XYZJVideo::frameParse(const XYZJFramePtr frame, YFramePtr yframe)
{
    auto &head = frame->head;
    yframe->w = std::max(head.width, head.vlWidth);
    yframe->h = std::max(head.height, head.vlHeight);
    yframe->type = head.uFormat;
    yframe->depth = 12;
    fps = head.ext[1];
    yframe->fps = fps;



    // qDebug() << QString("depth:%1 w:%2 h:%3 size:%4").arg(yframe->depth).arg(yframe->w).arg(yframe->h).arg(frame->size);
    return YuvParse(frame,yframe);
}

bool XYZJVideo::YuvParse(const XYZJFramePtr frame, YFramePtr yframe)
{
    bool ret{false};
    auto &head = frame->head;
    int w = yframe->w, h = yframe->h;
    int size = w*h;
    size_t yCnt{0},uCnt{0},vCnt{0};
    auto format = std::max((int8_t)head.uFormat, head.vlFormat);
    swap_endian(frame->data, frame->size);//采集输出数据需要大小端置换

    //参数行
    if(frame->head.uParamLen > 0){
        if((uint8_t)frame->data[frame->paramOffset] == 0xAA && (uint8_t)frame->data[frame->paramOffset+1] == 0x55){
            yframe->param.resize(frame->head.uParamLen);
            memcpy(yframe->param.data(), frame->data + frame->paramOffset, frame->head.uParamLen);
        }

    }

    if(format == VIDEO_FORMAT_UYVY){                    //YUV422p = Y8
        yframe->buf.resize(size*2);
        yframe->setYuvSize(size,size/2,size/2);
        char* pY = yframe->getY(), *pU = yframe->getU(), *pV = yframe->getV();
        for(size_t i = 0;i < yframe->buf.size();){
            pU[uCnt++] = frame->data[i++];
            pY[yCnt++] = frame->data[i++];
            pV[vCnt++] = frame->data[i++];
            pY[yCnt++] = frame->data[i++];
        }
        ret = true;
    }else if(format == VIDEO_FORMAT_Y16){               //Y16
        yframe->buf.resize(size*2);
        // qDebug("size:%d  ParamLen:%d",frame->size, frame->head.uParamLen);

        Data16ToRGB24((short*)frame->data, y16, size, 0);
        rgb2yuv422Planar(y16, h, w, w*3, (uint8_t*)yframe->buf.base);
        yframe->setYuvSize(size,size/2,size/2);
        ret = true;
    }else if(format == VIDEO_FORMAT_X16){               //X16

    }else{}
    // qDebug() << QString::asprintf("end y=%d", yframe.buf.base[0]);
    // qDebug() << QString::asprintf("end y=%d u=%d y=%d v=%d", frame->data[0], frame->data[1], frame->data[2], frame->data[3]);
    return ret;
}


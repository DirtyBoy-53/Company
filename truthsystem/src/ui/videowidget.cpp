#include "videowidget.h"
#include "ui_videowidget.h"
#include <QDebug>
#include <QDateTime>
#include <omp.h>

#include "configinfo.h"
#include "mapping.h"
#include "yuv2rgb.h"
#include "memcache.h"
#include "util.h"
// #include "y16proc.h"
// #include "apihandler.h"
#include "videoinfo.h"




static cv::Mat g_mixVec = (cv::Mat_<double>(3, 3) << 4.27090269e-01, 4.68508926e-03, -6.12136022e+01,
                           -8.80133421e-03, 4.13823592e-01, 3.50490322e-01,
                           1.56102825e-05, -2.62500790e-05, 1.00000000e+00);

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    qDebug() << "VideoWidget init ==========";
    ui->setupUi(this);
    m_vlScale = 1;
    m_irScale = 1;

    m_irYuv = (uint8_t*)malloc(1024 * 1024 * 8);
    m_irRgb = (uint8_t*)malloc(1024 * 1024 * 30);
    m_vlRgb = (uint8_t*)malloc(1024 * 1024 * 30);
    m_vlYuv = (uint8_t*)malloc(1024 * 1024 * 20);

    ui->irVideo->setWidgetIndex(IR_VIDEO_ID);
    ui->vlVideo->setWidgetIndex(VL_VIDEO_ID);

    connect(ui->irVideo, &GLMyYuvWidget::signalMousePress, this, &VideoWidget::onMousePressEvent);
    connect(ui->vlVideo, &GLMyYuvWidget::signalMousePress, this, &VideoWidget::onMousePressEvent);
    connect(ui->irVideo, &GLMyYuvWidget::signalKeyDown, this, &VideoWidget::onKeyPressEvent);
    connect(ui->vlVideo, &GLMyYuvWidget::signalKeyDown, this, &VideoWidget::onKeyPressEvent);
    connect(ui->irVideo, &GLMyYuvWidget::signalMouseDoublePress, this, &VideoWidget::onMouseDoublePress);
    connect(ui->vlVideo, &GLMyYuvWidget::signalMouseDoublePress, this, &VideoWidget::onMouseDoublePress);
    connect(ui->irVideo, &GLMyYuvWidget::signalWheelEvent, this, &VideoWidget::onWheelEvent);
    connect(ui->vlVideo, &GLMyYuvWidget::signalWheelEvent, this, &VideoWidget::onWheelEvent);
    connect(StaticSignals::getInstance(), &StaticSignals::sigUpdateX16ShowB, this, &VideoWidget::slotUpdateX16B);

    m_cacheQueue.clear();
    m_isRunning = true;
    m_videoThread = new std::thread(&VideoWidget::videoProcess, this);
    connect(this, &VideoWidget::signalUpdateImg, this, &VideoWidget::slotUpdateImg);

    ui->vlVideo->setVisible(false);
    MemCache::instance()->setValue("zoomSwitch","1");
//    Y16Proc::instance();
    qDebug() << "VideoWidget init end==========";
}

VideoWidget::~VideoWidget()
{
    m_isRunning = false;
    qInfo()<<"start wait end...";
    if(m_videoThread) {
        m_videoThread->join();
        delete m_videoThread;
        m_videoThread = nullptr;
    }
    qInfo()<<"wait ended...";
    m_cacheQueue.clear();

    if(!m_irYuv) {
        free(m_irYuv);
        m_irYuv = nullptr;
    }
    if(!m_irRgb) {
        free(m_irRgb);
        m_irRgb = nullptr;
    }
    if(!m_vlRgb) {
        free(m_vlRgb);
        m_vlRgb = nullptr;
    }
    if(!m_vlYuv) {
        free(m_vlYuv);
        m_vlYuv = nullptr;
    }
    delete ui;
}

void VideoWidget::addWatermark(const int videoId, int waterMarkId, const WatermarkInfo& image)
{
    if(videoId == IR_VIDEO_ID) {
        m_irWaterMark[waterMarkId] = image;
    } else if(videoId == VL_VIDEO_ID) {
        m_vlWaterMark[waterMarkId] = image;
    }
}

void VideoWidget::removeWatermark(const int videoId, int waterMarkId)
{
    if(videoId == IR_VIDEO_ID) {
        m_irWaterMark.remove(waterMarkId);
    } else if(videoId == VL_VIDEO_ID) {
        m_vlWaterMark.remove(waterMarkId);
    }
}


void VideoWidget::setWatermarkVisible(const int videoId, int waterMarkId, const bool visible)
{
    if(videoId == IR_VIDEO_ID) {
        m_irWaterMark[waterMarkId].visible = visible;
    } else if(videoId == VL_VIDEO_ID) {
        m_vlWaterMark[waterMarkId].visible = visible;
    }
}

void VideoWidget::moveWatermark(const int videoId, int waterMarkId, const int x, const int y)
{
    if(videoId == IR_VIDEO_ID) {
        m_irWaterMark[waterMarkId].x = x;
        m_irWaterMark[waterMarkId].y = y;
    } else if(videoId == VL_VIDEO_ID) {
        m_vlWaterMark[waterMarkId].x = x;
        m_vlWaterMark[waterMarkId].y = y;
    }
}

void VideoWidget::onMousePressEvent(const int index, QPoint point)
{
    LayoutPressPoint pos = m_mapPoint[index];
    if(pos.videoH <= 0 || pos.layoutH <= 0) return;

    int zoomSwitch = MemCache::instance()->value("zoomSwitch").toInt();
    if(zoomSwitch != 1) {
        pos.clickX = point.x() - (pos.layoutW - pos.videoW)/2;
        pos.clickY = point.y() - (pos.layoutH - pos.videoH)/2;
        if(pos.layoutW < pos.videoW) {
            pos.clickX = point.x();
        } else if(pos.layoutH < pos.videoH) {
            pos.clickY = point.y();
        }
    } else {
        float frameRatio = (float)pos.videoW / (float)pos.videoH;
        float layoutRatio = (float)pos.layoutW / (float)pos.layoutH;
        int videoW, videoH;
        if(frameRatio > layoutRatio) {
            videoW = pos.layoutW;
            videoH = pos.layoutW / frameRatio;
            pos.clickX = point.x();
            pos.clickY = point.y() - (pos.layoutH - videoH)/2;
        } else if(frameRatio < layoutRatio) {
            videoW = pos.layoutH * frameRatio;
            videoH = pos.layoutH;
            pos.clickX = point.x() - (pos.layoutW - videoW)/2;
            pos.clickY = point.y();
        } else {
            pos.clickX = point.x();
            pos.clickY = point.y();
        }
        pos.clickX = pos.clickX*pos.videoW/videoW;
        pos.clickY = pos.clickY*pos.videoH/videoH;
    }

    if(pos.clickX >= 0 && pos.clickX < pos.videoW && pos.clickY >= 0 && pos.clickY < pos.videoH) {
        emit signalPressPos(index, pos.clickX, pos.clickY);
    }

    // 修改水印信息
    m_mapPoint[index] = pos;
    m_badpointClicked.setX(pos.clickX);
    m_badpointClicked.setY(pos.clickY);
    qDebug() << point << "x y " << pos.layoutW << pos.layoutH << pos.clickX << pos.clickY;
}

void VideoWidget::onKeyPressEvent(const int index, const int key)
{
    LayoutPressPoint pos = m_mapPoint[index];
    if(pos.videoH <= 0 || pos.layoutH <= 0) return;
    switch(key)
    {
    case Qt::Key_Up:
        pos.clickY -= 1;
        break;
    case Qt::Key_Down:
        pos.clickY += 1;
        break;
    case Qt::Key_Left:
        pos.clickX -= 1;
        break;
    case Qt::Key_Right:
        pos.clickX += 1;
        break;
    default:
        break;
    }

    qDebug() << index << pos.layoutW << pos.layoutH << pos.clickX << pos.clickY;
    if(pos.clickX <= 0) pos.clickX = 0;
    if(pos.clickX >= pos.videoW) pos.clickX = pos.videoW-1;
    if(pos.clickY <= 0) pos.clickY = 0;
    if(pos.clickY >= pos.videoH) pos.clickY = pos.videoH-1;
    emit signalPressPos(index, pos.clickX, pos.clickY);

    // 修改水印信息
    m_mapPoint[index] = pos;
    m_badpointClicked.setX(pos.clickX);
    m_badpointClicked.setY(pos.clickY);
    qDebug() << index << pos.layoutW << pos.layoutH << pos.clickX << pos.clickY;
}

void VideoWidget::onMouseDoublePress(const int index)
{
    if(!m_hasVlFrame) return;

    if(index == IR_VIDEO_ID) {
        ui->vlVideo->setVisible(!ui->vlVideo->isVisible());
    } else if(index == VL_VIDEO_ID) {
        ui->irVideo->setVisible(!ui->irVideo->isVisible());
    }
}

void VideoWidget::onWheelEvent(const int index, const int direction)
{
    if(index == IR_VIDEO_ID) {
        if(direction > 0) {
            if(m_irScale != 4) {
                m_irScale *= 2;
            }
        } else if(direction < 0) {
            if(m_irScale != 1) {
                m_irScale /= 2;
            }
        }
    } else if(index == VL_VIDEO_ID) {
        if(direction > 0) {
            if(m_vlScale != 4) {
                m_vlScale *= 2;
            }
        } else if(direction < 0) {
            if(m_vlScale != 1) {
                m_vlScale /= 2;
            }
        }
    }
}

void VideoWidget::irParser(FrameInfo_S frameInfo, unsigned char* frame, H264Decode* h264Decoder)
{
    int w = frameInfo.width;
    int h = frameInfo.height;

    MemCache::instance()->setValue("videoFormat", g_mapStr[frameInfo.uFormat]);

    uint8_t * pData = frame + frameInfo.uNvsLen;
    if(frameInfo.uFormat == VIDEO_FORMAT_YUV422P || frameInfo.uFormat == 0xffff) {
        Yuv2Rgb::YuvToRgb422p(pData, m_irRgb, w, h);
    }else if(frameInfo.uFormat == VIDEO_FORMAT_UYVY) {
        Yuv2Rgb::YuvToRgbUYVY(pData, m_irRgb, w, h);
    }else if(frameInfo.uFormat == VIDEO_FORMAT_YUYV ) {
        Yuv2Rgb::YuvToRgbYUYV(pData, m_irRgb, w, h);
    }else if(frameInfo.uFormat == VIDEO_FORMAT_YVYU ) {
        Yuv2Rgb::YuvToRgbYVYU(pData, m_irRgb, w, h);
    }else if(VIDEO_FORMAT_YUV420 == frameInfo.uFormat) {
        Yuv2Rgb::YuvToRgb420p(pData, m_irRgb, w, h);
    }else if(VIDEO_FORMAT_Y16 == frameInfo.uFormat) {
        Mapping map;
        map.Data16ToRGB24((short*)pData, m_irRgb, w*h, 0);
    } else if(frameInfo.uFormat == VIDEO_FORMAT_X16) {
        if(w == 640) {
            uint8_t * tmpData = (uint8_t *)frame + frameInfo.uNvsLen + w*4*2;
            h = 512;
            if(m_isUpdateB) {
                uint16_t* tmp = (uint16_t*)malloc(w*h*2);
                uint16_t* x16Data = (uint16_t*)tmpData;
                uint16_t* bData = (uint16_t*)m_bData.data();
                for(int i=0; i<w*h; i++) {
                    tmp[i] = x16Data[i] - bData[i] + 15000;
                }
                // Y16Proc::instance()->process((uint16_t * )tmp, w, h, m_irRgb, false);
                free(tmp);
            } else {
                // Y16Proc::instance()->process((uint16_t * )tmpData, w, h, m_irRgb, false);
            }
        } else {
            Mapping map;
            map.Data16ToRGB24((short*)pData, m_irRgb, w*h, 9);
        }
    } else if(frameInfo.uFormat == VIDEO_FORMAT_H264) {
        uint8_t* irY = m_irYuv;
        uint8_t* irU = irY + w * h;
        uint8_t* irV = irY + w * h + w * h/4;
        bool result = h264Decoder->h264ToYuv(pData, irY, irU, irV, frameInfo.uYuvLen, w, h);
        if(result) {
            Yuv2Rgb::YuvToRgb420p(m_irYuv, m_irRgb, w, h);
        }
    } else if(frameInfo.uFormat == VIDEO_FORMAT_Y) {
        uint8_t* irY = m_irYuv;
        uint8_t* irUV = m_irYuv + w*h;
        memcpy(irY, pData, w*h);
        memset(irUV, 0x80, w*h/2);
        Yuv2Rgb::YuvToRgb420p(m_irYuv, m_irRgb, w, h);
    }

    QImage img(m_irRgb, w, h ,QImage::Format_RGB888);

    bool hasNvsObj = false;
    if((MemCache::instance()->value("isShowNvs").toInt() != 0 ||
        MemCache::instance()->value("isRecordShowNvs").toInt()) &&
            frameInfo.uNvsLen > 0) {
        hasNvsObj = m_nvsHandler.hasNvsObj(QByteArray((const char*)frame, frameInfo.uNvsLen));
        QByteArray ba((const char*)frame, frameInfo.uNvsLen);
        m_nvsHandler.drawNvs(img, ba);
    }

//    if(frameInfo.imuLen > 0) {
//        uint8_t* imuData = frame + frameInfo.uNvsLen + frameInfo.uYuvLen + frameInfo.uY16Len + frameInfo.uParamLen;
//        m_imu.drawImu(img, (int16_t*)imuData, frameInfo.uTimeTm, w, h);
//    }

    m_mapPoint[IR_VIDEO_ID].layoutW = ui->irVideo->geometry().width();
    m_mapPoint[IR_VIDEO_ID].layoutH = ui->irVideo->geometry().height();
    m_mapPoint[IR_VIDEO_ID].videoW = w;
    m_mapPoint[IR_VIDEO_ID].videoH = h;

    for(auto it : m_irWaterMark) {
        if(it.visible) {
            QPainter painter;
            painter.begin(&img);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawImage(it.x, it.y, it.img);
            painter.end();
        }
    }
    QImage badpointImg = img.copy(m_badpointClicked.x()-10, m_badpointClicked.y()-10, 20, 20);
    emit signalBadpointImg(badpointImg);

    // MixAlgorithmInfo cfgInfo = ConfigInfo::instance()->getMixAlgorithmInfo();
    if(MemCache::instance()->value("isShowLine").toInt() == 1) {
        QPainter painter;
        painter.begin(&img);
        QPen pen;
        pen.setWidth(2);
        // pen.setColor(cfgInfo.lineColor);
        painter.setPen(pen);
        painter.drawLine(img.width()/2, 0, img.width()/2, img.height());
        painter.drawLine(0, img.height()/2, img.width(), img.height()/2);
        painter.end();
    }

    int tmpW = img.width() / m_irScale;
    int tmpH = img.height() / m_irScale;

    if(MemCache::instance()->value("reampSwitch").toInt() == 1 && w == 1280) {
        if(!m_isRemap) {
            m_isRemap = true;
            m_remap.init(QCoreApplication::applicationDirPath() + "/alg/map.xml");
        }
        img = m_remap.remap(img);
    }

    if(MemCache::instance()->value("showFromNvs").toInt() == 1 && !hasNvsObj && m_delayShowCount == 0) {
        QImage tmpImg(w, h ,QImage::Format_RGB888);
        tmpImg.fill(Qt::black);
        emit signalUpdateImg(IR_VIDEO_ID, tmpImg, tmpW, tmpH);
    } else {
        emit signalUpdateImg(IR_VIDEO_ID, img,tmpW, tmpH);
        m_delayShowCount++;
        if(m_delayShowCount == 5) m_delayShowCount = 0;
    }
}

void VideoWidget::vlParser(FrameInfo_S frameInfo, unsigned char *frame, H264Decode* h264Decoder)
{
    int vlW = frameInfo.vlWidth;
    int vlH = frameInfo.vlHeight;
    if(frameInfo.vlFormat == 1) {
        qDebug() << "Format:" << frameInfo.vlFormat <<"  W:" << vlW << "  H:" << vlH;
        int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
        uint8_t* vlY = (uint8_t*)frame + start;
        uint8_t* vlU = vlY + vlW * vlH;
        uint8_t* vlV = vlY + vlW * vlH + vlW * vlH/2;
        omp_set_num_threads(20);
        #pragma omp parallel for
        for(int i=0; i<vlW*vlH/2; i++) {
            m_vlYuv[4*i]   = vlU[i];
            m_vlYuv[4*i+1] = vlY[2*i];
            m_vlYuv[4*i+2] = vlV[i];
            m_vlYuv[4*i+3] = vlY[2*i+1];
        }
        Yuv2Rgb::YuvToRgbUYVY(m_vlYuv, m_vlRgb, vlW, vlH);
    } else if(frameInfo.vlFormat == 2) {
        int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
        Yuv2Rgb::YuvToRgb420p((uint8_t*)frame + start, m_vlRgb, vlW, vlH);
    } else if(frameInfo.vlFormat == 3) {
        unsigned char* vlYuv = m_vlYuv;
        unsigned char* vlU = m_vlYuv + vlW*vlH;
        unsigned char* vlV = m_vlYuv + vlW*vlH + vlW*vlH/4;;
        int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
        bool result = h264Decoder->h264ToYuv(frame + start, vlYuv, vlU, vlV, frameInfo.vlLen, vlW, vlH);
        if(result) {
            Yuv2Rgb::YuvToRgb420p(m_vlYuv, m_vlRgb, vlW, vlH) ;
        }
    }else if(frameInfo.vlFormat == 4){
        int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
        omp_set_num_threads(20);
        #pragma omp parallel for
        for (auto i = 0; i < vlW * vlH * 2; i += 4) {
            m_vlYuv[i] = frame[i + 1];//U
            m_vlYuv[i + 1] = frame[i];//Y1
            m_vlYuv[i + 2] = frame[i + 3];//V
            m_vlYuv[i + 3] = frame[i + 2];//Y2
        }
        Yuv2Rgb::YuvToRgbUYVY(m_vlYuv, m_vlRgb, vlW, vlH);
    }

    QImage vlImg(m_vlRgb, vlW, vlH, QImage::Format_RGB888);
    // 按需求裁剪成5:4的宽高比
    if(frameInfo.vlFormat == 3) {
        vlW = 1600;
        vlImg = vlImg.copy(160, 0, vlW, vlH);
    }

    m_mapPoint[VL_VIDEO_ID].layoutW = ui->vlVideo->geometry().width();
    m_mapPoint[VL_VIDEO_ID].layoutH = ui->vlVideo->geometry().height();
    m_mapPoint[VL_VIDEO_ID].videoW = vlW;
    m_mapPoint[VL_VIDEO_ID].videoH = vlH;


    int tmpW = vlImg.width() / m_vlScale;
    int tmpH = vlImg.height() / m_vlScale;
    emit signalUpdateImg(VL_VIDEO_ID, vlImg, tmpW, tmpH);
}

void VideoWidget::updateFrame(QByteArray data)
{
    m_cacheQueue.put(data);
}

void VideoWidget::slotUpdateX16B(QByteArray bData)
{
    m_bData = bData;
    m_isUpdateB = true;
}

void VideoWidget::getScreen()
{
    m_screenBtn = true;
}

void VideoWidget::slotUpdateImg(int videoId, QImage img, int w, int h)
{
    int zoomSwitch = MemCache::instance()->value("zoomSwitch").toInt();
    ui->irVideo->zoomSwitch(zoomSwitch == 1);


    if(videoId == IR_VIDEO_ID) {
        ui->irVideo->onShowImg(img.copy((img.width()-w)/2, (img.height()-h)/2, w, h), w, h);
    } else if(videoId == VL_VIDEO_ID) {
        if(!m_hasVlFrame) {
            ui->vlVideo->setVisible(true);
            m_hasVlFrame = true;
        }
        ui->vlVideo->onShowImg(img.copy((img.width()-w)/2, (img.height()-h)/2, w, h), w, h);
    }
}

void VideoWidget::videoProcess()
{
    H264Decode vlH264Decoder;
    vlH264Decoder.initDecoder();
    H264Decode irH264Decoder;
    irH264Decoder.initDecoder();

    while(m_isRunning) {
        if(m_cacheQueue.size() <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        if(m_cacheQueue.size() >= 10) {
            m_cacheQueue.take();
            continue;
        }
        QByteArray frame = m_cacheQueue.take();

        QTime time;
        time.start();

        FrameInfo_S frameInfo;
        memcpy(&frameInfo, frame.data(), sizeof(FrameInfo_S));
//        qDebug() << frameInfo.uFormat << frameInfo.uTotalLen << frameInfo.width << frameInfo.height
//                 << frameInfo.uParamLen << frameInfo.uNvsLen << frameInfo.uY16Len << frameInfo.uYuvLen
//                 << frameInfo.vlFormat << frameInfo.vlLen << frameInfo.vlWidth << frameInfo.vlHeight;

        // 截图bmp功能
        if(m_screenBtn) {
            m_screenBtn = false;
            QString path = QCoreApplication::applicationDirPath() + "/screen/";
            Util::createFileDir(path);
            path += QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_zzz");

            QString irPath = path + ".bmp";
            QByteArray rgbData = VideoFormatParser::instance()->srcToRgb(&irH264Decoder, frameInfo.uFormat,
                                                                         frameInfo.width, frameInfo.height,
                                                                         (unsigned char*)frame.data() +128 + frameInfo.uNvsLen, frameInfo.uYuvLen);
            QImage img((uchar *)rgbData.data(), frameInfo.width, frameInfo.height, QImage::Format_RGB888);
            img.save(irPath, "BMP");
        }

        // 显示视频 如果是8m可见光就不用红外解析了
        if(frameInfo.vlLen < 3840*2160*2){
            irParser(frameInfo, (unsigned char*)frame.data()+128, &irH264Decoder);
        }
        
        // 有可见光数据
        if(frameInfo.vlLen > 0) {
            // qDebug() << "vlLen:" << frameInfo.vlLen ;
            vlParser(frameInfo, (unsigned char*)frame.data()+128, &vlH264Decoder);
        }
        qDebug() << "First use time" << time.elapsed();
    }
}

void VideoWidget::run()
{
    m_isConnect = true;
}

void VideoWidget::stop()
{
    m_isConnect = false;
}

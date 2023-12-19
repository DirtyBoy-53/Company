#include "videowidget.h"
#include "ui_videowidget.h"
#include <QDateTime>
#include <QDebug>
#include <QPainter>

#include "GammaMapping.h"
#include "yuv2rgb.h"
#include "util.h"

using namespace Util;

const int g_arrMaxLen = 640 * 512 * 4;

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    m_frameData = new unsigned char[g_arrMaxLen];
    m_rgbData = new unsigned char[g_arrMaxLen];
    m_y8Data = new unsigned char[g_arrMaxLen];
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::setScaleZoom(double zoomValue)
{
    ui->paintWidget1->setScaleZoom(zoomValue);
}

void VideoWidget::startRecord(QString pathDir)
{
    if(m_frameThread == nullptr) {
        qDebug() << "start record ........" << pathDir;
        m_isRecording = true;
        m_recordPath = pathDir;
        Util::createFileDir(m_recordPath);
        m_frameThread = new std::thread(&VideoWidget::recordThread, this);
    }
}

void VideoWidget::stopRecord()
{
    if(m_frameThread != nullptr) {
        m_isRecording = false;
        qDebug() << "stop record ........";
        if(m_frameThread->joinable()) {
            m_frameThread->join();
            m_frameThread = nullptr;
            m_recordQueue.clear();
        }
    }
}

void VideoWidget::setVideoFormat(VideoFormatInfo info)
{
    m_videoInfo = info;
}

void VideoWidget::addWatermark(int id, const WatermarkInfo& image)
{
    m_watermarkImgMap[id] = image;
}

void VideoWidget::setWatermarkVisible(int id, const bool visible)
{
    m_watermarkImgMap[id].visible = visible;
}

void VideoWidget::removeWatermark(int id)
{
    m_watermarkImgMap.remove(id);
}

void VideoWidget::drawImageData(QByteArray frameBuffer, qint64 timeT)
{
    {
    std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(m_frameData, frameBuffer.mid(m_videoInfo.nvsLen,m_videoInfo.frameLen), m_videoInfo.frameLen);
    }

    if(m_videoInfo.format == VIDEO_FORMAT_NV21) {
        Yuv2Rgb::YuvToRgbNV12(m_frameData, m_rgbData, m_videoInfo.width, m_videoInfo.height);
    } else if(m_videoInfo.format == VIDEO_FORMAT_UYUV) {
        Yuv2Rgb::YuvToRgbUYVY(m_frameData, m_rgbData, m_videoInfo.width, m_videoInfo.height);
    } else if(m_videoInfo.format == VIDEO_FORMAT_YUV420) {
        Yuv2Rgb::YuvToRgb420(m_frameData, m_rgbData, m_videoInfo.width, m_videoInfo.height);
    } else if(m_videoInfo.format == VIDEO_FORMAT_Y16) {
        m_mapNew.DRC_Mix(m_y8Data, (short*)m_frameData, m_videoInfo.width, m_videoInfo.height, 255, 80, 128, 200, 90, 50, 5, 5, 1);
        m_mapNew.y8ToRgb(m_y8Data, m_rgbData, m_videoInfo.width, m_videoInfo.height);
    } else if(m_videoInfo.format == VIDEO_FORMAT_X16) {
        m_map.Data16ToRGB24((short*)m_frameData, m_rgbData, m_videoInfo.width*m_videoInfo.height, 0);
    } else if(m_videoInfo.format == VIDEO_FORMAT_Y16_Y8) {
        Yuv2Rgb::YuvToRgbUYVY(m_frameData, m_rgbData, m_videoInfo.width, m_videoInfo.height);
    }

    QImage image(m_rgbData, m_videoInfo.width,m_videoInfo.height,QImage::Format_RGB888);
    for(int i=0; i<m_watermarkImgMap.size(); i++) {
        if(m_watermarkImgMap[i].visible) {
            QPainter painter;
            painter.begin(&image);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawImage(m_watermarkImgMap[i].x,
                              m_watermarkImgMap[i].y,
                              m_watermarkImgMap[i].img);
            painter.end();
        }
    }
    ui->paintWidget1->setImage(image, timeT);
}

void VideoWidget::clearImage()
{
    ui->paintWidget1->clear();
}

void VideoWidget::updateImage(QByteArray frameData, int timeT)
{
    drawImageData(frameData, timeT);
    if(m_isRecording) {
        m_recordQueue.put(std::make_tuple(frameData.mid(m_videoInfo.nvsLen, m_videoInfo.frameLen + m_videoInfo.paramLen), timeT));
    }
}

void VideoWidget::recordThread()
{
    int recordCount = 0;
    int recordIndex = 0;
    QString timeStr;
    QString filePath;
    while(m_isRecording) {
        if(m_recordQueue.size() <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        FrameData frameData = m_recordQueue.take();
        QByteArray frameBuffer = std::get<0>(frameData);

        if(recordCount == 0) {
            recordIndex++;
            timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss_zzz");
            filePath = QString(m_recordPath + "/" + timeStr + "_%1.nv").arg(QString::number(recordIndex));
        }
//        recordCount++;
        if(recordCount == 1500) recordCount = 0;

        QFile file(filePath);
        if(file.open(QIODevice::Append)) {
            file.write(frameBuffer);
            file.flush();
            file.close();
        }
    }
}

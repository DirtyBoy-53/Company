#include "yuvplay.h"
#include <QFileInfo>
#include <QTime>
#include <QDebug>
#include <QImage>
#include <QCoreApplication>
#include <QtEndian>
#include <QThread>

#include "constdef.h"
#include "yuv2rgb.h"
#include "if/h264decode.h"
#include "util.h"
#include "nvs_result.h"

YuvPlay::YuvPlay()
{
    m_isRunning = true;
    m_thread = new std::thread(&YuvPlay::palyProcess, this);
}

YuvPlay::~YuvPlay()
{

}

int YuvPlay::openFile(QString filePath)
{
    m_filePath = filePath;
    m_isPlayStatus = RECORD_STOP;

    m_playFile.setFileName(filePath);
    if(!m_playFile.open(QIODevice::ReadOnly)) {
        return 0;
    }
    m_vFrameInfo.clear();
    m_frameCount = 0;

    QFileInfo fileInfo(filePath);
    qint64 readSize = fileInfo.size();
    m_fileSize = readSize;
    QByteArray collectHead = m_playFile.read(HEADSIZE);
    // memcpy(&collectInfo, collectHead, sizeof(CollectInfo));

    while(readSize - 128) {
        qDebug() << "readSize is " << readSize;
        QByteArray head = m_playFile.read(HEADSIZE);
        if(head.size() <= 0) break;
        FrameInfo_S frameInfo;
        memcpy(&frameInfo, head, sizeof(FrameInfo_S));
        qDebug() << frameInfo.uTotalLen << frameInfo.width << frameInfo.height
                 << frameInfo.uParamLen << frameInfo.uNvsLen << frameInfo.uFormat;

        QByteArray frameData = m_playFile.read(frameInfo.uTotalLen - HEADSIZE);
        if(frameData.size() <= 0) break;
        readSize -= frameInfo.uTotalLen;
        m_vFrameInfo.append(frameInfo);
    }

    m_playFile.seek(HEADSIZE);
    return m_vFrameInfo.size();
}

void YuvPlay::closeFile()
{

}

void YuvPlay::play()
{
    if(!m_playFile.isOpen()) {
        m_playFile.open(QIODevice::ReadOnly);
        m_playFile.seek(HEADSIZE);
    }

    m_isPlayStatus = RECORD_PALYING;
}

void YuvPlay::suspend()
{
    m_isPlayStatus = RECORD_SUPPEND;
}

void YuvPlay::stop()
{
    m_frameCount = 0;
    m_isPlayStatus = RECORD_STOP;
    if(m_playFile.isOpen()) {
        m_playFile.close();
    }
}

void YuvPlay::next()
{
    if(m_isPlayStatus != RECORD_SUPPEND) {
        return;
    }

    m_frameCount++;
    moveTo(m_frameCount);
}

void YuvPlay::last()
{
    if(m_isPlayStatus != RECORD_SUPPEND) {
        return;
    }
    m_frameCount--;
    moveTo(m_frameCount);
}

void YuvPlay::moveTo(const int index)
{
    m_frameCount = index;
    qint64 movePos = 128;
    for(int i=0; i<index; i++) {
        movePos += m_vFrameInfo[i].uTotalLen;
        qDebug() << "len " << movePos;
    }
    m_playFile.seek(movePos);

    QByteArray head = m_playFile.read(HEADSIZE);
    FrameInfo_S frameInfo;
    QByteArray frameData;
    memcpy(&frameInfo, head, sizeof(FrameInfo_S));
    qDebug() << frameInfo.uTotalLen << frameInfo.width << frameInfo.height
             << frameInfo.uParamLen << frameInfo.uNvsLen << frameInfo.uFormat;
    QByteArray tmpData = m_playFile.read(frameInfo.uTotalLen - HEADSIZE);
    if (frameInfo.endian() == 1) {
        for (int i = 0; i < tmpData.size() / 2; i++) {
            frameData[2 * i] = tmpData[2 * i + 1];
            frameData[2 * i + 1] = tmpData[2 * i];
        }
    }
    else {
        frameData = tmpData;
    }
    emit signalUpdatePlayData(head + frameData, m_frameCount, m_isPlayStatus);
}

void YuvPlay::setNvsSwitch(const bool flag)
{

}

bool YuvPlay::getFrame(QString path, const int mode, const int interval, const GetFrameConfig config)
{
    if(interval == 0) return false;

    QByteArray rgbData;
    QByteArray vlRgbData;

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "open file failed! " << path;
        return false;
    }
    file.read(HEADSIZE);

    H264Decode vlH264Decoder;
    vlH264Decoder.initDecoder();
    H264Decode irH264Deocder;
    irH264Deocder.initDecoder();

    QFileInfo fileInfo(path);
    qint64 readSize = fileInfo.size();
    QString dstPath = fileInfo.absolutePath() + "/frame/";
    if(mode == GET_FRAME_BMP) {
        dstPath = fileInfo.absolutePath() + "/frame/bmp/";
    } else if(mode == GET_FRAME_YUV) {
        dstPath = fileInfo.absolutePath() + "/frame/raw/";
    }
    Util::createFileDir(dstPath);

    int frameIndex = 0;
    int iIdx = 0;
    QByteArray frameData;
    while(readSize - HEADSIZE) {
        QByteArray head = file.read(HEADSIZE);
        if(head.size() <= 0) break;
        FrameInfo_S frameInfo;
        memcpy(&frameInfo, head, sizeof(FrameInfo_S));
        qDebug() << frameInfo.uFormat << frameInfo.uTotalLen << frameInfo.width << frameInfo.height
                 << frameInfo.uParamLen << frameInfo.uNvsLen << frameInfo.uY16Len << frameInfo.uYuvLen;

        QByteArray tmpData = file.read(frameInfo.uTotalLen - HEADSIZE);
        if(tmpData.size() <= 0) break;
        readSize -= frameInfo.uTotalLen;
        frameIndex++;
        if(frameIndex % interval != 0) {
            if(frameInfo.vlLen > 0) {
                int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
                vlRgbData = VideoFormatParser::instance()->vlSrcToRgb(&vlH264Decoder, frameInfo.vlFormat, frameInfo.vlWidth, frameInfo.vlHeight,
                                          (uint8_t *)tmpData.data() + start, frameInfo.vlLen);
            }

            if(frameInfo.uFormat == VIDEO_FORMAT_H264) {
                rgbData = VideoFormatParser::instance()->srcToRgb(&irH264Deocder, frameInfo.uFormat, frameInfo.width, frameInfo.height,
                                           (uint8_t *)tmpData.data() + frameInfo.uNvsLen, frameInfo.uYuvLen);
            }
            continue;
        }
        iIdx++;


        if(frameInfo.endian() == 1) {
            for(int i=0; i<tmpData.size()/2; i++) {
                frameData[2*i] = tmpData[2*i+1];
                frameData[2*i+1] = tmpData[2*i];
            }
        } else {
            frameData = tmpData;
        }

        if(mode == GET_FRAME_BMP) {
            if (frameInfo.vlLen < 3840 * 2160 * 2) {
                rgbData = VideoFormatParser::instance()->srcToRgb(&irH264Deocder, frameInfo.uFormat, frameInfo.width, frameInfo.height,
                    (uint8_t*)frameData.data() + frameInfo.uNvsLen, frameInfo.uYuvLen);

                QImage img((uchar*)rgbData.data(), frameInfo.width, frameInfo.height, QImage::Format_RGB888);
                Util::createFileDir(dstPath + "/ir");
                QString irPath = dstPath + QString("/ir/%1_%2-ir.bmp").arg(fileInfo.baseName()).arg(iIdx);
                img.save(irPath, "BMP");
            }
            
            if(frameInfo.vlLen > 0) {
                int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
                vlRgbData = VideoFormatParser::instance()->vlSrcToRgb(&vlH264Decoder, frameInfo.vlFormat, frameInfo.vlWidth, frameInfo.vlHeight,
                                        (uint8_t *)frameData.data() + start, frameInfo.vlLen);

                Util::createFileDir(dstPath + "/vl");
                QString vlPath = dstPath + QString("/vl/%1_%2-vl.bmp").arg(fileInfo.baseName()).arg(iIdx);
                QImage vlImg((uchar *)vlRgbData.data(), frameInfo.vlWidth, frameInfo.vlHeight, QImage::Format_RGB888);
                vlImg.save(vlPath, "BMP");
            }
        }  else if(mode == GET_FRAME_YUV) {
            QString irPath;
            int writeLen;
            if(frameInfo.uFormat == VIDEO_FORMAT_Y16) {
                irPath = dstPath + QString("/ir/%1_%2-ir.y16").arg(fileInfo.baseName()).arg(iIdx);
                writeLen = frameInfo.uY16Len;
            } else {
                irPath = dstPath + QString("/ir/%1_%2-ir.yuv").arg(fileInfo.baseName()).arg(iIdx);
                writeLen = frameInfo.uYuvLen;
            }
            Util::createFileDir(dstPath + "/ir");
            QFile fileIr(irPath);
            fileIr.open(QIODevice::Append);
            if(frameInfo.uFormat == VIDEO_FORMAT_H264) {
                uint8_t* pData = (uint8_t *)frameData.data() + frameInfo.uNvsLen;
                unsigned char* vlYuv = new unsigned char[frameInfo.width*frameInfo.height*3/2];
                unsigned char* vlU = vlYuv+frameInfo.width*frameInfo.height;
                unsigned char* vlV = vlYuv+frameInfo.width*frameInfo.height + frameInfo.width*frameInfo.height/4;;
                irH264Deocder.h264ToYuv(pData, vlYuv, vlU, vlV, frameInfo.uYuvLen, frameInfo.width, frameInfo.height);
                fileIr.write((const char*)vlYuv, frameInfo.width*frameInfo.height*3/2);
                delete vlYuv;
            } else {
                fileIr.write(frameData.mid(frameInfo.uNvsLen, writeLen));
            }
            if(config.isGetParam) {
                uint8_t* paramData = (uint8_t *)frameData.data() + frameInfo.uNvsLen + writeLen;
                fileIr.write((const char*)paramData, frameInfo.uParamLen);
            }
            fileIr.close();

            if(frameInfo.vlLen > 0) {
                int start = frameInfo.uNvsLen + frameInfo.uY16Len + frameInfo.uYuvLen + frameInfo.uParamLen;
                vlRgbData = VideoFormatParser::instance()->vlSrcToYuv(&vlH264Decoder, frameInfo.vlFormat, frameInfo.vlWidth, frameInfo.vlHeight,
                                          (uint8_t *)frameData.data() + start, frameInfo.vlLen);
                QString vlPath = dstPath + QString("/vl/%1_%2-vl.yuv").arg(fileInfo.baseName()).arg(iIdx);
                Util::createFileDir(dstPath + "/vl");
                QFile fileVl(vlPath);
                fileVl.open(QFile::QIODevice::Append);
                fileVl.write(vlRgbData);
                fileVl.close();
            }
        }

        if(config.isGetNvsTxt) {
            BoxMsg_t box = *(BoxMsg_t*)frameData.data();
            QFile nvsFile(dstPath + QString("/ir/%1_%2-ir.txt").arg(fileInfo.baseName()).arg(iIdx));
            nvsFile.open(QIODevice::Append);
            if(box.obj_number <= 29) {
                QString txt;
                for(int i=0; i<box.obj_number; i++) {
                    txt = QString("%1 %2 %3 %4 %5\n")
                          .arg(box.bbox[i].class_id)
                          .arg((box.bbox[i].coordinates.x2 + box.bbox[i].coordinates.x1)/2/frameInfo.width)
                          .arg((box.bbox[i].coordinates.y2 + box.bbox[i].coordinates.y1)/2/frameInfo.height)
                          .arg((box.bbox[i].coordinates.x2 - box.bbox[i].coordinates.x1)/frameInfo.width)
                          .arg((box.bbox[i].coordinates.y2 - box.bbox[i].coordinates.y1)/frameInfo.height);
                    nvsFile.write(txt.toLatin1());
                }
            }
            nvsFile.close();
        }

    }
    vlH264Decoder.unInitDecoder();
    irH264Deocder.unInitDecoder();
}

void YuvPlay::getVideoData(QString path, bool hasParam)
{
    QFile readfile(path);
    if(!readfile.exists()) return;

    QFileInfo fileInfo(path);
    qint64 readSize = fileInfo.size();
    readfile.open(QIODevice::ReadOnly);
    readfile.read(HEADSIZE); // 去掉采集头

    QString outFileName = fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".raw";
    QFile outFile(outFileName);
    outFile.open(QIODevice::Append);


    while(readSize - HEADSIZE) {
        QByteArray head = readfile.read(HEADSIZE);
        if(head.size() <= 0) break;
        FrameInfo_S frameInfo;
        memcpy(&frameInfo, head, sizeof(FrameInfo_S));

        QByteArray frameData = readfile.read(frameInfo.uTotalLen - HEADSIZE);
        if(frameData.size() <= 0) break;
        readSize -= frameInfo.uTotalLen;

        if(hasParam) {
            outFile.write(frameData.mid(frameInfo.uNvsLen, frameInfo.uYuvLen + frameInfo.uY16Len + frameInfo.uParamLen));
        } else {
            outFile.write(frameData.mid(frameInfo.uNvsLen, frameInfo.uYuvLen + frameInfo.uY16Len));
        }

//        if(frameInfo.uFormat == VIDEO_FORMAT_Y16) {
//            if(hasParam) {
//                outFile.write(frameData.mid(frameInfo.uNvsLen, frameInfo.uY16Len + frameInfo.uParamLen));
//            } else {

//            }
//        } else {
//            outFile.write(frameData.mid(frameInfo.uNvsLen, frameInfo.uYuvLen));
//        }

        qDebug() << readSize << frameInfo.uTotalLen << frameInfo.width << frameInfo.height << frameInfo.uFormat
                 << frameInfo.uYuvLen << frameInfo.uY16Len << frameInfo.uNvsLen;
    }

    readfile.close();
    outFile.close();
    qDebug() << "write ok";
}

void YuvPlay::palyProcess()
{
    QByteArray head;
    QByteArray frameData;
    qDebug("palyProcess pid:%d",gettid());
    while(m_isRunning) {
        if(m_isPlayStatus == RECORD_STOP || m_isPlayStatus == RECORD_SUPPEND) {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
            continue;
        }

        if(m_isPlayStatus == RECORD_PALYING) {
            head = m_playFile.read(HEADSIZE);
            if(head.size() == HEADSIZE) {
                FrameInfo_S frameInfo;
                memcpy(&frameInfo, head, sizeof(FrameInfo_S));
                m_lastFrameInfo = frameInfo;
            //    qDebug() << frameInfo.uTotalLen << frameInfo.width << frameInfo.height
            //             << frameInfo.uParamLen << frameInfo.uNvsLen
            //             << frameInfo.imuLen << sizeof(BoxMsg_t);

                if(frameInfo.endian() == 1) {
                    QByteArray tmpData = m_playFile.read(frameInfo.uTotalLen - HEADSIZE);
                    for(int i=0; i<tmpData.size()/2; i++) {
                        frameData[2*i] = tmpData[2*i+1];
                        frameData[2*i+1] = tmpData[2*i];
                    }
                } else {
                    frameData = m_playFile.read(frameInfo.uTotalLen - HEADSIZE);
                }

                m_frameCount++;
                emit signalUpdatePlayData(head + frameData, m_frameCount, m_isPlayStatus);
            } else {
                stop();
                emit signalUpdatePlayData(QByteArray(0), 0, RECORD_STOP);
            }

        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(60));
        QThread::msleep(40);
    }
}

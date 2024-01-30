#include "recorder.h"
#include <thread>
#include <QThread>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QDebug>
// #include <mainwindow.h>
#include "memcache.h"
#include "opencv2/opencv.hpp"
#include "configinfo.h"
#include "Encode2H264.h"
#include <QMessageBox>

Recorder::Recorder()
{
    m_iStart = 0;
    m_thread = nullptr;
}

void Recorder::start(int channel)
{
    if(m_iStart == 1) {
        return;
    }
    m_iStart = 1;
    m_channel = channel;

    m_thread = new std::thread(&Recorder::process,this);
    //m_thread->detach();
}

void Recorder::stop()
{
    m_iStart = 0;
    if(m_thread) {
        m_thread->join();
        delete m_thread;
    }
}
#include "utils/global.hpp"
void Recorder::process()
{
    SharedMemBuffer mem;
    mem.init((ESHARECHANNEL)(int)m_channel, 0);
    QString sData = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString sRecordPath = MemCache::instance()->value("recordPath");
    ConfigInfo::getInstance()->GetSavePath(sRecordPath);
    if(sRecordPath.size() <= 0) {
        sRecordPath = "/media/xy/raid0";
    }
    // QString sPath = sRecordPath + "/" + sData +"/channel" + QString::number((int)m_channel);
    CSenceInfo _senceInfo;
    ConfigInfo::getInstance()->ReadAllConfig();
    ConfigInfo::getInstance()->GetSenceInfo(_senceInfo);
    QString sPath = QString("%1/%2_channel%3_%4_%5_%6_%7℃_%8%rh/").arg(sRecordPath).arg(_senceInfo._Project[(int)m_channel])
                                                                .arg(QString::number((int)m_channel+1)).arg(_senceInfo._Time)
                                                                .arg(_senceInfo._Weather).arg(_senceInfo._TimeQuantum).arg(_senceInfo._Temp).arg(_senceInfo._Wet);

    // QString sFilePre =
    //         sPath + "/" +
    //         QDateTime::currentDateTime().toString("yyyyMMddhhmmss_zzz") +
    //         "_channel" + QString::number((int)m_channel);
    
    QString sFilePre = QString("%1%2/video/").arg(sPath).arg(_senceInfo._Scene);

    int iFileIdx = 0;
    int iWriteCount = 0;
    QDir dir;
    dir.mkpath(sFilePre);
    char * cache = (char *) malloc(20 * 1024 * 1024);
    qInfo()<<"start to rec channel"<<(int)m_channel;
    FrameInfo_S frameHead;
    QString _suffix{".nv"};
    // QString _saveFile = sFilePre + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") +"_" + QString::number(iFileIdx);
    QString _saveFile = sFilePre + YDateTime::currentDateTime()+"_" + QString::number(iFileIdx);
    QString sFile{""};
    CEncode2H264 encoder;
    bool bFirstRecord = true;
    QString lvType = MemCache::instance()->value("light_videoType");
    QFile file;
    while(m_iStart == 1) {
        QCoreApplication::processEvents();
        //qDebug() << "4. Read Data...";
        int rLen = mem.readFrame(cache, 20 * 1024 * 1024);
        memset(&frameHead,0,sizeof(FrameInfo_S));
        memcpy(&frameHead,cache,sizeof(FrameInfo_S));

        if(rLen <= 0) {
            //QThread::usleep(10 * 1000);
            //qDebug() << "4. no Data...";
            continue;
        }
        //qDebug() << "4. Has Data...";

        if(iWriteCount >= 1500) {
            iFileIdx ++;
            iWriteCount = 0;
            // _saveFile = sFilePre + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") +"_" + QString::number(iFileIdx);
            _saveFile = sFilePre + YDateTime::currentDateTime() +"_" + QString::number(iFileIdx);
        }

        if(frameHead.uFormat == 1){//Y8
           _suffix = ".y8";
        }else if(frameHead.uFormat == 3){//Y16
           _suffix = ".y16";
        }else{
            _suffix = ".nv";
        }

        if (frameHead.ext[2] == 31 && lvType.contains("h264")) {
            if (iWriteCount % 1500 == 0) {
                _suffix = ".h264";
                sFile = _saveFile + _suffix;
                
                if (bFirstRecord) {
                    bFirstRecord = true;
                } else {
                    encoder.endCodec();
                }
                if (!encoder.initCodec(frameHead.width, frameHead.height, 25, (char*)sFile.toStdString().c_str())) {
                    QMessageBox::information(nullptr, "error", "初始化Codec失败[保存h264]");
                }
            }
            if (!encoder.addOneYuYv422((unsigned char*)cache + 128, rLen - 128, frameHead.uTimeTm)) {
                QMessageBox::information(nullptr, "error", "保存数据出错[保存h264]");
            }
        } else {
            sFile = _saveFile + _suffix;
            {
                if (iWriteCount % 1500 == 0) {
                    if(file.isOpen())
                        file.close();
                    iWriteCount = 0;
                }    
                if(!file.isOpen()){
                    file.setFileName(sFile);
                    file.open(QFile::Append | QFile::ReadWrite);
                    char szHeader[128]{0};
                    #if 1
                    file.write(szHeader, 128);
                    #endif
                }
                
                if(file.isOpen()) { // 将头部信息一起保存
                        file.write(cache, rLen);
                }else {
                    qInfo()<<"open file error "<< sFile;
                }
            }
        }
        iWriteCount ++;
    }

    if (frameHead.ext[2] == 31 && lvType.contains("h264")) {
        encoder.endCodec();
    }
    if(file.isOpen()){
        file.close();
    }
    
    if (cache != nullptr) {
        free(cache);
        cache = nullptr;
    }
    //qInfo()<<"end to rec channel"<<(int)m_channel;

}



#ifndef GLYUVWIDGET_H
#define GLYUVWIDGET_H


#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QThread>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include "recorder.h"
#include "memcache.h"
#include "MapNew.h"
#include <QLabel>
#include <QTimer>
#include "h264decode.h"
#include "common.h"
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

enum ScaleType{
    fitXY,
    fitCenter,
    CenterCorp
};
class GLAssistant : public QObject
{
    Q_OBJECT
public:
    GLAssistant(QObject* parent = nullptr, int slotId = 0) : QObject(parent),
        m_videoId(slotId),
        m_showCount(0)
    {    
        m_astThread = new QThread;
        moveToThread(m_astThread);
        m_astThread->start();
        
        m_rgb = (unsigned char *)malloc(1024*1024*20);
        m_y16 = (unsigned char *)malloc(1024*1024*20);
        m_shotSwitch = false;

        vlH264Decoder.initDecoder();
    }

    ~GLAssistant()
    {
        if (m_astThread) {
            delete m_astThread;
            m_astThread = nullptr;
        }
    }
public:
    EnumFormat m_videoFormat[MAX_ITEM_NUM];
private:
    int m_videoId;
    int m_showCount;
    int m_showType;
    H264Decode vlH264Decoder;
signals:
    void signalSetYuv(int slot, 
                      uchar *Yptr,
                      uchar *Uptr,
                      uchar *Vptr,
                      uint width,
                      uint height,
                      int type = (int)fitCenter);
public slots:
    void slotShotOne(bool shotOne = true)
    {
        m_shotSwitch = shotOne;
    }
    void  slotDataSetting(int slot, unsigned char* data, int len)
    {
        if (slot != m_videoId) {
            return;
        }
        // qDebug() << "GLAssistant::slotDataSetting " << QThread::currentThreadId();
        // if (m_showCount++ != 3) {
        //     return;
        // } else {
        //     m_showCount = 0;
        // }

        unsigned char* pData = new uchar[len + 1];
        memcpy (pData, data, len);
        // delete data;
        FrameInfo_S * pHead = (FrameInfo_S *) pData;
        ChannelInfo info = MemCache::instance()->getChannel(slot);
        m_showType = info.typeId;
        // qDebug() << "m_showType:" << m_showType;
        // m_showType = TYPEI2CDIRECT;
        m_videoFormat[slot] = EnumFormat::Enum_Yuv444;
#pragma region IIC相机
        if (m_showType == TYPEI2CDIRECT) {
            
            if(len != HEADSIZE + pHead->width * pHead->height * 2 + pHead->uParamLen) {
                qInfo()<<">>> channel " << m_videoId << " format error";
                return;
            }
            uchar * pY = m_rgb;
            uchar * pU = m_rgb + pHead->width * pHead->height;
            uchar * pV = m_rgb + pHead->width * pHead->height+ pHead->width * pHead->height / 2;

            int iY = 0;
            int iUv = 0;

            uchar * p = pData + HEADSIZE;

            int dataType = 0; // 1-yuv 3-y16
            //qDebug() << "Current Packet Type is " << pHead->ext[2];
            //qDebug() << "Current Packet Format is " << pHead->uFormat;
            // if (pHead->ext[2] == 37 || pHead->ext[2] == 38) { // 红外都会带参数行
            //     //big endian to little endian
            //     dataType = pHead->uFormat;
            //     if (dataType == 3) {
            //         unsigned short * pShort = (unsigned short*)p;
            //         for (int foo = pHead->uNvsLen; foo < (len - HEADSIZE - pHead->uParamLen) / 2; ++foo) {
            //             swap_endian(pShort[foo]);
            //         }
            //     }
            //     // int ipos = pHead->width * (pHead->height + 2) * 2; // height contains the param line (4 lines)
            //     // if (pHead->uParamLen > 0) {
            //     //     if ((unsigned char)p[ipos] == 0xAA && (unsigned char)p[ipos + 1] == 0x55) {
            //     //         dataType = (unsigned char)(p[ipos + 16]);
            //     //         qDebug() << "Current Data Type is " << dataType << " 1-Y8, 3-Y16";
            //     //     }
            //     // }

            // } else { // 1280
            //     dataType = 1;
            // }

            if (pHead->uFormat == 1) {
                // qDebug() << "Y8 data...";
                for(int j = 0;j < pHead->width * pHead->height * 2; j += 4) {
                    // if (pHead->uFormat == 37) { // IR
                    //     pY[iY] = p[j + 1];
                    //     pY[iY + 1] = p[j + 3];
                    //     pU[iUv] = p[j] ;
                    //     pV[iUv] = p[j + 2] ; 
                    // } else 
                    { // light UYVY => YUYV
                        pY[iY] = p[j];
                        pY[iY + 1] = p[j + 2];
                        pU[iUv] = p[j + 1] ;
                        pV[iUv] = p[j + 3] ; 
                    }

                    iUv ++;
                    iY += 2;
                }
            } else if (pHead->uFormat == 3) {
                //big endian to little endian
                unsigned short * pShort = (unsigned short*)p;
                for (int foo = pHead->uNvsLen; foo < (len - HEADSIZE - pHead->uParamLen) / 2; ++foo) {
                    swap_endian(pShort[foo]);
                }
                // qDebug() << "Y16 data...";
                Data16ToRGB24((short*)p, m_y16, pHead->width* pHead->height, 0);
                rgb2yuv422Planar(m_y16, pHead->height, pHead->width, pHead->width * 3, m_rgb);
            }
            
            // m_yPtr = pY;
            // m_uPtr = pU;
            // m_vPtr = pV;

            // videoW = pHead->width;
            // videoH = pHead->height;
            // m_type = fitCenter;
            signalSetYuv(slot, pY, pU, pV, pHead->width, pHead->height, (int)fitCenter);
            

            if (m_shotSwitch) {
                QString path = QCoreApplication::applicationDirPath() + "/shot/video" + QString::number(m_videoId);
                QDir dir(path);
                if (!dir.exists(path)) {
                    dir.mkpath(path);
                }
                qDebug() << "save shot to path " << path;
                QFile file(path + "/shot.raw");
                file.open(QIODevice::ReadWrite);
                file.write((char*)p, pHead->width * pHead->height * 2);
                file.close();

                // QImage image(m_y16[i], pHead->width, pHead->height, QImage::Format_RGB888);
                // image.save(path + "/shot.bmp");
                m_shotSwitch = false;
            }
        } 
#pragma endregion
#pragma region 网络相机
        else if (m_showType == TYPEARTOSYN) {
            // qDebug() << pHead->width << " " << pHead->height << " " << pHead->uTotalLen << " " << pHead->uFormat;
#pragma region 显示红外视频
            if(MemCache::instance()->value("Video_Dis_Type") == "IR"){
                uchar * pY = m_rgb;
                uchar * pU = m_rgb + pHead->width * pHead->height;
                uchar * pV = m_rgb + pHead->width * pHead->height+ pHead->width * pHead->height / 2;
                uchar * p = pData + HEADSIZE + pHead->uNvsLen;

                // qDebug() << "Artosyn One Frame Size : " << rLen;
                if(len != HEADSIZE + pHead->uNvsLen + pHead->uParamLen + pHead->width * (pHead->height ) * 2 + pHead->vlLen) {
                    qInfo()<<"TYPEARTOSYN  IR>>> channel " << m_videoId << " format error";
                    return;
                }

                if (pHead->uFormat == 1) {// //1 yuv422 3 yuv16 5 y16+yuv 6 yuv420  7 x16     
                    int iY = 0;
                    int iUv = 0;

                    int dataType = 0;
                    // qDebug() << "Y8 data...";
                    for(int j = 0;j < pHead->width * pHead->height * 2; j += 4) {
                        pY[iY] = p[j + 1];
                        pY[iY + 1] = p[j + 3];                            
                        pU[iUv] = p[j] ;
                        pV[iUv] = p[j + 2] ;
                        iUv ++;
                        iY += 2;
                    }
                } else if (pHead->uFormat == 3) {
                    Data16ToRGB24((short*)p, m_y16, pHead->width* pHead->height, 0);
                    rgb2yuv422Planar(m_y16, pHead->height, pHead->width, pHead->width * 3, m_rgb);
                    if (m_shotSwitch) {
                        QString path = QCoreApplication::applicationDirPath() + "/video" + QString::number(m_videoId);
                        QDir dir(path);
                        if (!dir.exists(path)) {
                            dir.mkpath(path);
                        }
                        qDebug() << "save shot to path " << path;
                        QFile file(path + "/shot.y16");
                        file.open(QIODevice::ReadWrite);
                        file.write((char*)pData + HEADSIZE, pHead->width * pHead->height * 2);
                        file.close();

                        QImage image(m_y16, pHead->width, pHead->height, QImage::Format_RGB888);
                        image.save(path + "/shot.bmp");
                        m_shotSwitch = false;
                    }
                }

                if (m_shotSwitch) {
                    QString path = QCoreApplication::applicationDirPath() + "/shot/video" + QString::number(m_videoId);
                    QDir dir(path);
                    if (!dir.exists(path)) {
                        dir.mkpath(path);
                    }
                    qDebug() << "save shot to path " << path;
                    QFile file(path + "/shot1.raw");
                    file.open(QIODevice::ReadWrite);
                    file.write((char*)p + HEADSIZE, pHead->width * pHead->height * 2);
                    file.close();

                    // QImage image(m_y16[i], pHead->width, pHead->height, QImage::Format_RGB888);
                    // image.save(path + "/shot.bmp");
                    m_shotSwitch = false;
                }
                //qDebug() << "调用方 =》" << QThread::currentThreadId();
                
                // m_yPtr = pY;
                // m_uPtr = pU;
                // m_vPtr = pV;

                // videoW = pHead->width;
                // videoH = pHead->height;
                // m_type = fitCenter;

                emit signalSetYuv(slot, pY, pU, pV, pHead->width, pHead->height, (int)fitCenter);
            }
#pragma endregion
#pragma region 显示可见光视频
            else if(MemCache::instance()->value("Video_Dis_Type") == "VL"){
                m_videoFormat[slot] = EnumFormat::Enum_Yuv420;
                // qDebug() << "Artosyn One Frame Size : " << rLen;
                if(len != HEADSIZE + pHead->uNvsLen + pHead->uParamLen + pHead->width * (pHead->height ) * 2 + pHead->vlLen) {
                    qInfo()<<"TYPEARTOSYN  VL>>> channel " << m_videoId << " format error";
                    return;
                }
                uchar * pY = m_rgb;
                uchar * pU = m_rgb + pHead->vlWidth * pHead->vlHeight;
                uchar * pV = m_rgb + pHead->vlWidth * pHead->vlHeight+ pHead->vlWidth * pHead->vlHeight / 2;


                // uchar * p = pData + HEADSIZE+pHead->uNvsLen + pHead->uParamLen + pHead->width * (pHead->height ) * 2;
                uchar * p = pData + HEADSIZE + pHead->uNvsLen + pHead->uY16Len + pHead->uYuvLen + pHead->uParamLen;
                uint32_t vlWH = pHead->vlWidth * pHead->vlHeight;
                if (pHead->vlFormat == 1) {
                
                } else if (pHead->vlFormat == 2) {
                    memcpy(pY,p,sizeof(uchar)*vlWH);
                    memcpy(pU,p+vlWH,sizeof(uchar)*vlWH/4);
                    memcpy(pV,p+vlWH*5/4,sizeof(uchar)*vlWH/4);
                }else if(pHead->vlFormat == 3){
                    vlH264Decoder.h264ToYuv(p,pY,pU,pV,pHead->vlLen,pHead->vlWidth,pHead->vlHeight);
                }
            
                signalSetYuv(slot, pY, pU, pV, pHead->vlWidth, pHead->vlHeight, (int)fitCenter);
            

                if (m_shotSwitch) {
                    QString path = QCoreApplication::applicationDirPath() + "/shot/video" + QString::number(m_videoId);
                    QDir dir(path);
                    if (!dir.exists(path)) {
                        dir.mkpath(path);
                    }
                    qDebug() << "save shot to path " << path;
                    QFile file(path + "/shot.raw");
                    file.open(QIODevice::ReadWrite);
                    file.write((char*)p, pHead->width * pHead->height * 2);
                    file.close();
                    m_shotSwitch = false;
                }
            }
#pragma endregion
        }
#pragma endregion
        if (pData) {
            delete []pData;
            pData = nullptr;
        }
    }

private:
    QThread* m_astThread;

    unsigned char* m_rgb = nullptr;
    unsigned char* m_y16 = nullptr;
    bool m_shotSwitch = false;
};

/// @brief 
class GLYuvWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLYuvWidget(QWidget *parent =0, int videoId = 0, bool needAssitant = false);
    ~GLYuvWidget();
public slots:
    void onShowYUV(uchar *Yptr,uchar *Uptr,uchar *Vptr,uint width,uint height, ScaleType type = fitCenter);
    void videoControl(QWidget* type);
    void slotSetYuv(int slot, uchar *Yptr,uchar *Uptr,uchar *Vptr,uint width,uint height, int type = (int)fitCenter);
    void slotSetData(int slot, unsigned char* data, int len);
    void slotSetShowType(int slot, int type);
    void slot_timeout();
signals:
    void shotOne(bool shot = true);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void set_bt_Icon(QPushButton *bt, QString path);
    void paintControl();

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    

    //opengl中y、u、v分量位置
    GLuint posUniformY,posUniformU,posUniformV;

    //纹理
    QOpenGLTexture *textureY = nullptr,*textureU = nullptr,*textureV = nullptr;

    //纹理ID，创建错误返回0
    GLuint m_idY,m_idU,m_idV;

    //像素分辨率
    uint videoW,videoH;

    uchar *m_yuvPtr = nullptr;
    //Y U V 分量
    uchar *m_yPtr = nullptr;
    uchar *m_uPtr = nullptr;
    uchar *m_vPtr = nullptr;
    ScaleType m_type;

    bool m_fullScreen;
    Qt::WindowFlags m_lastWindowFlags;
    QRect m_lastGeometry;

    bool m_isInitialized = false;
    QMutex m_showMutex;
    QPushButton* m_btnPlay;
    QPushButton* m_btnRecord;
    QPushButton* m_btnShot;
    QLabel* m_label_record_time;
    QLabel* m_label_connect_status;
    QImage m_img;

    bool m_hasPlayed = false;
    bool m_hasRecorded = false;
    bool m_hasReplayed = false;

    int m_videoId = -1;
    Recorder m_videoRecord;

    GLAssistant * m_assitant;
    QTimer m_timer_record;
    QTime m_time_record;

};


#endif // GLYUVWIDGET_H

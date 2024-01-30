#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QLabel>
#include "videoinfo.h"
#include "StaticSignals.h"
#include "constdef.h"
#include "nvshandler.h"
#include "miximage.h"
#include "if/h264decode.h"
#include "blockingqueue.h"
#include "cremap.h"
#include "imuhandler.h"

struct LayoutPressPoint {
    int layoutW;
    int layoutH;
    int videoW;
    int videoH;
    int clickX;
    int clickY;
};

namespace Ui {
class VideoWidget;
}

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    void getScreen();
    void updateFrame(QByteArray data);
    void run();
    void stop();

public slots:
    void addWatermark(const int videoId, int waterMarkId, const WatermarkInfo& image);
    void setWatermarkVisible(const int videoId, int waterMarkId, const bool visible);
    void moveWatermark(const int videoId, int waterMarkId, const int x, const int y);
    void removeWatermark(const int videoId, int waterMarkId);

    void onMousePressEvent(const int index, QPoint point);
    void onKeyPressEvent(const int index, const int key);
    void onMouseDoublePress(const int index);
    void onWheelEvent(const int index, const int direction);

    void slotUpdateX16B(QByteArray bData);
    void slotUpdateImg(int videoId, QImage img, int w, int h);
private:
    void vlParser(FrameInfo_S frameInfo, unsigned char* frame, H264Decode* h264Decoder);
    void irParser(FrameInfo_S frameInfo, unsigned char* frame, H264Decode* h264Decoder);
    void videoProcess();

signals:
    void signalPressPos(const int videoId, const int x, const int y);
    void signalUpdateImg(int videoId, QImage img, int w, int h);
    void signalBadpointImg(QImage img);

private:
    Ui::VideoWidget *ui;
    NvsHandler m_nvsHandler;

    QMap<int, LayoutPressPoint> m_mapPoint;
    QMap<int, WatermarkInfo> m_irWaterMark;
    QMap<int, WatermarkInfo> m_vlWaterMark;

    uint8_t* m_irYuv = nullptr;
    uint8_t* m_irRgb = nullptr;
    uint8_t* m_vlRgb = nullptr;
    uint8_t* m_vlYuv = nullptr;

    bool m_screenBtn = false;

    QPixmap* m_vlImg;
    QPixmap* m_irImg;

    int m_vlScale = 1;
    int m_irScale = 1;

    MixImage m_mix;
    CRemap m_remap;
    bool m_isRemap = false;

    QPoint m_badpointClicked;

    H264Decode m_vlH264Decoder;
    int m_x16ShutterCount = 0;
    QByteArray m_bData;
    bool m_isUpdateB = false;

    std::thread* m_videoThread = nullptr;
    bool m_isRunning = false;
    BlockingQueue<QByteArray> m_cacheQueue;
    bool m_hasVlFrame = false;

    int m_delayShowCount = 0;
    bool m_isConnect = false;

    ImuHandler m_imu;
};

#endif // VIDEOWIDGET_H

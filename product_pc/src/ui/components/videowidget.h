#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include "blockingqueue.h"
#include "videopaintwidget.h"
#include "nvs_result.h"
#include "videoinfo.h"
#include "mapping.h"
#include "CInfraredCore.h"
#include "mappingnew.h"

namespace Ui {
class VideoWidget;
}

struct WatermarkInfo
{
    bool visible = false;
    QImage img;
    int x = 0;
    int y = 0;
};


typedef std::tuple<QByteArray, qint64> FrameData;


class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
    void setScaleZoom(double zoomValue);
    void startRecord(QString pathDir);
    void stopRecord();

    void setVideoFormat(VideoFormatInfo info);
    void updateImage(QByteArray frameData, int timeT);
    void clearImage();

    void addWatermark(int waterMarkId, const WatermarkInfo& image);
    void setWatermarkVisible(int waterMarkId, const bool visible);
    void removeWatermark(int id);

private:
    void drawImageData(QByteArray frameBuffer, qint64 timeT);
    void recordThread();

    Ui::VideoWidget *ui;

    int m_maxCount = 1;
    unsigned char* m_frameData;
    unsigned char* m_rgbData;
    unsigned char* m_y8Data;

    std::mutex m_mutex;

    QMap<int, WatermarkInfo> m_watermarkImgMap;

    Mapping m_map;
    MappingNew m_mapNew;
    VideoFormatInfo m_videoInfo;

    bool m_isRecording = false;
    std::thread* m_frameThread;
    BlockingQueue<FrameData> m_recordQueue;
    QString m_recordPath;
};

#endif // VIDEOWIDGET_H

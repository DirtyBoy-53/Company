#ifndef YUVPLAY_H
#define YUVPLAY_H

#include <QFile>
#include <QVector>
#include <thread>
#include "playfactory.h"
#include "videoinfo.h"



enum RecordStatus {
    RECORD_STOP = 0,
    RECORD_PALYING = 1,
    RECORD_SUPPEND = 2,
};

class YuvPlay : public PlayFactory
{
    Q_OBJECT
public:
    YuvPlay();
    ~YuvPlay();

    int openFile(QString filePath) override;
    void closeFile() override;
    void play() override;
    void suspend() override;
    void stop() override;
    void next() override;
    void last() override;
    void moveTo(const int index) override;

    void setNvsSwitch(const bool flag) override;
    bool getFrame(QString path, const int mode, const int interval, const GetFrameConfig config) override;
    void getVideoData(QString path, bool hasParam) override;


private:
    void palyProcess();

private:
    QString m_filePath;
    QFile m_playFile;
    QVector<FrameInfo_S> m_vFrameInfo;
    bool m_isRunning = false;
    int m_isPlayStatus = RECORD_STOP;
    std::thread* m_thread = nullptr;
    int m_frameCount = 0;
    qint64 m_readSize = 0;
    qint64 m_fileSize = 0;

    FrameInfo_S m_lastFrameInfo;
};

#endif // YUVPLAY_H

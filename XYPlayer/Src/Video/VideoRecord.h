#ifndef VIDEO_RECORD_H
#define VIDEO_RECORD_H

#include <QThread>
#include <QObject>
#include <fstream>
#include <QFile>
#include <QElapsedTimer>
#include <string>

#include "YStruct.h"
#include "GQueue.h"
#include "Media.h"

class VideoRecord : public QThread
{
    Q_OBJECT
public:
    VideoRecord();
    
    ~VideoRecord();
    void stop();
    void close();
    void addFrame(XYZJFramePtr frame);
    void setPath(const QString &path);
    void startRun();
    void setMedia(Media &media);
private:
    QString createFile();
    bool openFile();
    bool saveFrame(const QByteArray &frame);
    void createFrame(XYZJFramePtr data, QByteArray &frame);
    void writeVideoHead(QFile &handle);

    void saveXYZJVideo(XYZJFramePtr data, QByteArray &frame);
protected:
    void run() override;
private:
    GQueue<XYZJFramePtr>        mQueue;
    bool                        isRunning{true};
    QString                     file_name_last{""};
    QString                     file_name_cur{""};
    QFile                       file_handle;
    QElapsedTimer               elapsedTimer;
    QString                     save_path;

    Media                       mMedia;
};

#endif // VIDEO_RECORD_H

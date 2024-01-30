#ifndef PLAYFACTORY_H
#define PLAYFACTORY_H

#include <QObject>
#include "configinfo.h"

struct GetFrameConfig
{
    bool isGetNvsTxt = false;
    bool isGetParam = false;
};

class PlayFactory : public QObject
{
    Q_OBJECT
public:
    explicit PlayFactory() {}

    virtual int openFile(QString filePath) = 0;
    virtual void closeFile() = 0;
    virtual void play() = 0;
    virtual void suspend() = 0;
    virtual void stop() = 0;
    virtual void next() = 0;
    virtual void last() = 0;
    virtual void moveTo(const int index) = 0;

    virtual void setNvsSwitch(const bool flag) = 0;
    virtual bool getFrame(QString path, const int mode, const int interval, const GetFrameConfig config) = 0;
    virtual void getVideoData(QString path, bool hasParam) = 0;

signals:
    void signalUpdatePlayData(QByteArray frame, int frameCount, int playStatus);
};

#endif // PLAYFACTORY_H

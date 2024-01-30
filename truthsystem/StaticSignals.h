#ifndef STATICSIGNALS_H
#define STATICSIGNALS_H

#include <QObject>
#include <QImage>
#include "constdef.h"

enum WaterMarkEnum {
    ALG_POINT_ARROW = 0,
    BAD_POINT_ARROW = 1,
    IR_PIP_IMG = 2,
};

struct WatermarkInfo
{
    bool visible = false;
    QImage img;
    int x = 0;
    int y = 0;
};

class StaticSignals : public QObject
{
    Q_OBJECT

signals:
    void sigAddWatermark(const int videoId, int waterMarkId, const WatermarkInfo& image);
    void sigSetWatermarkVisible(const int videoId, int waterMarkId, const bool visible);
    void sigMoveWatermark(const int videoId, int waterMarkId, const int x, const int y);
    void sigUpdateX16ShowB(QByteArray bData);
public:
    static StaticSignals* getInstance()
    {
        static StaticSignals instance;
        return &instance;
    }

private:
    StaticSignals() {
        qRegisterMetaType<WatermarkInfo>("WatermarkInfo");
    }
    ~StaticSignals() {}
};

#endif // STATICSIGNALS_H

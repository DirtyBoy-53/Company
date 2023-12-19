#ifndef STATICSIGNALS_H
#define STATICSIGNALS_H

#include <QObject>

class StaticSignals : public QObject
{
    Q_OBJECT
signals:
    void statusUpdatePos(const int x, const int y);
    void statusUpdateOffset(const int offset);
    void statusUpdateEnable(const bool enable);
    void statusUpdateImg(QString path);
    void statusChangeVideoPage(const int type);
    void statusUpdateWorkResult(QString mnm, QString reticle);

    void mtfGetOnceData(QString sn);
    void kStartCollectY16(int timeout);
    void kStopCollectY16();

    void packingNotFullPrint();

public:
    static StaticSignals* getInstance()
    {
        static StaticSignals instance;
        return &instance;
    }

private:
    StaticSignals() {}
    ~StaticSignals() {}
};

#endif // STATICSIGNALS_H

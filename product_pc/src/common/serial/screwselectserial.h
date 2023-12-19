#ifndef SCREWSELECTSERIAL_H
#define SCREWSELECTSERIAL_H

#include <QObject>
#include <QTimer>
#include "cserialport.h"

struct ScrewSelectInfo {
    int posCheckState; // 螺丝批在位孔位检测
    int posWorkState;  // 夹紧孔位检测
};

typedef void(*ScrewSelectFunc)(ScrewSelectInfo info, void* param);

class ScrewSelectSerial : public QObject
{
    Q_OBJECT
public:
    ScrewSelectSerial();
    ~ScrewSelectSerial();

    bool openSerial(const QString& com);
    void closeSerial();

    void registerInfoCb(ScrewSelectFunc cb, void* param);
    void unregisterInfoCb();
    bool setPosClamped(const int pos, bool isClamp);

private slots:
    void slotTimeout();

private:
    int judgetData(unsigned char *data, int length);
    int getScrewCheckState();
    int getScrewWorkState();

    CSerialPort* m_serial;
    QTimer* m_timer;
    ScrewSelectFunc m_infoCb;
    void* m_infoParam;
};

#endif // SCREWSELECTSERIAL_H

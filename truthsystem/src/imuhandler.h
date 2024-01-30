#ifndef IMUHANDLER_H
#define IMUHANDLER_H

#include <QObject>
#include "imu.h"

class ImuHandler : public QObject
{
    Q_OBJECT
public:
    ImuHandler();
    void setImuParam(ImuParams param);
    void drawImu(QImage& img, int16_t* imuData, int64_t tmT, int w, int h);

private:
    ImuAlg m_imu;
    CurrentVelocity m_curVelocity;
};

#endif // IMUHANDLER_H

#include "imu.h"
#include <QDebug>

ImuAlg::ImuAlg()
{

}

void ImuAlg::setParam(ImuParams param)
{
    m_params = param;
}

float ImuAlg::doMean(int16_t *imuRow, int imuCout)
{
    float tmp = 0.0;
    for (int i = 0; i < imuCout; i++) {
        tmp += (float)imuRow[i];
    }
    return tmp / imuCout;
}

bool ImuAlg::processImu(int16_t *imuData, int w, int64_t tmT, CurrentVelocity &vilocity)
{
    if (vilocity.timestamp == -1) {
        vilocity.timestamp = tmT;
        return false;
    }

    IMUdata temp;
    temp.angular_acceleration_x = doMean(imuData, m_params.imu_data_num);
    temp.angular_acceleration_y = doMean(imuData + w, m_params.imu_data_num);
    temp.angular_acceleration_z = doMean(imuData + w*2, m_params.imu_data_num);
    temp.acceleration_x = doMean(imuData + w*3, m_params.imu_data_num);
    temp.acceleration_y = doMean(imuData + w*4, m_params.imu_data_num);
    temp.acceleration_z = doMean(imuData + w*5, m_params.imu_data_num);
    qDebug() << temp.acceleration_x << temp.acceleration_y << temp.acceleration_z;

    int64_t delta_t = (tmT - vilocity.timestamp);
    vilocity.vx += delta_t * (temp.acceleration_x - m_params.ax) * m_params.speed_dimension * m_params.g;
    vilocity.vy += delta_t * (temp.acceleration_y - m_params.ay) * m_params.speed_dimension * m_params.g;
    vilocity.vz += delta_t * (temp.acceleration_z - m_params.az) * m_params.speed_dimension * m_params.g;
    vilocity.timestamp = tmT;

    return true;
}


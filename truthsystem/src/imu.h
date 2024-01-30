#ifndef IMU_H
#define IMU_H

#include "videoinfo.h"

struct IMUdata
{
    float angular_acceleration_x;
    float angular_acceleration_y;
    float angular_acceleration_z;
    float acceleration_x;
    float acceleration_y;
    float acceleration_z;
};

struct CurrentVelocity {
    double vx = 0.0;        //当前的速度分量x
    double vy = 0.0;        //当前的速度分量y
    double vz = 0.0;        //当前的速度分量z
    int64_t timestamp = -1; //当前帧的时间戳
};

struct ImuParams{
    int ax = 475;     //当前安装位置x轴下的重力加速度分量
    int ay = -208;     //当前安装位置y轴下的重力加速度分量
    int az = -15335;  //当前安装位置z轴下的重力加速度分量
    float angle_dimension = 8.75;  // 角速度量纲  mdps/lsb
    float speed_dimension = 0.122; // 加速度量纲 mg/LSB
    float g = 9.8; //重力加速度m/s
    unsigned int imu_data_num = 15; //每行15个imu数据
};

class ImuAlg
{
public:
    ImuAlg();
    void setParam(ImuParams param);
    bool processImu(int16_t* imuData, int w, int64_t tmT, CurrentVelocity& vilocity);

private:  
    float doMean(int16_t* imuRow, int imuCout);
    ImuParams m_params;

    int m_count = 5;
};

#endif // IMU_H

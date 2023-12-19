#ifndef ELECLOCKSERIALMANAGER_H
#define ELECLOCKSERIALMANAGER_H

#include <QObject>
#include <QTimer>

#include "cserialport.h"

struct ElecLockWorkInfo {
//    int workId;      // 任务号
    float circleNum; // 拧紧圈数
    float mnmMax;    // 拧紧过程中最大扭力值
    int workTime;    // 拧紧时间
    int workState;   // 工作状态   1正在工作 0未工作 （0-1-0)的一个过程代表打螺丝
    int workResult;  // 工作结果   1成功 0未完成 -1失败
    int errorCode;   // 错误码 0成功 1浮高；2滑牙；3过流（断电重启）4过压（检查供电电压是否偏高）5欠压（检查供电电压是否偏低）6飞车; 7I2T过热（检查批头是否打滑，螺丝是否打滑）;8反转不到位；9位置偏差过大（检测电机线与encoder线接触是否良好）;10电批断线 ;11力矩偏差过大;12拧松失败;32试用超时;
    int direction;   // 1拧紧  2拧松

    int realMnm;   // 实时扭力
    int realRpm;   // 实时反馈速度
    int realControlRpm;  //实时指令速度
    float realCircleNum; // 实时圈数
    int realWorkTime;    // 实时运行时间
    float vol;  // 实时供电电压
};

struct ElecLockPosInfo {
    int x;     // X坐标
    int y;     // Y坐标
    bool flag;  // 校准状态，false提示需要校准，true已校准
};

enum WorkTaskAddr {
    TASK_ADDR_CONTROL_MODE = 61506,  // 电批控制模式：0X3AAA IO控制，0X3BBB PC控制
    TASK_ADDR_CONTROL = 61500,       // 电批运动： 0X2AAA开始拧紧 0X2BBB停止拧紧 / 0X2CCC开始拧松 0X2DDD停止拧松； 必须成对调用
    TASK_ADDR_ENABLE = 3927,         // 电批使能： 0 disable   1 enable
    TASK_ADDR_BACK_UP_MODE = 4000,   // 电批拧紧模式
    TASK_ADDR_BACK_UP_MNM = 4002,    // 目标扭力  0.500~4.200（放大1000倍的整数）
//    TASK_ADDR_BACK_UP_CIRCLE = 4060, // 拧紧圈数  0~100
//    TASK_ADDR_BACK_UP_SPEED = 4061,  // 拧紧速度
    TASK_ADDR_BACK_OUT_CIRCLE = 4110,// 拧松圈数 0~100
    TASK_ADDR_BACK_OUT_SPEED = 4111, // 拧松速度
    TASK_ADDR_ALARM_ENABLE = 4016,   // 是否开启报警，1开 0关，报警后再转一下蜂鸣器会停，开启报警后实时数据里有errorcode
    TASK_ADDR_ALARM_FLOAT_CIRCLE = 4009, // 报警浮高界定的圈数。开启报警有效
    TASK_ADDR_ALARM_SCREWLOOSE_CIRCLE = 4010, // 报警滑牙界定的圈数。开启报警有效
    TASK_ADDR_MNM_MAX = 4026,
    TASK_ADDR_MNM_MIN = 4027,
    TASK_ADDR_STEP0_CIRCLE = 4060,
    TASK_ADDR_STEP0_SPEED = 4061,
    TASK_ADDR_STEP1_CIRCLE = 4065,
    TASK_ADDR_STEP1_SPEED = 4066,
    TASK_ADDR_STEP2_CIRCLE = 4070,
    TASK_ADDR_STEP2_SPEED = 4071,

};


typedef void(*realDataFunc)(ElecLockWorkInfo info, void* param);
typedef void(*realPosInfoFunc)(ElecLockPosInfo info, void* param);

class ElecLockSerialManager : public QObject
{
    Q_OBJECT
public:
    ElecLockSerialManager();
    ~ElecLockSerialManager();

    /**
     * @brief openSerial 打开串口
     * @param writeCom   控制串口号
     * @param readCom    机械臂点位端口号
     * @return
     */
    bool openSerial(const QString& writeCom, const QString& readCom);
    void closeSerial();
    void setEnable(const bool flag);  // 电批使能
    bool getEnable();
    void save();  // 保存配置

    void readValue(const int addr, int& value);
    void writeValue(const int addr, const int value);

    void registerRealDataCb(realDataFunc cb, void* param);
    void unregisterRealDataCb();
    void registerRealPosInfoCb(realPosInfoFunc cb, void* param);
    void unregisterRealPosInfoCb();

private slots:
    void slotTimeout();
    void slotPosTimeout();

private:
    void initSerial();
    void getCRCCmd(unsigned char *cmd, const unsigned int len);
    int judgetData(unsigned char *data, int length);
    // 获取实时工作信息
    bool getElecLockWorkInfo(ElecLockWorkInfo& info);
    // 获取点位信息
    bool getElecLockPosInfo(ElecLockPosInfo& info);

    CSerialPort* m_serial;
    CSerialPort* m_posSerial;
    QByteArray m_cache;
    QByteArray m_posCache;

    QTimer* m_timer;
    QTimer* m_posTimer;

    realDataFunc m_realDataCb;
    void* m_realDataParam;
    realPosInfoFunc m_realPosInfoCb;
    void* m_realPosInfoParam;

    bool m_enable = false;
};

#endif // ELECLOCKSERIALMANAGER_H

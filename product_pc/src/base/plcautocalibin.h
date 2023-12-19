#ifndef PlcAutoCalibin_H
#define PlcAutoCalibin_H

#include <QObject>
#include "cserialport.h"
#include <singleton.h>

enum PlcAxisDireciton {
    PLC_AXIS_DIRECITON_30N = -30,
    PLC_AXIS_DIRECITON_15N = -15,
    PLC_AXIS_DIRECITON_0 = 0,
    PLC_AXIS_DIRECITON_15P = 15,
    PLC_AXIS_DIRECITON_30P = 30,
};

enum PlcAxisDistance {
    PLC_AXIS_DISTANCE_45CM = 45,
    PLC_AXIS_DISTANCE_50CM = 50,
    PLC_AXIS_DISTANCE_60CM = 60,
    PLC_AXIS_DISTANCE_70CM = 70,
    PLC_AXIS_DISTANCE_80CM = 80,
    PLC_AXIS_DISTANCE_90CM = 90,
    PLC_AXIS_DISTANCE_100CM = 100
};

enum HandleControlMode {
    HANDLE_CONTROL_UP = 0,     // 工作位
    HANDLE_CONTROL_DOWN         // 上料位
};

class PlcAutoCalibin
{
public:
    PlcAutoCalibin();
    ~PlcAutoCalibin();
    void setConfig(QString writeName, QString readName);
    bool openSerial(const QString& writeName, const QString& readName);
    void closeSerial();

    int judgetData(unsigned char *data, int length);

    /**
     * @brief reset 设备整体复位
     */
    bool reset();

    /**
     * @brief stop 设备停止运行
     */
    bool stop();

    bool handleControl(const HandleControlMode mode);

    bool moveAxis(const int distance, const int direction);

    bool moveOrigin();

private:

    bool sendCommand(unsigned char* data, const int len);

    CSerialPort* m_writeSerial;
    CSerialPort* m_readSerial;
    QString m_wName;
    QString m_rName;
};

#endif // PLCSERIALMANAGER_H

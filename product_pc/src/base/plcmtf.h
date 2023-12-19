#ifndef PlcMtf_H
#define PlcMtf_H

#include <QObject>
#include <IDataLinkTransport.h>
#include <singleton.h>
#include <QLibrary>
#include <QFile>
#include <QCoreApplication>

enum AxisIndex {
    PLC_AXIS_X = 0,
    PLC_AXIS_Y,
    PLC_AXIS_Z,
    PLC_AXIS_R,
    PLC_AXIS_H,
    PLC_AXIS_V,
};

enum AxisMoveDirection {
    PLC_AXIS_MOVE_LEFT = 0,
    PLC_AXIS_MOVE_RIGHT,
};

enum AxisMoveMode {
    PLC_AXIS_MOVE_LONG = 0,  // 持续运动
    PLC_AXIS_MOVE_STEP,      // 步进运动
};

enum GasHandleIndex {
    PLC_GAS_LOAD = 0,
    PLC_GAS_CLAMP
};

enum GasHandleMode {
    GAS_HANDLE_WORK = 0,     // 气缸工作位
    GAS_HANDLE_RESET         // 气缸上料位
};

class PlcMtf : public Singleton<PlcMtf>
{
public:
    PlcMtf();
    ~PlcMtf();

    void setActive(bool bActive = true);
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
    void clearErrorFlag();
    int errorFlag();
    bool setAutoWorkMode();

    /**
     * @brief handleLoad  上料气缸动作
     * @param mode        上料、下料
     */
    bool handleLoad(const int mode);

    /**
     * @brief handleClamp 定位气缸动作
     * @param mode
     */
    bool handleClamp(const int mode);

    /**
     * @brief stopMove  停止轴运动
     * @param axis      轴id AxisIndex
     * @param direction 方向 AxisMoveDirection
     */
    bool stopMove(const int axis, const int direction);

    /**
     * @brief rasize  回校正位
     * @param axis    轴id
     */
    bool rasize(const int axis);

    /**
     * @brief changeMoveMode  改变轴运动模式
     * @param axis            轴id
     * @param mode            运动模式 AxisMoveMode
     */
    bool changeMoveMode(const int axis, const int mode);

    /**
     * @brief setMoveStep  设置单次步进运动长度
     * @param axis         轴id
     * @param value        运动距离
     */
    bool setMoveStep(const int axis, const int value);

    /**
     * @brief setMoveSpeed  设置轴运动速度
     * @param axis          轴id
     * @param value         运动速度
     */
    bool setMoveSpeed(const int axis, const int value);

    /**
     * @brief moveLeft  轴向左运动
     * @param axis      轴id
     */
    bool moveLeft(const int axis);

    /**
     * @brief moveRight  轴向右运动
     * @param axis       轴id
     */
    bool moveRight(const int axis);


    bool getSerialResource();

    void  freeSerialResource();

    bool initResource(QString writeName, int writeBandrate, QString readName, int readBandrate, bool hex);

    void deInitResource();
    QString mErrString;
private:

    bool sendCommand(unsigned char* data, const int len, bool getRet = true);
    bool hasError();

    IDataLinkTransport* m_writeSerial;
    IDataLinkTransport* m_readSerial;
    QString m_wName;
    QString m_rName;
    bool mPlcInitializeOk = false;

    QString mDllPath;
    QLibrary mLibSerial;

    bool mBoolActive = true;
    int m_errorFlag = 0;
};

#endif // PLCSERIALMANAGER_H

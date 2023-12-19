#ifndef MCUSERIALMANAGER_H
#define MCUSERIALMANAGER_H

#include <QObject>
#include "cserialport.h"


struct McuSysInfo {
    int ioState;
};

struct McuWorkInfo {
    float volOut;
    float volAdc1_inp2;
    float volAdc1_inp5;
    float volAdc1_inp6;
    float volAdc1_inp9;
    float volAdc1_inp10;
    float volAdc1_inp16;
    float volAdc2_inp2;// 控制盒电压
    float volAdc2_inp6;// 摄像头电压
    float volAdc3_inp0;
    float volAdc3_inp1;
    float volAdc3_inp4;
    float volAdc3_inp5;
    float volAdc3_inp7;
    float volAdc3_inp9;
    float volAdc3_inp13;
    float volAdc3_inp15;
    float volAdc3_inp16;
    QString version;
};

struct ArmWorkInfo {
    float elecValue;
    int canWorkFlag;     // CAN准备好标志 1准备好
    int canWorkEnable;   // CAN检测结果   1成功
    QString mcuVersion;
    QString asicVersion;
    QString armVersion;
    int mcuAuthorCompleted; // 授权完成 1完成
    int mcuAuthorValid; // 已授权 1为已授权
    int rs232CheckFlag;  // rs232检测准备好标志   1 准备好
    int rs232CheckResult;// rs232检测结果        1检测成功
    int snWriteResult; // sn写入结果，只有1写入成功 2失败，之前已写入过 只有1、2有效
	int dateWriteResult; // sn写入结果，只有1写入成功 2失败，之前已写入过 只有1、2有效
    int oemFlag;  // 1有效 0无效
    QString oemHwVersion;  // oem硬件版本号
    QString oemSwVersion;  // oem软件版本号
};

struct SensorWorkInfo {
    float elecValue;
    QString version;
};

struct SysWorkInfo {
    bool bPressStatus; // true is pressed, false unpressed with the test fixture
    int  nVersion; // 版本
    int  nServer2;
};

enum CommonSwitch {
    COMMON_SWITCH_OFF = 0,
    COMMON_SWITCH_ON
};

enum McuWorkMode {
    MCU_WORK_MODE_DEFAULT = 0,
    MCU_WORK_MODE_SET     = 1,
    MCU_WORK_MODE_CHECK   = 2
};

enum SysWorkMode {
    MODE_USB_ADC = 0,
    MODE_SN_DATE = 1,
    MODE_CAN_232 = 2
};

class Q_API_FUNCTION McuSerialManager : public QObject
{
    Q_OBJECT
public:
    McuSerialManager();
    ~McuSerialManager();

    QString errString();

    bool openSerial(const QString& mcuCom);
    void closeSerial();
    int  version();


    /***************
     * 单片机板卡
     * *************/
    // 查询MCU工作信息
    bool getMcuWorkInfo(McuWorkInfo& info);

    bool pressed();
    bool setSysMode(SysWorkMode mode);

    // 查询MCU系统信息
    bool getSysWorkInfo(SysWorkInfo& info);

    // 控制绿灯亮灭
    bool controlGreenLight(const CommonSwitch mode);

    // 控制红灯亮灭
    bool controlRedLight(const CommonSwitch mode);

    // 控制蜂鸣器
    bool controlBuzzer(const CommonSwitch mode);

    // P_5V_OUT上下电
    bool controlSysPower(const CommonSwitch mode);
    // MCU板卡功能切换
    bool changeMcuWorkMode(const McuWorkMode mode);

    /***************
     * 控制盒
     * *************/
    // 查询控制盒工作信息
    bool getArmWorkInfo(ArmWorkInfo& info);
    bool getArmWorkInfoB(ArmWorkInfo& info);
    // ARM宽电压控制
    bool controlArmPower(const int voltage, bool checkRst = true, bool step = true);
    bool controlArmPowerStepUp(const int voltage);
    // 写SN
    bool writeSn(const QString sn);
    // 读SN
    bool readSn(QString& sn);

    /***************
     * 摄像头
     * *************/
    bool getSensorWorkInfo(SensorWorkInfo& info);
    float getSensorVoltage();
    float getArmVoltage();
    float getSensorCurrent();
    float getArmCurrent();
    // SENSOR电压控制
    bool controlSensorPower(const int voltage, bool checkRst = true, bool step = true);
    bool controlSensorPowerStepUp(const int voltage);

private:
    void initSerial();
    void getCRCCmd(unsigned char *cmd, const unsigned int len);
    int getMcuCrcCmd(unsigned char* cmd, const int len);
    bool cmpCRCCmd(unsigned char* cmd, const unsigned int len);
    bool checkRespAck();

    CSerialPort* m_serial;
    QByteArray m_cache;
    QString mErrString;
    int m_currentArmPower = 0.0f;
    int m_currentSensorPower =0.0f;
    int m_versionId = 0;
};

#endif // MCUSERIALMANAGER_H

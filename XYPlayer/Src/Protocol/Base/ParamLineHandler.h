#ifndef PARAMLINEHANDLER_H
#define PARAMLINEHANDLER_H

#include <QObject>


struct ParamLineInfo {
    uint8_t shutterFlag;    // 快门状态
    uint8_t shutterMode;    // 快门工作模式
    uint16_t closeFlag;     // 大闭环标志
    uint16_t nucFlag;       // 本地NUC标志
    uint8_t adFlag;         // AD使能标志
    uint8_t autoFlag;       // 自适应补偿开关
    uint8_t colorType;      // 伪彩状态
    uint8_t freezeStatus;   // 冻结状态
    int16_t jTemp;          // 焦温
    uint8_t outputMode;     // 数据格式
    uint8_t sceneMode;      // 场景模式

    uint16_t comType;       // 补偿触发类型
    int32_t hist;           // 宽动态范围
    int32_t avgAd;          // AD平均值

    uint16_t adc0;          // ADC0 值
    uint16_t adc1;          // ADC1 值

    int16_t ringTemp;       // 加热环温度
    int16_t pcbTemp;        // PCB温度

    uint8_t ringState;      // 加热环模式

    uint8_t ringSwitch;     // 加热环开关

    uint8_t light;          // 亮度
    uint8_t contrast;       // 对比度
    uint8_t enhance;        // 场景增强
    uint8_t doNoise;        // 降噪

    uint8_t errCode[8];           //错误码

    uint8_t bigGear;        
    uint8_t smallGearL;
    uint8_t smallGearH;
    uint16_t bDiff;         //故障码
};

class ParamLineHandler
{
public:
    ParamLineHandler();

    ParamLineInfo parseParamLine(QByteArray data);
    ParamLineInfo getParamInfo();

    QStringList &getParamStr();
private:
    ParamLineInfo m_paramInfo;
    QStringList mParamStr;
};

#endif // PARAMLINEHANDLER_H

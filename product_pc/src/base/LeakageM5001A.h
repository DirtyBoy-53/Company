#ifndef LEAKAGEM5001A_H
#define LEAKAGEM5001A_H

#include "CSerialDirect.h"
#include "crc16_modbus.h"

typedef enum {
    CycleID_Start_W = 1,
    CycleID_Stop_W = 2,
    CycleID_Start_WR = 3,
    CycleID_Over_R = 4,
    CycleID_Testing_R = 5
} CycleID;

typedef enum {
    LS_NoTest = 0,
    LS_Verify = 1,
    LS_Pump = 2,
    LS_Keep = 3,
    LS_Test = 4,
    LS_PumpOut = 5,
    LS_Over = 6,
    LS_PrePumpIn = 7,
    LS_VectorPumpIn = 8
} LeakageStatus;

typedef struct {
    float imPressure;
    float imLeakage;
    ushort imStatus;//
    ushort lastResult; // 1 pass, 2 fail, 0 no result
    float lastPressure;
    float lastLeakage;
    float imPumpTimer;
    float imStableTimer;
    float imVerifyTimer;
    float imPumpOutTimer;
    float imLargeLeakage;
    ushort lastErrorCode; // 0 pass, 1 leakage, 2 large leak
} LeakageResult;
class LeakageM5001A : public CSerialDirect
{
public:
    LeakageM5001A();

    bool startTest();

    bool stopTestManual();

    bool isTestOver(bool & excetStoped);


    bool getImmidiateStatus(LeakageResult& result);

    bool getPressure(float & pressure);

    LeakageStatus getStep();


//    float readPressureAddr10()
//    {
//        int nlen = 8;
//        uchar cmd[] = {0x01, 0x05, 0x00, 0x02, 0xFF, 0x00, 0xCD, 0xCA};
//        return readSingleCircle(cmd, nlen);
//    }

//    bool write30KpaAddr10()
//    {
//        int nlen = 13;
//        uchar cmd[] = {0x01, 0x10, 0x00, 0x0A, 0x00, 0x02, 0x04, 0x41, 0xF0, 0x00, 0x00, 0xCD, 0xCA};
//        writeCycle(cmd, nlen);
//    }

private:
    // 写线圈
    bool writeSingleCircle(uchar addr, bool flag);
    bool readSingleCircle(uchar addr, uchar & value);

    // 按照说明文档，目前只有2个字节，16位
    // 从起始线圈位0x01读取0x10个数量的线圈状态
    bool readMultiCircle16bits(ushort & value);

    bool readRegister(uchar addr, int nlen, ushort* & value);

    float getFloat(uint32_t value);

};

#endif // LEAKAGEM5001A_H

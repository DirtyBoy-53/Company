#include "LeakageM5001A.h"
#include <QDebug>

LeakageM5001A::LeakageM5001A()
{

}

bool LeakageM5001A::startTest()
{
    qDebug() << "通知开始测试>>>>>>>>>>>>>:";
    return writeSingleCircle(0x0001, true);
}

bool LeakageM5001A::stopTestManual()
{
    qDebug() << "通知停止测试>>>>>>>>>>>>>:";
    return writeSingleCircle(0x0002, true);
}

bool LeakageM5001A::isTestOver(bool &excetStoped)
{
    ushort value = 0;
    if (readMultiCircle16bits(value)) {
        qDebug() << ">>>>>>>>>>执行线圈读取指令 值：" << QString::number(value, 16);
        if ((value & 0x0008) == 0x0008) {
            qDebug() << ">>>>>>>>>>执行线圈读取指令 正常退出";
            excetStoped = false;
            return true;
        }
        if ((value & 0x0100) == 0x0100) {
            qDebug() << ">>>>>>>>>>执行线圈读取指令 异常退出";
            excetStoped = true;
            return true;
        }
    }
    qDebug() << ">>>>>>>>>>执行线圈读取指令失败";

    return false;
}

//typedef struct {
//    float imPressure;
//    float imLeakage;
//    ushort imStatus;//
//    ushort lastResult; // 1 pass, 2 fail, 0 no result
//    float lastPressure;
//    float lastLeakage;
//    float imPumpTimer;
//    float imStableTimer;
//    float imVerifyTimer;
//    float imPumpOutTimer;
//    float imLargeLeakage;
//    ushort lastErrorCode; // 0 pass, 1 leakage, 2 large leak
//} LeakageResult;

bool LeakageM5001A::getImmidiateStatus(LeakageResult& result)
{
    ushort value[1024] = {0x00};
    ushort * pValue = (ushort*)value;
    if (readRegister(26, 46, pValue)) {
        uint32_t pInt = ((pValue[0] << 16 ) & 0xFFFF0000 ) | (pValue[1] & 0x0000FFFF);
        result.imPressure = getFloat(pInt);
        qDebug() << "获取到气压值(IEEE754)：" << pInt << "获取到气压值(float)：" << result.imPressure;

        pInt = ((pValue[2] << 16 ) & 0xFFFF0000 ) | (pValue[3] & 0x0000FFFF);
        result.imLeakage = getFloat(pInt);
        qDebug() << "获取到泄露值(IEEE754)：" << pInt << "获取到泄露值(float)：" << result.imLeakage;

        pInt = ((pValue[2] << 16 ) & 0xFFFF0000 ) | (pValue[3] & 0x0000FFFF);
        result.imLeakage = getFloat(pInt);
        qDebug() << "获取到泄露值(IEEE754)：" << pInt << "获取到泄露值(float)：" << result.imLeakage;

        result.imStatus = (LeakageStatus)value[12];
        result.lastResult = value[13];
        qDebug() << "获取到仪表测试状态：" << result.imStatus << "获取到仪表最终测试结果：" << result.lastResult;

        pInt = ((pValue[14] << 16 ) & 0xFFFF0000 ) | (pValue[15] & 0x0000FFFF);
        result.lastPressure = getFloat(pInt);
        qDebug() << "测试完成后压力(IEEE754)" << pInt << "测试完成后压力(float)：" << result.lastPressure;

        pInt = ((pValue[16] << 16 ) & 0xFFFF0000 ) | (pValue[17] & 0x0000FFFF);
        result.lastLeakage = getFloat(pInt);
        qDebug() << "测试完成后泄露(IEEE754)" << pInt << "测试完成后泄露(float)：" << result.lastLeakage;

        pInt = ((pValue[30] << 16 ) & 0xFFFF0000 ) | (pValue[31] & 0x0000FFFF);
        result.imPumpTimer = getFloat(pInt);
        qDebug() << "当前充气时间(IEEE754)" << pInt << "当前充气时间(float)：" << result.imPumpTimer;

        pInt = ((pValue[32] << 16 ) & 0xFFFF0000 ) | (pValue[33] & 0x0000FFFF);
        result.imStableTimer = getFloat(pInt);
        qDebug() << "当前保压时间(IEEE754)" << pInt << "当前保压时间(float)：" << result.imStableTimer;

        pInt = ((pValue[34] << 16 ) & 0xFFFF0000 ) | (pValue[35] & 0x0000FFFF);
        result.imVerifyTimer = getFloat(pInt);
        qDebug() << "当前检测时间(IEEE754)" << pInt << "当前检测时间(float)：" << result.imVerifyTimer;

        pInt = ((pValue[36] << 16 ) & 0xFFFF0000 ) | (pValue[37] & 0x0000FFFF);
        result.imPumpOutTimer = getFloat(pInt);
        qDebug() << "当前放气时间(IEEE754)" << pInt << "当前放气时间(float)：" << result.imPumpOutTimer;

        pInt = ((pValue[45] << 16 ) & 0xFFFF0000 ) | (pValue[46] & 0x0000FFFF);
        result.imLargeLeakage = getFloat(pInt);
        qDebug() << "当前大漏值(IEEE754)" << pInt << "当前大漏值(float)：" << result.imLargeLeakage;

        result.lastErrorCode = pValue[47];
        return true;
    }

    return false;
}

bool LeakageM5001A::getPressure(float &pressure)
{
    ushort value[3] = {0x00};
    ushort * pValue = (ushort*)value;
    if (readRegister(26, 3, pValue)) {
        pressure = value[0];
        return true;
    }

    return false;
}

LeakageStatus LeakageM5001A::getStep()
{
    ushort value[1];
    ushort * pValue = (ushort*)value;
    readRegister(38, 1, pValue);
    switch(value[0]) {
    case LS_NoTest:
        qDebug() << "没有开始测试";
        break;
    case LS_Verify:
        qDebug() << "充气前检验阶段";
        break;
    case LS_Pump:
        qDebug() << "充气阶段";
        break;
    case LS_Keep:
        qDebug() << "保压阶段";
        break;
    case LS_Test:
        qDebug() << "测试阶段";
        break;
    case LS_PumpOut:
        qDebug() << "泄气阶段";
        break;
    case LS_Over:
        qDebug() << "测试结束";
        break;
    case LS_PrePumpIn:
        qDebug() << "预先充气";
        break;
    case LS_VectorPumpIn:
        qDebug() << "容积充气";
        break;
    }

    return (LeakageStatus)value[0];
}

bool LeakageM5001A::writeSingleCircle(uchar addr, bool flag)
{
    int len = 8;
    uchar cmd[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
    cmd[2] = (addr >> 0x08) & 0xFF;
    cmd[3] = addr & 0xFF;
    if (flag) {
        cmd[4] = 0xFF;
        cmd[5] = 0x00;
    } else {
        cmd[4] = 0x00;
        cmd[5] = 0x00;
    }

    ushort crc = crc16_modbus(cmd, len - 2);
    cmd[6] = crc & 0xFF;
    cmd[7] = (crc >> 8) & 0xFF;

    write(cmd, len);
    uchar retHex[16] = {0x00};
    int retLen = 0;

    char* pRet = (char*)retHex;
    int requreLen = 8;
    if (get((char*)cmd, 2, nullptr, requreLen, pRet, retLen)) {
        qDebug("Return Length: %02d, Need Length=: %02d", retLen, requreLen);
        ushort retCrc = (retHex[requreLen - 2] & 0xFF) | ((retHex[requreLen - 1] << 8) & 0xFF00);
        ushort crcRetCal = crc16_modbus(retHex, requreLen - 2);
        qDebug("Return CRC: %04X, Calculate CrC: %04X", retCrc, crcRetCal);
        if (retCrc != crcRetCal) {
            return false;
        }

        return true;
    }

    return false;

}

bool LeakageM5001A::readSingleCircle(uchar addr, uchar &value)
{
    int len = 8;
    uchar cmd[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
    cmd[2] = (addr >> 0x08) & 0xFF;
    cmd[3] = addr & 0xFF;
    cmd[4] = (value >> 0x08) & 0xFF;
    cmd[5] = value & 0xFF;

    ushort crc = crc16_modbus(cmd, len - 2);
    cmd[6] = crc & 0xFF;
    cmd[7] = (crc >> 8) & 0xFF;

    write(cmd, len);
    uchar retHex[16] = {0x00};
    int retLen = 0;

    char* pRet = (char*)retHex;
    int requireLen = 6;
    if (get((char*)cmd, 2, nullptr, requireLen, pRet, retLen)) {
        ushort retCrc = (retHex[requireLen - 2] & 0xFF) | ((retHex[requireLen - 1] << 8) & 0xFF00);
        ushort crcRetCal = crc16_modbus(retHex, requireLen - 2);
        qDebug("Return CRC: %04X, Calculate CrC: %04X", retCrc, crcRetCal);
        if (retCrc != crcRetCal) {
            return false;
        }

        // 单个读取的长度目前来看只能位1
        if (retHex[2] != 0x01) {
            return false;
        }

        value = retHex[3]; // 单个读写，不用考虑长度，直接读数据

        return true;
    }

    return false;
}

bool LeakageM5001A::readMultiCircle16bits(ushort &value)
{
    int len = 8;
    uchar cmd[] = {0x01, 0x01, 0x00, 0x01, 0x00, 0x10, 0xFD, 0xCA};
    ushort crc = crc16_modbus(cmd, len - 2);
    cmd[6] = crc & 0xFF;
    cmd[7] = (crc >> 8) & 0xFF;

    for (int foo = 0; foo < len; foo++) {
        qDebug("id %d, value cmd[%02X]", foo, cmd[foo]);
    }
    write(cmd, len);
    uchar retHex[16] = {0x00};
    int retLen = 0;
    int requireLen = 7;

    char* pRet = (char*)retHex;
    if (get((char*)cmd, 2, nullptr, requireLen, pRet, retLen)) {
        ushort retCrc = (retHex[requireLen - 2] & 0xFF) | ((retHex[requireLen - 1] << 8) & 0xFF00);
        ushort crcRetCal = crc16_modbus(retHex, requireLen - 2);
        qDebug("Return CRC: %04X, Calculate CrC: %04X", retCrc, crcRetCal);
        if (retCrc != crcRetCal) {
            return false;
        }

        // 单个读取的长度目前来看只能位2
        if (retHex[2] != 0x02) {
            return false;
        }

        value = (retHex[3] & 0xFF ) | ((retHex[4] << 8) & 0xFF00 ); // 单个读写，不用考虑长度，直接读数据

        return true;
    }

    return false;
}

bool LeakageM5001A::readRegister(uchar addr, int nlen, ushort *&value)
{
    int len = 8;
    uchar cmd[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA};
    cmd[2] = (addr >> 0x08) & 0xFF;
    cmd[3] = addr & 0xFF;
    cmd[4] = (nlen >> 0x08) & 0xFF;
    cmd[5] = nlen & 0xFF;
    ushort crc = crc16_modbus(cmd, len - 2);
    cmd[6] = crc & 0xFF;
    cmd[7] = (crc >> 8) & 0xFF;

    qDebug() << "Write String: ";
    for (int foo = 0; foo < len; foo++) {
        qDebug() << "0x" << QString::number(cmd[foo], 16);
    }
    write(cmd, len);
    uchar retHex[1024] = {0x00};
    int retLen = 0;


    int requreLen = nlen * 2 + 2 + 3; // 3字节头部(01 03 长度) + nlen * 2字节返回值 + 2字节CRC
    char* pRet = (char*)retHex;
    if (get((char*)cmd, 2, nullptr, requreLen, pRet, retLen)) {
        qDebug("Return Length: %02d, Need Length=: %02d", retLen, requreLen);
        ushort retCrc = (retHex[requreLen - 2] & 0xFF) | ((retHex[requreLen - 1] << 8) & 0xFF00);
        ushort crcRetCal = crc16_modbus(retHex, requreLen - 2);
        qDebug("Return CRC: %04X, Calculate CrC: %04X", retCrc, crcRetCal);
        if (retCrc != crcRetCal) {
            qDebug() << "DDDDDDDDDDDDDDDDDDD>> CRC SHIBAI";
            return false;
        }

        // 单个读取的长度目前来看只能位1
        qDebug("Return Length: %02d, Need Length=: %02d", retHex[2], nlen * 2);
        if (retHex[2] != nlen * 2) {
            qDebug() << "DDDDDDDDDDDDDDDDDDD>> LENGTH SHIBAI";
            return false;
        }

        uchar * pstart = (uchar*)(pRet + 3);
        for (int foo = 0; foo < nlen; foo++) {
            value[foo] = ((pstart[2 * foo] << 8 )& 0xFF00) | (pstart[2 * foo+1] & 0xFF); // 单个读写，不用考虑长度，直接读数据
            qDebug() << "读取的值： " << foo << value[foo];
        }

        return true;
    }

    return false;
}

float LeakageM5001A::getFloat(uint32_t value)
{
    union {
        float f;
        uint32_t u;
    }X;

    X.u = value;
    return X.f;
}

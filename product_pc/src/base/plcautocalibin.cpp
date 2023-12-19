#include "plcautocalibin.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>


static const int g_plcRespLen = 19;
static const char g_plcResp[9] = {0x01, 0x10, 0x13, 0x88, 0x00, 0x05, 0x0A, 0x00, 0x01};

static unsigned char g_plcReset[8] = { 0x01, 0x05, 0x20, 0x5B, 0xFF, 0x00, 0xF6, 0x29 };
static unsigned char g_plcStop[8]  = { 0x01, 0x05, 0x20, 0x3D, 0xFF, 0x00, 0x16, 0x36 };

static unsigned char g_handleUp   [8] = { 0x01, 0x05, 0x20, 0xCE, 0xFF, 0x00, 0xE6, 0x05 };
static unsigned char g_handleDown [8] = { 0x01, 0x05, 0x20, 0xCE, 0x00, 0x00, 0xA7, 0xF5 };
static unsigned char g_plcOrigin  [8] = { 0x01, 0x05, 0x23, 0xE8, 0xFF, 0x00, 0x07, 0x8A };

//XD01A治具
static unsigned char g_plc45CM_0  [8] = { 0x01, 0x05, 0x25, 0x50, 0xFF, 0x00, 0x87, 0x27 };
static unsigned char g_plc45CM_15N[8] = { 0x01, 0x05, 0x25, 0x52, 0xFF, 0x00, 0x26, 0xE7 };
static unsigned char g_plc45CM_30N[8] = { 0x01, 0x05, 0x25, 0x54, 0xFF, 0x00, 0xC6, 0xE6 };
static unsigned char g_plc45CM_15P[8] = { 0x01, 0x05, 0x25, 0x56, 0xFF, 0x00, 0x67, 0x26 };
static unsigned char g_plc45CM_30P[8] = { 0x01, 0x05, 0x25, 0x58, 0xFF, 0x00, 0x06, 0xE5 };

static unsigned char g_plc50CM_0  [8] = { 0x01, 0x05, 0x25, 0x5A, 0xFF, 0x00, 0xA7, 0x25 };
static unsigned char g_plc50CM_15N[8] = { 0x01, 0x05, 0x25, 0x5C, 0xFF, 0x00, 0x47, 0x24 };
static unsigned char g_plc50CM_30N[8] = { 0x01, 0x05, 0x25, 0x5E, 0xFF, 0x00, 0xE6, 0xE4 };
static unsigned char g_plc50CM_15P[8] = { 0x01, 0x05, 0x25, 0x60, 0xFF, 0x00, 0x87, 0x28 };
static unsigned char g_plc50CM_30P[8] = { 0x01, 0x05, 0x25, 0x62, 0xFF, 0x00, 0x26, 0xE8 };

static unsigned char g_plc60CM_0  [8] = { 0x01, 0x05, 0x29, 0x38, 0xFF, 0x00, 0x05, 0xAB };
static unsigned char g_plc60CM_15N[8] = { 0x01, 0x05, 0x29, 0x3A, 0xFF, 0x00, 0xA4, 0x6B };
static unsigned char g_plc60CM_30N[8] = { 0x01, 0x05, 0x29, 0x3C, 0xFF, 0x00, 0x44, 0x6A };
static unsigned char g_plc60CM_15P[8] = { 0x01, 0x05, 0x29, 0x3E, 0xFF, 0x00, 0xE5, 0xAA };
static unsigned char g_plc60CM_30P[8] = { 0x01, 0x05, 0x29, 0x40, 0xFF, 0x00, 0x85, 0xB2 };

static unsigned char g_plc70CM_0  [8] = { 0x01, 0x05, 0x29, 0x42, 0xFF, 0x00, 0x24, 0x72 };
static unsigned char g_plc70CM_15N[8] = { 0x01, 0x05, 0x29, 0x44, 0xFF, 0x00, 0xC4, 0x73 };
static unsigned char g_plc70CM_30N[8] = { 0x01, 0x05, 0x29, 0x46, 0xFF, 0x00, 0x65, 0xB3 };
static unsigned char g_plc70CM_15P[8] = { 0x01, 0x05, 0x29, 0x48, 0xFF, 0x00, 0x04, 0x70 };
static unsigned char g_plc70CM_30P[8] = { 0x01, 0x05, 0x29, 0x4A, 0xFF, 0x00, 0xA5, 0xB0 };

static unsigned char g_plc80CM_0  [8] = { 0x01, 0x05, 0x3C, 0xC0, 0xFF, 0x00, 0x80, 0x56 };
static unsigned char g_plc80CM_15N[8] = { 0x01, 0x05, 0x3C, 0xC2, 0xFF, 0x00, 0x21, 0x96 };
static unsigned char g_plc80CM_30N[8] = { 0x01, 0x05, 0x3C, 0xC4, 0xFF, 0x00, 0xC1, 0x97 };
static unsigned char g_plc80CM_15P[8] = { 0x01, 0x05, 0x3C, 0xC6, 0xFF, 0x00, 0x60, 0x57 };
static unsigned char g_plc80CM_30P[8] = { 0x01, 0x05, 0x3C, 0xC8, 0xFF, 0x00, 0x01, 0x94 };

static unsigned char g_plc90CM_0  [8] = { 0x01, 0x05, 0x3C, 0xCA, 0xFF, 0x00, 0xA0, 0x54 };
static unsigned char g_plc90CM_15N[8] = { 0x01, 0x05, 0x3C, 0xCC, 0xFF, 0x00, 0x40, 0x55 };
static unsigned char g_plc90CM_30N[8] = { 0x01, 0x05, 0x3C, 0xCE, 0xFF, 0x00, 0xE1, 0x95 };
static unsigned char g_plc90CM_15P[8] = { 0x01, 0x05, 0x3C, 0xD0, 0xFF, 0x00, 0x81, 0x93 };
static unsigned char g_plc90CM_30P[8] = { 0x01, 0x05, 0x3C, 0xD2, 0xFF, 0x00, 0x20, 0x53 };

static unsigned char g_plc100CM_0  [8]= { 0x01, 0x05, 0x24, 0x56, 0xFF, 0x00, 0x66, 0xDA };
static unsigned char g_plc100CM_15N[8]= { 0x01, 0x05, 0x24, 0x57, 0xFF, 0x00, 0x37, 0x1A };
static unsigned char g_plc100CM_30N[8]= { 0x01, 0x05, 0x24, 0x58, 0xFF, 0x00, 0x07, 0x19 };
static unsigned char g_plc100CM_15P[8]= { 0x01, 0x05, 0x24, 0x59, 0xFF, 0x00, 0x56, 0xD9 };
static unsigned char g_plc100CM_30P[8]= { 0x01, 0x05, 0x24, 0x5A, 0xFF, 0x00, 0xA6, 0xD9 };


PlcAutoCalibin::PlcAutoCalibin()
{
    qRegisterMetaType<SerialParam>("SerialParam");
    m_writeSerial = new CSerialPort();
    m_readSerial = new CSerialPort();
}

PlcAutoCalibin::~PlcAutoCalibin()
{

}

void PlcAutoCalibin::setConfig(QString writeName, QString readName)
{
    m_wName = writeName;
    m_rName = readName;
}

bool PlcAutoCalibin::sendCommand(unsigned char *data, const int len)
{
    if(!openSerial(m_wName, m_rName)) {
        return false;
    }
    if(0 != m_writeSerial->sendCommand(data, len)) {
        return false;
    }
    QByteArray allData;
    int iRetryCount = 5;
    while(iRetryCount > 0) {
        QByteArray arr = m_readSerial->readSerial();

        allData.append(arr);
        if(arr.size() > 0) {
            qInfo("read size %d", arr.size());
        }
        if(allData.size() >= g_plcRespLen) {
            break;
        }
        iRetryCount--;
    }
    int currendIndex = 0;


    unsigned char* cache = (unsigned char*)allData.data();
    qInfo() << "PLC recv data " << QString(allData.toHex());
    if(allData.size() >= g_plcRespLen) {
        return false;
    }
    bool bRet = false;
    if(cache[currendIndex] == 0x01 &&
       cache[currendIndex+1] == 0x10 && cache[currendIndex+2] == 0x13 &&
       cache[currendIndex+3] == 0x88 && cache[currendIndex+4] == 0x00 &&
       cache[currendIndex+5] == 0x05 && cache[currendIndex+6] == 0x0A) {
        int axisIndex = cache[12]-1;
        switch(allData.at(8)) {
        case 1:
            qInfo("PLC Reset OK");
            bRet = true;
            break;
        case 2:
            qInfo("PLC move OK %d", axisIndex);
            bRet = true;
            break;
        case 6:
            qInfo("PLC reset ok!!! %d", axisIndex);
            bRet = true;
            break;
        case 7:
            qInfo("PLC handle up finish!!! %d", axisIndex);
            bRet = true;
            break;
        case 8:
            qInfo("PLC handle down finish!!! %d", axisIndex);
            bRet = true;
            break;
        }
    }
    return bRet;
}

bool PlcAutoCalibin::openSerial(const QString& writeName, const QString& readName)
{
    qDebug() << "open plc com " << writeName << readName;
    SerialParam param;
    param.baudRate    = QSerialPort::Baud9600;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;
    int iSucc = 0;
    if(m_writeSerial->openSerial(writeName, param, true)) {
        iSucc ++;
    }
    if(m_readSerial->openSerial(readName, param, true)) {
        iSucc ++;
    }
    qInfo("open succ count %d", iSucc);
    return iSucc >= 2 ? true : false;
}

void PlcAutoCalibin::closeSerial()
{
    qInfo("PLC com close ");
    m_writeSerial->closeSerial();
    m_readSerial->closeSerial();
}

int PlcAutoCalibin::judgetData(unsigned char *data, int length)
{
    unsigned short crc = 0xffff;
    for(int n = 0; n < length; n++){
        crc = data[n] ^ crc;
        for(int i = 0;i < 8;i++){
            if(crc & 0x01){
                crc = crc >> 1;
                crc = crc ^ 0xa001;
            }
            else{
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

bool PlcAutoCalibin::reset()
{
    qInfo() << "PLC begin to reset......";
    return sendCommand(g_plcReset, 8);
}

bool PlcAutoCalibin::stop()
{
    qInfo() << "PLC stop working";
    return sendCommand(g_plcStop, 8);
}

bool PlcAutoCalibin::handleControl(const HandleControlMode mode)
{
    unsigned char* send = nullptr;
    switch(mode) {
    case HANDLE_CONTROL_UP: {
        send = g_handleUp;
        break;
    }
    case HANDLE_CONTROL_DOWN: {
        send = g_handleDown;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC handle move %d......", mode);
        return sendCommand(send, 8);
    }
    return false;
}

bool PlcAutoCalibin::moveAxis(const int distance, const int direction)
{
    unsigned char* send = nullptr;
    switch(direction)
    {
    case PLC_AXIS_DIRECITON_30N:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = g_plc45CM_30N;
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = g_plc50CM_30N;
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = g_plc60CM_30N;
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = g_plc70CM_30N;
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = g_plc80CM_30N;
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = g_plc90CM_30N;
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = g_plc100CM_30N;
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_15N:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = g_plc45CM_15N;
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = g_plc50CM_15N;
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = g_plc60CM_15N;
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = g_plc70CM_15N;
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = g_plc80CM_15N;
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = g_plc90CM_15N;
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = g_plc100CM_15N;
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_0:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = g_plc45CM_0;
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = g_plc50CM_0;
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = g_plc60CM_0;
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = g_plc70CM_0;
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = g_plc80CM_0;
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = g_plc90CM_0;
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = g_plc100CM_0;
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_15P:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = g_plc45CM_15P;
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = g_plc50CM_15P;
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = g_plc60CM_15P;
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = g_plc70CM_15P;
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = g_plc80CM_15P;
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = g_plc90CM_15P;
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = g_plc100CM_15P;
            break;
        }
        break;
    }
    case PLC_AXIS_DIRECITON_30P:
    {
        switch(distance) {
        case PLC_AXIS_DISTANCE_45CM:
            send = g_plc45CM_30P;
            break;
        case PLC_AXIS_DISTANCE_50CM:
            send = g_plc50CM_30P;
            break;
        case PLC_AXIS_DISTANCE_60CM:
            send = g_plc60CM_30P;
            break;
        case PLC_AXIS_DISTANCE_70CM:
            send = g_plc70CM_30P;
            break;
        case PLC_AXIS_DISTANCE_80CM:
            send = g_plc80CM_30P;
            break;
        case PLC_AXIS_DISTANCE_90CM:
            send = g_plc90CM_30P;
            break;
        case PLC_AXIS_DISTANCE_100CM:
            send = g_plc100CM_30P;
            break;
        }
        break;
    }
    default:
        break;
    }
    if(send != nullptr) {
        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC begin to move %dcm, %d ......", distance, direction);
        return sendCommand(send, 8);
    }
    return false;
}

bool PlcAutoCalibin::moveOrigin()
{
    qInfo("PLC begin to move origin......");
    return sendCommand(g_plcOrigin, 8);
}

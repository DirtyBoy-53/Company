#include "PlcMtf.h"
#include <QThread>
#include <QDebug>
#include <QMessageBox>
#include "crc16_modbus.h"

static const int g_plcRespLen = 19;
static const char g_plcResp[9] = {0x01, 0x10, 0x13, 0x88, 0x00, 0x05, 0x0A, 0x00, 0x01};

static unsigned char g_reset[8]      = {0x01, 0x05, 0x20, 0x5B, 0xFF, 0x00, 0xF6, 0x29};
static unsigned char g_stop[8]       = {0x01, 0x05, 0x20, 0x3D, 0xFF, 0x00, 0x16, 0x36};
static unsigned char g_handleClamp[8]= {0x01, 0x05, 0x20, 0xC9, 0xFF, 0x00, 0x57, 0xC4};
static unsigned char g_handleLoad[8] = {0x01, 0x05, 0x20, 0xCE, 0xFF, 0x00, 0xE6, 0x05};
static unsigned char g_setAutoMode[8]= {0x01, 0x05, 0x22, 0x14, 0xFF, 0x00, 0xC6, 0x46};

static unsigned char g_xZero[8]      = {0x01, 0x05, 0x2F, 0xAF, 0xFF, 0x00, 0xB4, 0xCF};
static unsigned char g_xRasize[8]    = {0x01, 0x05, 0x30, 0xF4, 0xFF, 0x00, 0xC2, 0xC8};
static unsigned char g_xMode[8]      = {0x01, 0x05, 0x2F, 0xA6, 0xFF, 0x00, 0x13, 0x25};
static unsigned char g_xMoveLeft[8]  = {0x01, 0x05, 0x2F, 0xA1, 0xFF, 0x00, 0x11, 0x15};
static unsigned char g_xMoveRight[8] = {0x01, 0x05, 0x2F, 0xA0, 0xFF, 0x00, 0x84, 0xCC};
static unsigned char g_xSpeed[8]     = {0x01, 0x06, 0x0F, 0xA6, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_xMoveStep[8]  = {0x01, 0x06, 0x0F, 0xC2, 0x00, 0x00, 0x00, 0x00};

static unsigned char g_yZero[8]      = {0x01, 0x05, 0x30, 0x13, 0xFF, 0x00, 0x72, 0xFF};
static unsigned char g_yRasize[8]    = {0x01, 0x05, 0x30, 0xF5, 0xFF, 0x00, 0x93, 0x08};
static unsigned char g_yMode[8]      = {0x01, 0x05, 0x2F, 0xA7, 0xFF, 0x00, 0x35, 0x0D};
static unsigned char g_yMoveLeft[8]  = {0x01, 0x05, 0x30, 0x05, 0xFF, 0x00, 0x93, 0x3B};
static unsigned char g_yMoveRight[8] = {0x01, 0x05, 0x30, 0x04, 0xFF, 0x00, 0xC2, 0xFB};
static unsigned char g_ySpeed[8]     = {0x01, 0x06, 0x10, 0x0A, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_yMoveStep[8]  = {0x01, 0x06, 0x10, 0x8A, 0x00, 0x00, 0x00, 0x00};

static unsigned char g_zZero[8]      = {0x01, 0x05, 0x30, 0x77, 0xFF, 0x00, 0x33, 0x20};
static unsigned char g_zRasize[8]    = {0x01, 0x05, 0x30, 0xF6, 0xFF, 0x00, 0x63, 0x08};
static unsigned char g_zMode[8]      = {0x01, 0x05, 0x2F, 0xA8, 0xFF, 0x00, 0x05, 0x0E};
static unsigned char g_zMoveLeft[8]  = {0x01, 0x05, 0x30, 0x69, 0xFF, 0x00, 0x53, 0x26};
static unsigned char g_zMoveRight[8] = {0x01, 0x05, 0x30, 0x68, 0xFF, 0x00, 0x02, 0xE6};
static unsigned char g_zSpeed[8]     = {0x01, 0x06, 0x10, 0x6E, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_zMoveStep[8]  = {0x01, 0x06, 0x10, 0x26, 0x00, 0x00, 0x00, 0x00};

static unsigned char g_rZero[8]      = {0x01, 0x05, 0x30, 0x9A, 0xFF, 0x00, 0xA3, 0x15};
static unsigned char g_rRasize[8]    = {0x01, 0x05, 0x30, 0xF7, 0xFF, 0x00, 0x32, 0xC8};
static unsigned char g_rMode[8]      = {0x01, 0x05, 0x2F, 0xA9, 0xFF, 0x00, 0x54, 0xCE};
static unsigned char g_rMoveLeft[8]  = {0x01, 0x05, 0x2F, 0xD2, 0xFF, 0x00, 0x24, 0xD7};
static unsigned char g_rMoveRight[8] = {0x01, 0x05, 0x2F, 0xD3, 0xFF, 0x00, 0x75, 0x17};
static unsigned char g_rSpeed[8]     = {0x01, 0x06, 0x10, 0x9A, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_rMoveStep[8]  = {0x01, 0x06, 0x0F, 0xC6, 0x00, 0x00, 0x00, 0x00};

static unsigned char g_hZero[8]      = {0x01, 0x05, 0x2F, 0xB2, 0xFF, 0x00, 0x24, 0xC9};
static unsigned char g_hRasize[8]    = {0x01, 0x05, 0x30, 0xF8, 0xFF, 0x00, 0x02, 0xCB};
static unsigned char g_hMode[8]      = {0x01, 0x05, 0x2F, 0XAA, 0xFF, 0x00, 0xA4, 0xCE};
static unsigned char g_hMoveLeft[8]  = {0x01, 0x05, 0x2F, 0xB0, 0xFF, 0x00, 0x85, 0x09};
static unsigned char g_hMoveRight[8] = {0x01, 0x05, 0x2F, 0xB1, 0xFF, 0x00, 0xD4, 0xC9};
static unsigned char g_hSpeed[8]     = {0x01, 0x06, 0x0F, 0xC8, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_hMoveStep[8]  = {0x01, 0x06, 0x10, 0x2A, 0x00, 0x00, 0x00, 0x00};

static unsigned char g_vZero[8]      = {0x01, 0x05, 0x2F, 0xB9, 0xFF, 0x00, 0x55, 0x0B};
static unsigned char g_vRasize[8]    = {0x01, 0x05, 0x30, 0xF9, 0xFF, 0x00, 0x53, 0x0B};
static unsigned char g_vMode[8]      = {0x01, 0x05, 0x2F, 0xAB, 0xFF, 0x00, 0xF5, 0x0E};
static unsigned char g_vMoveLeft[8]  = {0x01, 0x05, 0x2F, 0xB4, 0xFF, 0x00, 0xC4, 0xC8};
static unsigned char g_vMoveRight[8] = {0x01, 0x05, 0x2F, 0xB5, 0xFF, 0x00, 0x95, 0x08};
static unsigned char g_vSpeed[8]     = {0x01, 0x06, 0x0F, 0xCA, 0x00, 0x00, 0x00, 0x00};
static unsigned char g_vMoveStep[8]  = {0x01, 0x06, 0x10, 0x2C, 0x00, 0x00, 0x00, 0x00};


PlcMtf::PlcMtf()
{
    m_writeSerial = nullptr;
    m_readSerial = nullptr;
}

PlcMtf::~PlcMtf()
{

}

void PlcMtf::setActive(bool bActive)
{
    mBoolActive = bActive;
}

void PlcMtf::setConfig(QString writeName, QString readName)
{
    m_wName = writeName;
    m_rName = readName;
}

bool PlcMtf::sendCommand(unsigned char *data, const int len, bool getRet)
{
    int rstlen = 0;
    char ucRst[128] = {0x00};
    char* pResult = ucRst;

    QByteArray bax((char*)data, len);
    qDebug() << "send: " << bax;


    if (hasError()) {
        m_readSerial->linkClearBuffer();
        return false;
    }

    if (m_errorFlag != 0) return false;

    m_readSerial->linkClearBuffer();
    m_writeSerial->linkWrite((char*)data, len);

    if (!getRet) return true;

    QThread::msleep(300);
    // find...
    QByteArray header;
    header.append((char)0x01);
    header.append((char)0x10);
    header.append((char)0x13);
    header.append((char)0x88);
    header.append((char)0x00);
    header.append((char)0x05);
    header.append((char)0x0A);

    qDebug() << "header is " << header;
    if (!m_readSerial->linkGet(header.toStdString().c_str(), header.size(), nullptr, g_plcRespLen, pResult, rstlen, 10000)) {
        qDebug() << "has not found " << header << " in data " <<  ucRst;
        return false;
    }
    QByteArray result = QByteArray((char*)ucRst, rstlen);
    qDebug() << "has found " << header << " in data " <<  result << "length is " << rstlen;
    for (int foo= 0 ; foo < rstlen; foo++) {
        qDebug("%02X", result.at(foo));
    }

    if (rstlen < g_plcRespLen) {
        qDebug() << "returned data is shorter , not enough the length 19";
        return false;
    }
    unsigned char szResult[ g_plcRespLen ] = { 0x00 };
    memcpy(szResult, result.data(), g_plcRespLen - 2);
    unsigned short crc  = crc16_modbus(szResult, g_plcRespLen - 2);
    unsigned short crcRet = (((unsigned char)result.at(18) << 8) & 0xFF00) | (unsigned char)result.at(17);
    if (crc != crcRet) {
        qDebug("calc CRC: %04X, returned CRC: %04X", crc, crcRet);
        qDebug() << "CRC Calculate is failed!";
        return false;
    }
    qDebug() << "CRC compared ok";
    if (result.at(16) == 0x03) {
        qInfo("PLC axis move Failed!!!");
        m_errorFlag = -1;
        return false;
    }

    bool bRet = false;
    int axisIndex = result.at(12) - 1;
    switch(result.at(8)) {
    case 1:
        qInfo("PLC Reset OK");
        bRet = true;
        break;
    case 2:
        qInfo("PLC move OK %d", axisIndex);
        bRet = true;
        break;
    case 3:
        qInfo("PLC axis %d move Failed!!!", axisIndex);
        bRet = false;
        m_errorFlag = -1;
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
    case 9:
        qInfo("PLC Gas load handle work !!!");
        bRet = true;
        break;
    }


    if (hasError())
        return false;

    return bRet;
}

bool PlcMtf::hasError()
{
    unsigned char ucRst[128];
    memset(ucRst, 0x00, 128);
    char * pResult = (char*)ucRst;
    QByteArray header;
    header.append((char)0x01);
    header.append((char)0x10);
    header.append((char)0x13);
    header.append((char)0x88);
    header.append((char)0x00);
    header.append((char)0x05);
    header.append((char)0x0A);

    int rstlen = 0;
    if (m_readSerial->linkGet(header.toStdString().c_str(), header.size(), nullptr, g_plcRespLen, pResult, rstlen, 1000)) {
        qDebug() << ">>> has found " << header << " in data " <<  ucRst;
        if (rstlen >= g_plcRespLen) {
            unsigned char szResult[ g_plcRespLen ] = { 0x00 };
            memcpy(szResult, ucRst, g_plcRespLen);
            unsigned short crc  = crc16_modbus(szResult, g_plcRespLen - 2);
            unsigned short crcRet = (((unsigned char)ucRst[18] << 8) & 0xFF00) | (unsigned char)ucRst[17];
            if (crc == crcRet) {
                qDebug() << ">>> CRC compared ok" << ucRst[16];
                if (ucRst[16] == 0x03) {
                    qInfo(">>>PLC axis move Failed!!!");
                    m_errorFlag = -1;
                    return true;
                }
            }
        }
    }

    return false;
}

bool PlcMtf::openSerial(const QString& writeName, const QString& readName)
{
    if (!mBoolActive) return true;
    qDebug() << "open plc com " << writeName << readName;
    if (!mPlcInitializeOk) {
        m_writeSerial->linkSetFormat(FormatHex);
        if (!m_writeSerial->linkSerialInit(writeName.toLocal8Bit().data(), 9600)) {
            char szTmp[1024] = {0};
            char* pszTmp = szTmp;
            m_writeSerial->linkGetLastError(pszTmp);
            qDebug() << "err: " << szTmp;
            return false;
        }

        m_readSerial->linkSetFormat(FormatHex);
        if (!m_readSerial->linkSerialInit(readName.toLocal8Bit().data(), 9600)) {
            char szTmp[1024] = {0};
            char* pszTmp = szTmp;
            m_readSerial->linkGetLastError(pszTmp);
            qDebug() << "err: " << szTmp;
            return false;
        }
        mPlcInitializeOk = true;

        qDebug() << "PLC Serial port open ok!!!";
    }
    return mPlcInitializeOk;
}

void PlcMtf::closeSerial()
{
    if (!mBoolActive) return;
    qInfo("PLC com close ");
    if (m_readSerial) {
        m_readSerial->linkDeInit();
    }

    if (m_writeSerial) {
        m_writeSerial->linkDeInit();
    }

    mPlcInitializeOk = false;
}

int PlcMtf::judgetData(unsigned char *data, int length)
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

bool PlcMtf::reset()
{
    if (!mBoolActive) return true;
    qInfo() << "PLC begin to reset......";
    return sendCommand(g_reset, 8);
}

bool PlcMtf::stop()
{
    if (!mBoolActive) return true;
    qInfo() << "PLC stop working";
    return sendCommand(g_stop, 8);
}

void PlcMtf::clearErrorFlag()
{
    m_errorFlag =  0;
}

int PlcMtf::errorFlag()
{
    return m_errorFlag;
}

bool PlcMtf::setAutoWorkMode()
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    send = g_setAutoMode;
    send[4] = 0xFF;
    qInfo("PLC change auto work mode .");

    int crc = judgetData(send, 6);
    send[6] = crc & 0xff;
    send[7] = (crc & 0xff00) >> 8;

    return sendCommand(send, 8, false);
}

bool PlcMtf::handleLoad(const int mode)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    send = g_handleLoad;
    if(mode == GAS_HANDLE_WORK) {
        send[4] = 0xFF;
        qInfo("PLC load handle close ......");
    } else if(mode == GAS_HANDLE_RESET) {
        send[4] = 0x00;
        qInfo("PLC load handle open ......");
    }
    int crc = judgetData(send, 6);
    send[6] = crc & 0xff;
    send[7] = (crc & 0xff00) >> 8;

    return sendCommand(send, 8);
}

bool PlcMtf::handleClamp(const int mode)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    send = g_handleClamp;
    if(mode == GAS_HANDLE_WORK) {
        send[4] = 0xFF;
        qInfo("PLC load handle close ......");
    } else if(mode == GAS_HANDLE_RESET) {
        send[4] = 0x00;
        qInfo("PLC load handle open ......");
    }
    int crc = judgetData(send, 6);
    send[6] = crc & 0xff;
    send[7] = (crc & 0xff00) >> 8;

    return sendCommand(send, 8);
}

bool PlcMtf::stopMove(const int axis, const int direction)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_xMoveLeft;
        } else {
            send = g_xMoveRight;
        }
        break;
    }
    case PLC_AXIS_Y: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_yMoveLeft;
        } else {
            send = g_yMoveRight;
        }
        break;
    }
    case PLC_AXIS_Z: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_zMoveLeft;
        } else {
            send = g_zMoveRight;
        }
        break;
    }
    case PLC_AXIS_R: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_rMoveLeft;
        } else {
            send = g_rMoveRight;
        }
        break;
    }
    case PLC_AXIS_H: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_hMoveLeft;
        } else {
            send = g_hMoveRight;
        }
        break;
    }
    case PLC_AXIS_V: {
        if(direction == PLC_AXIS_MOVE_LEFT) {
            send = g_zMoveLeft;
        } else {
            send = g_zMoveRight;
        }
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        send[4] = 0x00;

        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d stop move ......", axis);
        return sendCommand(send, 8);
    }
    return false;
}

bool PlcMtf::rasize(const int axis)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xRasize;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_yRasize;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zRasize;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rRasize;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hRasize;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vRasize;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d begin to rasize ......", axis);
        return sendCommand(send, 8, false);
    }
    return false;
}

bool PlcMtf::changeMoveMode(const int axis, const int mode)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xMode;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_yMode;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zMode;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rMode;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hMode;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vMode;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        if(mode == PLC_AXIS_MOVE_LONG) {
            send[4] = 0x00;
        } else if(mode == PLC_AXIS_MOVE_STEP){
            send[4] = 0xff;
        } else {
            send[4] = 0xff;
        }

        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d change mode %d......", axis, mode);
        return sendCommand(send, 8, false);
    }
    return false;
}

bool PlcMtf::setMoveStep(const int axis, const int value)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xMoveStep;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_yMoveStep;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zMoveStep;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rMoveStep;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hMoveStep;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vMoveStep;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        send[4] = (value & 0xff00) >> 8;
        send[5] = value & 0xff;

        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d set move step %d", axis, value);
        return sendCommand(send, 8, false);
    }
    return false;
}

bool PlcMtf::setMoveSpeed(const int axis, const int value)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xSpeed;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_ySpeed;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zSpeed;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rSpeed;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hSpeed;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vSpeed;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        send[4] = (value & 0xff00) >> 8;
        send[5] = value & 0xff;

        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC crc is %x, axis %d set move step %d", crc, axis, value);
        return sendCommand(send, 8, false);
    }
    return false;
}

bool PlcMtf::moveLeft(const int axis)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xMoveLeft;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_yMoveLeft;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zMoveLeft;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rMoveLeft;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hMoveLeft;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vMoveLeft;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d begin move to left......", axis);
        return sendCommand(send, 8);
    }
    return false;
}

bool PlcMtf::moveRight(const int axis)
{
    if (!mBoolActive) return true;
    unsigned char* send = nullptr;
    switch(axis)
    {
    case PLC_AXIS_X: {
        send = g_xMoveRight;
        break;
    }
    case PLC_AXIS_Y: {
        send = g_yMoveRight;
        break;
    }
    case PLC_AXIS_Z: {
        send = g_zMoveRight;
        break;
    }
    case PLC_AXIS_R: {
        send = g_rMoveRight;
        break;
    }
    case PLC_AXIS_H: {
        send = g_hMoveRight;
        break;
    }
    case PLC_AXIS_V: {
        send = g_vMoveRight;
        break;
    }
    default:
        break;
    }

    if(send != nullptr) {
        int crc = judgetData(send, 6);
        send[6] = crc & 0xff;
        send[7] = (crc & 0xff00) >> 8;
        qInfo("PLC axis %d begin to move right......", axis);
        return sendCommand(send, 8);
    }
    return false;
}

bool PlcMtf::getSerialResource()
{
    if (!mBoolActive) return true;
    if (m_readSerial && m_writeSerial) {
        return true;
    }

    QString crtPath = QCoreApplication::applicationDirPath();
    mDllPath.clear();
    mDllPath.append(crtPath);
    mDllPath.append("/");
    mDllPath.append(DATALINKDLLNAME);

    QFile file(mDllPath);
    if(!file.exists()) {
        mErrString.append("dll file is not exist in ");
        mErrString.append(mDllPath);
        return false;
    }

    mLibSerial.setFileName(mDllPath);
    if (!mLibSerial.load()) {
        mErrString.append(mLibSerial.errorString());
        return false;
    }

    typedef bool (*OBJ)(char*, void**);
    OBJ getObj = (OBJ)mLibSerial.resolve("GetInterface");
    if (!getObj) {
        mErrString.append("get interface 'GetInterface' failed!!!");
        mLibSerial.unload();
        return false;
    }

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&m_readSerial))) {
        mErrString.append("get read interface pointer failed!!!");
        mLibSerial.unload();
        return false;
    }

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&m_writeSerial))) {
        mErrString.append("get write interface pointer failed!!!");
        mLibSerial.unload();
        return false;
    }

    if (m_readSerial && m_writeSerial) {
        mErrString.clear();
        return true;
    }

    mErrString.append("unknow error");
    mLibSerial.unload();
    return false;
}

void PlcMtf::freeSerialResource()
{
    if (!mBoolActive) return;
    if (m_readSerial) {
        m_readSerial->linkRelease();
        m_readSerial = nullptr;
    }

    if (m_writeSerial) {
        m_writeSerial->linkRelease();
        m_writeSerial = nullptr;
    }
    mLibSerial.unload();
}


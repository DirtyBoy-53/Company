#include "eleclockserialmanager.h"
#include <QThread>
#include <QDebug>
#include "crc16_modbus.h"

ElecLockSerialManager::ElecLockSerialManager()
{
    m_serial = new CSerialPort();
    m_posSerial = new CSerialPort();
    m_timer = new QTimer();
    m_posTimer = new QTimer();
    connect(m_timer,&QTimer::timeout, this, &ElecLockSerialManager::slotTimeout);
    connect(m_posTimer, &QTimer::timeout, this, &ElecLockSerialManager::slotPosTimeout);
}

ElecLockSerialManager::~ElecLockSerialManager()
{

}

bool ElecLockSerialManager::openSerial(const QString &writeCom, const QString& posCom)
{
    closeSerial();

    SerialParam param;
    param.baudRate    = QSerialPort::Baud115200;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;

    bool ret = m_serial->openSerial(writeCom, param, true);
    ret &= m_posSerial->openSerial(posCom, param, true);
    qDebug() << "open serial result " << ret;
    return ret;
}

void ElecLockSerialManager::closeSerial()
{
    if(m_timer->isActive()) {
        m_timer->stop();
    }
    if(m_posTimer->isActive()) {
        m_posTimer->stop();
    }
    m_serial->closeSerial();
    m_posSerial->closeSerial();
    m_cache.clear();
    m_posCache.clear();
}

int ElecLockSerialManager::judgetData(unsigned char *data, int length)
{
    unsigned short crc = 0xffff;
    for(int n = 0; n < length-2; n++){
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
    data[length-2] = crc & 0xff;
    data[length-1] = (crc & 0xff00) >> 8;
    return crc;
}

void ElecLockSerialManager::readValue(const int addr, int &value)
{
    unsigned char cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = (addr & 0xff00) >> 8;
    cmd[3] = addr & 0xff;
    cmd[5] = 1;
    judgetData(cmd, 8);
    unsigned char* resp = (unsigned char*)m_serial->sendWaitForResp(cmd, 8, 7).data();
    if(resp[0] == 0x01 && resp[1] == 0x03) {
        value = (resp[3] << 8) | resp[4];
    }
    qDebug() << "value is " << value;
}

void ElecLockSerialManager::writeValue(const int addr, const int value)
{
    unsigned char cmd[8] = {0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = (addr & 0xff00) >> 8;
    cmd[3] = addr & 0xff;
    cmd[4] = (value & 0xff00) >> 8;
    cmd[5] = value & 0xff;
    judgetData(cmd, 8);
    m_serial->sendWaitForResp(cmd, 8, 8);
}

void ElecLockSerialManager::registerRealDataCb(realDataFunc cb, void *param)
{
    m_realDataCb = cb;
    m_realDataParam = param;
    if(m_timer->isActive()) {
        m_timer->stop();
    }
    m_timer->start(200);
}

void ElecLockSerialManager::unregisterRealDataCb()
{
    m_timer->stop();
}

void ElecLockSerialManager::registerRealPosInfoCb(realPosInfoFunc cb, void *param)
{
    m_realPosInfoCb = cb;
    m_realPosInfoParam = param;
    if(m_posTimer->isActive()) {
        m_posTimer->stop();
    }
    m_posTimer->start(300);
}

void ElecLockSerialManager::unregisterRealPosInfoCb()
{
    m_posTimer->stop();
}

void ElecLockSerialManager::slotTimeout()
{
    ElecLockWorkInfo info;
    if (getElecLockWorkInfo(info)) {
        if(m_realDataCb != nullptr) {
            m_realDataCb(info, m_realDataParam);
        }
    }
}

void ElecLockSerialManager::slotPosTimeout()
{
    ElecLockPosInfo info;
    if (getElecLockPosInfo(info)) {
        if(m_realPosInfoCb != nullptr) {
            m_realPosInfoCb(info, m_realPosInfoParam);
        }
    }
}

bool ElecLockSerialManager::getElecLockWorkInfo(ElecLockWorkInfo& info)
{
    unsigned char cmd[8] = {0x01, 0x03, 0xEC, 0xD1, 0x00, 0x1E, 0xA1, 0x6B};
    int len = cmd[5] * 2 + 5;
/*
    QByteArray recv = m_serial->sendWaitForResp(cmd, 8, len);
    if(recv.size() <= 0) {
        qDebug() << "长度 ERROR";
        return false;
    }
    unsigned char* cache = (unsigned char*)recv.data();
*/


    QByteArray header;
    header.append((char)0x01);
    header.append((char)0x03);
    QByteArray recv = m_serial->sendWaitForRespEx(cmd, 8, header, len);
    if(recv.size() < len) {
        qDebug() << "Get Work Info 长度 ERROR" << recv.size();
        return false;
    }

    ushort rtCrc = (recv.at(len -2) & 0xFF) | ((recv.at(len - 1) & 0xFF) << 8);
    unsigned char* cache = (unsigned char*)recv.left(len).data();

    ushort crc = crc16_modbus(cache, len - 2);
    qDebug("WORK INFO 返回CRC：%X，计算CRC：%X", rtCrc, crc);
    if (rtCrc != crc) {
        qDebug() << "CRC ERROR";
        return false;
    }


    info.circleNum = (float)((cache[5] << 8) | cache[6]) / 100;
    info.mnmMax = (cache[9] << 8) | cache[10];
    info.workTime = (cache[11] << 8) | cache[12];
    int result = (cache[13] << 8) | cache[14];
    if(result == 0x4aaa) {
        info.workResult = -1;
    } else if(result == 0x4bbb) {
        info.workResult = 1;
    } else if(result == 0x4ccc) {
        info.workResult = 0;
    }
    info.errorCode = cache[30];
    info.direction = cache[34];
    info.workState = cache[38] & 0x01;

    info.realMnm = (cache[53] << 8) | cache[54];
    info.realRpm = (cache[55] << 8) | cache[56];
    info.realControlRpm = (cache[57] << 8) | cache[58];
    info.realCircleNum = (float)((cache[59] << 8) | cache[60]) / 100;
    info.realWorkTime = (cache[61] << 8) | cache[62];
    info.vol = (cache[63] << 8) | cache[64];

    m_cache.remove(0, len+5);
//    qDebug() << result << info.workState << info.workResult << info.errorCode << info.direction;
    return true;
}

bool ElecLockSerialManager::getElecLockPosInfo(ElecLockPosInfo &info)
{
    unsigned char cmd[8] = {0x01, 0x03, 0xEC, 0xD1, 0x00, 0x05, 0xE1, 0x60};
    int len = cmd[5] * 2 + 5;

    QByteArray recv = m_posSerial->sendWaitForResp(cmd, 8, len);
    if(recv.size() < 0) {
        qDebug() << "Get Pos 长度 ERROR";
        return false;
    }
    unsigned char* cache = (unsigned char*)recv.data();

    int x = cache[7] << 24;
    x |= cache[8] << 16;
    x |= cache[5] << 8;
    x |= cache[6];

    int y = cache[11] << 24;
    y |= cache[12] << 16;
    y |= cache[9] << 8;
    y |= cache[10];
    qDebug() << "X: " << x << " Y: " << y;

    if(cache[3] == 0x55 && cache[4] == 0x55) {
        info.flag = false;
    } else {
        info.flag = true;
    }
    info.x = x;
    info.y = y;
    return true;
}

void ElecLockSerialManager::setEnable(const bool flag)
{
    m_enable = flag;
    writeValue(3927, flag ? 1 : 0);
}

bool ElecLockSerialManager::getEnable()
{
    return m_enable;
}

void ElecLockSerialManager::save()
{
    writeValue(61501, 0x1aaa);
}



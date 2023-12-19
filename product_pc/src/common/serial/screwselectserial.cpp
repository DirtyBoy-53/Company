#include "screwselectserial.h"

ScrewSelectSerial::ScrewSelectSerial()
{
    m_serial = new CSerialPort();
    m_timer = new QTimer();
    connect(m_timer,&QTimer::timeout, this, &ScrewSelectSerial::slotTimeout);
}

ScrewSelectSerial::~ScrewSelectSerial()
{

}

int ScrewSelectSerial::judgetData(unsigned char *data, int length)
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

int ScrewSelectSerial::getScrewCheckState()
{
    unsigned char cmd[8] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x08, 0x79, 0xCC};
    QByteArray recv = m_serial->sendWaitForResp(cmd, 8, 6);
    if(recv.size() == 0) {
        return false;
    }

    unsigned char* cache = (unsigned char*)recv.data();
    int state = cache[3];
    return state;
}

int ScrewSelectSerial::getScrewWorkState()
{
    unsigned char cmd[8] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x08, 0x3D, 0xCC};
    QByteArray recv = m_serial->sendWaitForResp(cmd, 8, 6);
    if(recv.size() == 0) {
        return false;
    }

    unsigned char* cache = (unsigned char*)recv.data();
    int state = cache[3];
    return state;
}

bool ScrewSelectSerial::openSerial(const QString &com)
{
    SerialParam param;
    param.baudRate    = QSerialPort::Baud9600;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;
    return m_serial->openSerial(com, param, true);
}

void ScrewSelectSerial::closeSerial()
{
    m_serial->closeSerial();
    QThread::msleep(1000);
}

void ScrewSelectSerial::registerInfoCb(ScrewSelectFunc cb, void *param)
{
    m_infoCb = cb;
    m_infoParam = param;
    if(m_timer->isActive()) {
        m_timer->stop();
    }
    m_timer->start(300);
}

void ScrewSelectSerial::unregisterInfoCb()
{
    m_timer->stop();
}

bool ScrewSelectSerial::setPosClamped(const int pos, bool isClamp)
{
    unsigned char cmd[8] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[3] = pos & 0xff;
    cmd[4] = isClamp ? 0x00 :  0xff;
    QByteArray recv = m_serial->sendWaitForResp(cmd, 8, 8);
    if(recv.size() == 0) {
        return false;
    }
    return true;
}

void ScrewSelectSerial::slotTimeout()
{
    ScrewSelectInfo info;
    info.posCheckState = getScrewCheckState();
    info.posWorkState = getScrewWorkState();
    if(m_infoCb != nullptr) {
        m_infoCb(info, m_infoParam);
    }
}

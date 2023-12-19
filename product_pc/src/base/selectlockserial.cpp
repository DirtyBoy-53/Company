#include "selectlockserial.h"
#include <QDebug>

SelectLockSerial::SelectLockSerial()
{
    m_seleckSerial = new CSerialPort();
}


int SelectLockSerial::openSelectLockSerial(const QString& com)
{
    closeSelectLockSerial();

    SerialParam param;
    param.baudRate    = QSerialPort::Baud9600;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;

    bool ret = m_seleckSerial->openSerial(com, param, true);
    qDebug() << "open select serial result " << ret;
    return ret;
}

void SelectLockSerial::closeSelectLockSerial()
{
    m_seleckSerial->closeSerial();
}

int SelectLockSerial::checkLock()
{
    unsigned char cmd[8] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x08, 0x79, 0xCC};
    QByteArray recv = m_seleckSerial->sendWaitForResp(cmd, 8, 6).data();
    unsigned char* recvData = (unsigned char*)recv.data();
    qInfo()<<"checkLock ============ "<<QString(recv.toHex());
    if(recvData[0] == 0x01 && recvData[1] == 0x02) {
        qDebug() << "select lock is " << recvData[3];
        return recvData[3];
    } else {
        return 0;
    }

}

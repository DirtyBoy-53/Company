#include "ioserialmanager.h"
#include <QDebug>

IoSerialManager::IoSerialManager()
{
    m_serial = new CSerialPort();
}

IoSerialManager::~IoSerialManager()
{

}

bool IoSerialManager::openSerial(const QString &port)
{
    SerialParam param;
    param.baudRate    = QSerialPort::Baud9600;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;

    connect(m_serial, &CSerialPort::signalReadSerialData, this, &IoSerialManager::slotRecvSerialData);

    return m_serial->openSerial(port, param, false);
}

void IoSerialManager::closeSerial()
{
    m_serial->closeSerial();
    m_cache.clear();
}

void IoSerialManager::getIoDirection()
{
    unsigned char cmd[8] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x02, 0x65, 0xCB};
    m_serial->sendCommand(cmd, 8);
}

void IoSerialManager::slotOperate(int code, bool status)
{
    QString cmd;
    if (code == OID_Out1) { //
        if (status) {
            cmd = "OUT1_ON";
        } else {
            cmd = "OUT1_OFF";
        }
    } else if (code == OID_Out4) {
        if (status) {
            cmd = "OUT4_ON";
        } else {
            cmd = "OUT4_OFF";
        }
    } else if (code == OID_GetStatusIN1) {
        cmd = "IN1_STATUS";
    } else if (code == OID_GetStatusIN2) {
        cmd = "IN2_STATUS";
    } else if (code == OID_GetStatusIN3) {
        cmd = "IN3_STATUS";
    } else if (code == OID_GetStatusIN4) {
        cmd = "IN4_STATUS";
    } else if (code == OID_GetStatusOUT1) {
        cmd = "OUT1_STATUS";
    } else if (code == OID_GetStatusOUT4) {
        cmd = "OUT4_STATUS";
    }
    m_serial->readSerial(200);// 多余的部分丢弃
    m_serial->sendCommand((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
    QThread::msleep(200);
}

void IoSerialManager::slotRecvSerialData(QByteArray data)
{
    m_cache.append(data);
//    if(m_comType == 1) {
        if(m_cache.size() < 3) return;

        unsigned char* recvData = (unsigned char*)m_cache.data();
        if(recvData[0] == 0x55 && recvData[1] == 0xAA) {
            if(recvData[2] == 0x01) {
                qDebug() << "state close --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_CLOSE);
            } else if(recvData[2] == 0x00) {// 放了产品开盖板
                qDebug() << "state open --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_OPEN);
            } else if(recvData[2] == 0x02) { // 没放产品盖盖板
                qDebug() << "state no produce --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_NO_PRODUCE);
            }  else if(recvData[2] == 0x03) {
                qDebug() << "state no produce --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_FRONT);
            }  else if(recvData[2] == 0x04) {
                qDebug() << "state no produce --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_BACK);
            } else if(recvData[2] == 0x10) {
                qDebug() << "state left --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_LEFT);
            } else if(recvData[2] == 0x11) {
                qDebug() << "state right --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_RIGHT);
            } else if(recvData[2] == 0xFF) {
                qDebug() << "state error --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_ERROR);
            }else if(recvData[2] == 0x08) {
                qDebug() << "state error --------- ";
                emit signalUpdateIoState(IO_SERIAL_STATE_PRODUCT_ON);
            }
        }
        m_cache.remove(0, 4);
//    } else if(m_comType == 2) {
//        if(m_cache.size() < 9) return;

//        unsigned char* recvData = (unsigned char*)m_cache.data();
//        if(recvData[0] == 0x01 && recvData[1] == 0x03) {
//            if(recvData[4] == 0x01) {
//                emit signalUpdateIoState(IO_SERIAL_STATE_RIGHT);
//            } else if(recvData[6] == 0x01) {
//                emit signalUpdateIoState(IO_SERIAL_STATE_LEFT);
//            }
//        }
//        m_cache.remove(0, 9);
//    }
}

#include "cserialport.h"
#include <QDebug>
#include <thread>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

CSerialPort::CSerialPort()
{

}

CSerialPort::~CSerialPort()
{
    if(m_serialPort != nullptr && m_serialPort->isOpen()) {
        m_serialPort->close();
        m_serialPort->clear();
        m_serialPort = nullptr;
    }
}

bool CSerialPort::openSerial(const QString &serialName, const SerialParam &param, bool isSync)
{
    if(m_serialPort == nullptr) {
        m_serialPort = new QSerialPort();
    }

    if(m_serialPort->isOpen()) {
        return true;
    }

    m_serialPort->setPortName(serialName);
    if(m_serialPort->open(QIODevice::ReadWrite)) {
        //设置串口参数
        m_serialPort->setBaudRate(param.baudRate,QSerialPort::AllDirections);//设置波特率和读写方向
        m_serialPort->setDataBits(param.dataBits);//数据位位8位
        m_serialPort->setFlowControl(param.flowControl);//无流控制
        m_serialPort->setParity(param.parity);//偶校验位
        m_serialPort->setStopBits(param.stopBits);//一位停止位
        if(!isSync) {
            connect(m_serialPort,&QSerialPort::readyRead,this,&CSerialPort::onReadData);
        }
        emit signalOpenResult(true);
        return true;
    } else {
        emit signalOpenResult(false);
        return false;
    }
}

void CSerialPort::closeSerial()
{
    if(m_serialPort != nullptr && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

int CSerialPort::sendCommand(unsigned char* data, const int len)
{
    if(!m_serialPort->isOpen()) {
        return -1;
    }
    QByteArray log((const char*)data, len);
    qDebug() << "send serial data" << QString(log.toHex()) << len;
    if(m_serialPort != nullptr && m_serialPort->isOpen()) {
        m_serialPort->write((const char*)data, len);
        m_serialPort->flush();
        return 0;
    } else {
        return -1;
    }
}

int CSerialPort::sendCommandWait(unsigned char *data, const int len)
{
    QByteArray log((const char*)data, len);
    qDebug() << "send serial data" << QString(log.toHex()) << len;
    if(m_serialPort != nullptr && m_serialPort->isOpen()) {
        m_serialPort->write((const char*)data, len);
        m_serialPort->flush();
        QElapsedTimer timer;
        timer.start();
        int timeout = 6000;
        int wLen = 0;
        while (timer.elapsed() < timeout) {
            QCoreApplication::processEvents();
            wLen += m_serialPort->writeChannelCount();
            if(wLen >= len) {
                break;
            }
        }
//        qInfo()<<"send "<<wLen<<" take "<<timer.elapsed();
        return 0;
    } else {
        return -1;
    }
    return -1;
}

QByteArray CSerialPort::readSerial(const int timeout)
{
    QByteArray resp;
    if(m_serialPort->isOpen()) {
        m_serialPort->waitForReadyRead(timeout);
        resp = m_serialPort->readAll();
    }
    return resp;
}

QByteArray CSerialPort::readSerialHold(const int timeout, int needLen)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray cache;
    if(!m_serialPort->isOpen()) {
        qDebug()<<"串口未打开";
        return cache;
    }
    int iCount = 0;
    while (timer.elapsed() < timeout) {
//        QCoreApplication::processEvents();
        m_serialPort->waitForReadyRead(timeout);
        cache.append(m_serialPort->readAll());
        if (needLen != -1) {
            if (cache.size() >= needLen) {
                qDebug()<<"收到数据:"<<cache;
                break;
            }
        }
    }
    if(cache.size() <= 0){
        qDebug()<<"数据异常:"<<cache;
    }
    return cache;
}

bool CSerialPort::findSerial(QByteArray find, int timeout)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray cache;
    if(!m_serialPort->isOpen()) {
        return false;
    }

    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        cache.append(m_serialPort->readAll());
        if (cache.contains(find)) return true;
    }
    return false;
}

QByteArray CSerialPort::read(int readLen)
{
    QByteArray cache;
    if(!m_serialPort->isOpen()) {
        return cache;
    }
    while(cache.size() < readLen) {
        if(m_serialPort->bytesAvailable() > 0) {
            cache.append(m_serialPort->read(readLen - cache.size()));
        } else if(m_serialPort->waitForReadyRead(1000)) {
            cache.append(m_serialPort->read(readLen - cache.size()));
        }
    }
    return cache;
}

QByteArray CSerialPort::sendWaitForResp(unsigned char *data, const int len, const int respLen)
{
    if(!m_serialPort->isOpen()) return "";
    sendCommand(data, len);

//    QByteArray log((const char*)data, len);
//    qDebug() << "send serial data" << QString(log.toHex()) << len;
    QByteArray cache;
    while(cache.size() < respLen) {
        if(m_serialPort->waitForReadyRead(3000)) {
            cache.append(m_serialPort->read(respLen - cache.size()));
        } else {
            return QByteArray();
        }
    }

//    qDebug() << "read: " << QString(cache.toHex());
    return cache;
}

QByteArray CSerialPort::sendWaitForRespEx(unsigned char *data, const int len, QByteArray header, const int respLen)
{
    if(!m_serialPort->isOpen()) return "";
    sendCommand(data, len);

//    QByteArray log((const char*)data, len);
    //qDebug() << "send serial data" << QString(log.toHex()) << len;
    bool hasHeader = false;
    QByteArray cache;
    QByteArray buffer;
    QElapsedTimer timer;
    timer.start();
    int headerIndex = -1;
    while((cache.size() < respLen)) {
        // 3. 加入机制
        QCoreApplication::processEvents();
        if(m_serialPort->waitForReadyRead(300)) {
            QByteArray ba = m_serialPort->read(respLen);
            qDebug() << "### >>> read: " << QString(ba.toHex());
            buffer.append(ba);
            if (!hasHeader) {
                int index = buffer.indexOf(header);
                if (index != -1) {
                    hasHeader = true;
                    headerIndex = index;
                }
            }

            if (((buffer.size() - headerIndex) >= respLen) && hasHeader) {
                cache = QByteArray(buffer.data() + headerIndex, buffer.size() - headerIndex);
                break;
            }
        } else {
            return QByteArray();
        }
    }

    // qDebug() << "read: " << QString(cache.toHex());
    return cache;
}

void CSerialPort::onReadData()
{
    QByteArray data = m_serialPort->readAll();
    emit signalReadSerialData(data);
}

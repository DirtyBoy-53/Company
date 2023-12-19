#include "ctcpsocket.h"

CTcpSocket::CTcpSocket(QObject* parent) :
    QTcpSocket(parent)
{
    connect(this, &QTcpSocket::connected, this, &CTcpSocket::onConnect);
    connect(this, &QTcpSocket::disconnected, this, &CTcpSocket::onDisConnect);
}

void CTcpSocket::onInit(QString ip, int port)
{
    this->connectToHost(ip, port);
}

void CTcpSocket::onDeInit()
{
    while(this->bytesAvailable()) {
        this->readAll();
    }

    this->disconnectFromHost();
    this->abort();
}

QByteArray CTcpSocket::sendWaitForResp(const char *data, const int len, const int respLen)
{
    if(!this->isOpen()) return "";
    write((const char*)data, len);

    QByteArray log((const char*)data, len);
    qDebug() << "send serial data" << QString(log.toHex()) << len;
    QByteArray cache;
    while(cache.size() < respLen) {
        if(this->waitForReadyRead(3000)) {
            cache.append(this->readAll());
        } else {
            return QByteArray();
        }
    }
    return cache;
}

void CTcpSocket::onConnect()
{
    connect(this, &QTcpSocket::readyRead, this, &CTcpSocket::onRead);
    emit this->signalConnected();
}

void CTcpSocket::onRead()
{
    if(this->bytesAvailable() <= 0) {
        return;
    }

    QByteArray buffer = this->readAll();
    emit this->signalReadData(buffer);
}

void CTcpSocket::onDisConnect()
{
    abort();
    emit this->signalDisconnected();
}

void CTcpSocket::onSendData(const QByteArray& buffer)
{
    if(isValid()) {
        write(buffer);
        //        flush();
        //        waitForBytesWritten();
    }
}

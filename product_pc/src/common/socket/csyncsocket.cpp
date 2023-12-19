#include "csyncsocket.h"

CSyncSocket::CSyncSocket(QObject* parent) :
    QTcpSocket(parent)
{

}

bool CSyncSocket::connect(QString ip, int port)
{
    connectToHost(ip, port);
    return waitForConnected(10 * 1000);
}

bool CSyncSocket::disConnect()
{
    close();
    disconnectFromHost();
    return waitForDisconnected();
}

QByteArray CSyncSocket::sendWaitForResp(const char *data, const int len)
{
    write(data, len);
    waitForBytesWritten();
    waitForReadyRead(10 * 1000);
    return readAll();
}

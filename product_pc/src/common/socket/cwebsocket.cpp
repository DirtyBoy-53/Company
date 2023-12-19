#include "cwebsocket.h"

CWebSocket::CWebSocket()
{

}

void CWebSocket::slotConnectToUrl(const QString &url)
{
    open(QUrl(url));
}

void CWebSocket::slotStopConnect()
{
    close();
}

void CWebSocket::slotSendTextMessage(const QString &data)
{
    sendTextMessage(data);
}

void CWebSocket::slotSendBinaryMessage(const QByteArray &data)
{
    sendBinaryMessage(data);
}

void CWebSocket::onDeInit()
{
    qDebug() << "websocket onDeInit ";
    this->close();
    this->abort();
}

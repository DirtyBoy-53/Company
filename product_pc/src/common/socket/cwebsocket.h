#ifndef CWEBSOCKET_H
#define CWEBSOCKET_H

#include <QObject>
#include <QWebSocket>
#include <QThread>
#include <comdll.h>

class Q_API_FUNCTION CWebSocket : public QWebSocket
{
    Q_OBJECT
public:
    CWebSocket();

public slots:
    void slotConnectToUrl(const QString &url);
    void slotStopConnect();
    void slotSendTextMessage(const QString& data);
    void slotSendBinaryMessage(const QByteArray& data);
    void onDeInit();
};

#endif // CWEBSOCKET_H

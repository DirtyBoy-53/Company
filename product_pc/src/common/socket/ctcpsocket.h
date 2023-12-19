#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include <comdll.h>

class Q_API_FUNCTION CTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit CTcpSocket( QObject* parent=0);

signals:
    void signalReadData(QByteArray buffer);
    void signalDisconnected();
    void signalConnected();

public slots:
    void onRead();
    void onConnect();
    void onDisConnect();
    void onSendData(const QByteArray& buffer);
    void onInit(QString ip, int port);
    void onDeInit();
    QByteArray sendWaitForResp(const char *data, const int len, const int respLen);
};

#endif // CTCPSOCKET_H

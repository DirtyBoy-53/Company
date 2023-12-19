#ifndef CSYNCSOCKET_H
#define CSYNCSOCKET_H

#include <QTcpSocket>

class CSyncSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit CSyncSocket(QObject* parent=0);
    bool connect(QString ip, int port);
    bool disConnect();
    QByteArray sendWaitForResp(const char *data, const int len);

};

#endif // CSYNCSOCKET_H

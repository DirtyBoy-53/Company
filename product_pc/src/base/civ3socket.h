#ifndef CIV3SOCKET_H
#define CIV3SOCKET_H

#include <QObject>
#include <QTcpSocket>

class CIv3Socket : public QObject
{
    Q_OBJECT
public:
    explicit CIv3Socket(QObject *parent = nullptr);
    ~CIv3Socket();
    bool init(QString ip);
    bool setProjectCode(QString code);
    QString productCode();
signals:
private slots:

private:
    QTcpSocket* m_iv3SocketWindow;
    QTcpSocket* m_iv3Socket;

};

#endif // CIV3SOCKET_H

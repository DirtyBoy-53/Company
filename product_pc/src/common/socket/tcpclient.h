#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <iostream>
#include <string>
#include <map>
#include <comdll.h>
#include <QJsonObject>

class TcpClient
{
public:
    TcpClient();
    void init(QString ip, QString port);
    bool sendCommand(QString command, QString &sOut);
    bool sendCommand(int id, int action, QJsonObject json, QString &sOut);
    bool sendCommand(int id, int action, QString &sOut);
private:

    bool getData(QString sData, QString & sOut);

    QString m_ip;
    QString m_port;
};

#endif // TCPCLIENT_H

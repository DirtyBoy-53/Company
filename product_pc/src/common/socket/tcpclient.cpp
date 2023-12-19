#include "tcpclient.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/Timespan.h"
#include <QByteArray>
#include <QJsonDocument>

using Poco::Net::Socket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::NetException;
using Poco::Net::ConnectionRefusedException;
using Poco::Net::InvalidSocketException;
using Poco::Timespan;
using Poco::TimeoutException;
using Poco::IOException;

const int RECV_BUF_SIZE = 1024*1024;
const int SEND_BUF_SIZE = 1024*1024;

const QString g_dataHead = "!head!";
const QString g_dataEnd = "!end!";

TcpClient::TcpClient()
{

}

void TcpClient::init(QString ip, QString port)
{
    m_ip = ip;
    m_port = port;
}

bool TcpClient::sendCommand(QString command, QString &sOut)
{
    return getData(command, sOut);
}

bool TcpClient::sendCommand(int id, int action, QJsonObject json, QString &sOut)
{
    QJsonObject json1;
    json1.insert("id", id);
    json1.insert("action", action);
    json1.insert("content", QJsonValue(json));
    return getData(QString(QJsonDocument(json1).toJson(QJsonDocument::Compact)).simplified(),
                   sOut);
}

bool TcpClient::sendCommand(int id, int action, QString &sOut)
{
    QJsonObject json1;
    json1.insert("id", id);
    json1.insert("action", action);
    return getData(QString(QJsonDocument(json1).toJson(QJsonDocument::Compact)).simplified(),
                   sOut);
}

bool TcpClient::getData(QString sData, QString &sOut)
{
    SocketAddress socket_addr(m_ip.toStdString().data(), m_port.toInt());
    StreamSocket stream;
    QString sSend = g_dataHead + sData + g_dataEnd;
    Timespan timeout(2000000);
    qInfo("connect %s %s", m_ip.toStdString().data(), m_port.toStdString().data());
    try
    {
        stream.connect(socket_addr,timeout);
    }
    catch (ConnectionRefusedException&)
    {
        qInfo("connect refuse");
        return false;
    }
    catch (NetException&)
    {
        qInfo("net exception");
        return false;
    }
    catch (TimeoutException&)
    {
        qInfo("connect time out");
        return false;
    }
    Timespan timeout3(5000000);
    stream.setReceiveTimeout(timeout3); //retn void
    Timespan timeout4(5000000);
    stream.setSendTimeout(timeout4); //retn void
    stream.setReceiveBufferSize(RECV_BUF_SIZE); //retn void
    stream.setSendBufferSize(SEND_BUF_SIZE); //retn void
    stream.setNoDelay(true);
    int sLen = 0;
    try
    {
        qInfo("send %s", sSend.toStdString().data());
        sLen = stream.sendBytes(sSend.toStdString().data(), sSend.size()); //block
    }
    catch (TimeoutException&)
    {
        qInfo("send time out");
    }
    catch (InvalidSocketException&)
    {
        qInfo("invalid socket exception");
    }
    catch (IOException&)
    {
        qInfo("write io exception");
    }
    if(sLen != sSend.size()) {
        qInfo("send error....");
        return false;
    }
    uint8_t buffer[1024 * 10];
    QString arrCache;
    int rLen = 0;
    int iRecvOk = 0;
    int iWaitCount = 2;
    while(iWaitCount > 0)
    {
        iWaitCount --;
        try
        {
//            if(stream.available())
            {
                rLen=0;
                memset(buffer,0,sizeof(buffer));
                rLen = stream.receiveBytes(buffer,sizeof(buffer)); //block
                arrCache.append(QByteArray::fromRawData((char *)buffer, rLen));
            }
            qInfo("recv len %d %s", rLen, arrCache.toStdString().data());
        }
        catch (TimeoutException&)
        {
            qInfo("recv time out");
        }
        catch (InvalidSocketException&)
        {
            qInfo("invalid socket exception");
        }
        if(arrCache.contains(g_dataHead) && arrCache.contains(g_dataEnd)) {
            iRecvOk = 1;
            qInfo("recv %s", arrCache.toStdString().data());
            sOut = arrCache.mid(g_dataHead.size(), arrCache.size() - g_dataHead.size() - g_dataEnd.size());
            break;
        }

    }
    try
    {
        stream.close();
    }
    catch (IOException&)
    {
        qInfo("close io exception");
    }
    return iRecvOk ? true : false;
}

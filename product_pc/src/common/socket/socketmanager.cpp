#include "socketmanager.h"
#include <QDateTime>
#include <QDataStream>
#include <QThread>

const QString g_dataHead = "!head!";
const QString g_dataEnd = "!end!";


SocketManager::SocketManager()
{
    m_socket = new CTcpSocket;
    m_videoSocket = new CTcpSocket;
    m_updateSocket = new CTcpSocket;

    QThread* threadVideo = new QThread();
    QThread* threadCommand = new QThread();
    QThread* threadUpdate = new QThread();

    connect(this, &SocketManager::signalInitCommand, m_socket, &CTcpSocket::onInit);
    connect(this, &SocketManager::signalInitVideo, m_videoSocket, &CTcpSocket::onInit);
    connect(this, &SocketManager::signalInitUpdate, m_updateSocket, &CTcpSocket::onInit);
    connect(this, &SocketManager::signalDeInit, m_socket, &CTcpSocket::onDeInit);
    connect(this, &SocketManager::signalDeInit, m_videoSocket, &CTcpSocket::onDeInit);
    connect(this, &SocketManager::signalDeInit, m_updateSocket, &CTcpSocket::onDeInit);


    connect(this, &SocketManager::signalSendToCommand, m_socket, &CTcpSocket::onSendData);
    connect(this, &SocketManager::signalSendToUpdate, m_updateSocket, &CTcpSocket::onSendData);

    connect(m_videoSocket, &CTcpSocket::signalReadData, this, &SocketManager::onRecvDataVideo);
    connect(m_socket, &CTcpSocket::signalConnected, this, &SocketManager::showConnect);
    connect(m_socket, &CTcpSocket::signalDisconnected, this, &SocketManager::disConnect);
    connect(m_socket, &CTcpSocket::signalReadData, this, &SocketManager::onRecvData);

    m_socket->moveToThread(threadCommand);
    m_videoSocket->moveToThread(threadVideo);
    m_updateSocket->moveToThread(threadUpdate);

    threadCommand->start();
    threadVideo->start();
    threadUpdate->start();
}

SocketManager::~SocketManager()
{
    deInit();
}

void SocketManager::setFrameLen(int len)
{
    m_videoFrameLen = len;
}

void SocketManager::init(QString ip)
{
    if(ip != "") {
        qInfo() << "init ip is " << ip;
        emit signalInitCommand(ip, 6001);
        emit signalInitVideo(ip, 6000);
        emit signalInitUpdate(ip, 6002);
    } else {
        emit signalInitCommand("192.168.1.10", 6001);
        emit signalInitVideo("192.168.1.10", 6000);
        emit signalInitUpdate("192.168.1.10", 6002);
    }
}

void SocketManager::deInit()
{
    emit signalDeInit();
}

void SocketManager::disConnect()
{
    qDebug() << "disconnect";
    m_isConnected = false;
    emit signalDisConnect();

    if(!m_cache.isEmpty()) {
        m_cache.clear();
    }
}

void SocketManager::showConnect()
{
    qDebug() << "showConnect";
    m_isConnected = true;
    emit signalConnectedOk();
}

void SocketManager::sendUpdateData(QByteArray data)
{
    emit signalSendToUpdate(data);
}

void SocketManager::onRecvData(QByteArray data)
{
    m_commandCache.append(data);
    int pos = m_commandCache.indexOf(g_dataEnd);
    while(pos > 0) {
        QString command = m_commandCache.mid(g_dataHead.size(), pos-g_dataHead.size()).toLower();
        qDebug() << "recvMsg recv : " << command;
        m_jsonParser.recvMsg(command);
        m_commandCache.remove(0, pos + g_dataEnd.size());
        pos = m_commandCache.indexOf(g_dataEnd);

    }
}

void SocketManager::onRecvDataVideo(QByteArray data)
{
    QByteArray buffer = data;
    m_cache.append(buffer);
    if(m_videoFrameLen == 0) return;

    int timeLen = 13;
    if(m_cache.size() >= m_videoFrameLen) {
        QString timeT = m_cache.left(timeLen);
        emit signalRecvVideoFrame(m_cache.mid(timeLen, m_videoFrameLen-timeLen),timeT.toLongLong());
        m_cache.remove(0, m_videoFrameLen);
    }
}

bool SocketManager::sendCommand(QString command)
{
    QByteArray send;
    QDataStream out(&send,QIODevice::ReadWrite);
    out.writeRawData(g_dataHead.toStdString().c_str(), g_dataHead.size());
    out.writeRawData(command.toStdString().c_str(), command.length());
    out.writeRawData(g_dataEnd.toStdString().c_str(), g_dataEnd.size());
    emit signalSendToCommand(send);
    return m_isConnected;
}

bool SocketManager::sendCommand(int id, int action, QJsonObject json)
{
    QByteArray send;
    send.append(g_dataHead);
    send.append(m_jsonParser.splitMsg(id, action, json));
    send.append(g_dataEnd);
    qDebug() << "send msg : " << send;
    emit signalSendToCommand(send);
    return m_isConnected;
}

bool SocketManager::sendCommand(int id, int action)
{
    QByteArray send;
    send.append(g_dataHead);
    send.append(m_jsonParser.splitMsg(id, action));
    send.append(g_dataEnd);
    qDebug() << "send msg : " << send;
    emit signalSendToCommand(send);
    return m_isConnected;
}

bool SocketManager::getConnectedState()
{
    return m_isConnected;
}

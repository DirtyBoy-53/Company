#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QObject>
#include <QTimer>
#include <thread>

#include "ctcpsocket.h"
#include "jsonmanager.h"
#include "blockingqueue.h"
#include <comdll.h>

class Q_API_FUNCTION SocketManager : public QObject
{
    Q_OBJECT
public:
    explicit SocketManager();
    ~SocketManager();
    bool sendCommand(QString command);
    bool sendCommand(int id, int action, QJsonObject json);
    bool sendCommand(int id, int action);

    bool getConnectedState();
    void sendUpdateData(QByteArray data);

    void init(QString ip);
    void deInit();
    void changeVideoInfo(const int mode);
    void setFrameLen(int len);

signals:
    void signalRecvVideoFrame(QByteArray frameData, int timeT);
    void signalConnectedOk();
    void signalDisConnect();

    void signalSendToCommand(QByteArray data);
    void signalSendToUpdate(QByteArray data);
    void signalInitCommand(QString ip, int port);
    void signalInitVideo(QString ip, int port);
    void signalInitUpdate(QString ip, int port);
    void signalDeInit();


public slots:
    void disConnect();
    void showConnect();

    void onRecvData(QByteArray data);
    void onRecvDataVideo(QByteArray data);

private:
    bool m_isConnected = false;
    CTcpSocket* m_socket;
    CTcpSocket* m_videoSocket;
    CTcpSocket* m_updateSocket;

    QByteArray m_cache;
    QByteArray m_commandCache;
    JsonManager m_jsonParser;
    int m_videoFrameLen = 0;
};

#endif // SOCKETMANAGER_H

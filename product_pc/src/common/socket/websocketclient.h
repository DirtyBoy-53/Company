#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <string>
#include <QMutex>
#include <QMutexLocker>
#include <functional>
#include <atomic>

typedef std::function<void(char*, int)> WSFRAMECALL;
typedef std::function<void(int)> WSSTATUSCALL;

class WebSocketClient
{
public:
    WebSocketClient();

    void connect(std::string addr, int port);

    void disconnect();

    void setCall(WSFRAMECALL frameCall, WSSTATUSCALL statusCall);

private:
    void process();

    void connectWs();

private:
    QMutex m_mutex;
    std::atomic_int m_status ;
    std::string m_addr = "";
    int m_port = 0;
    char * g_cacheBUffer = nullptr;
    int g_cahceSize = 8 * 1024 * 1024;

    WSFRAMECALL m_frameCall = nullptr;
    WSSTATUSCALL m_statusCall = nullptr;
};

#endif // WEBSOCKETCLIENT_H

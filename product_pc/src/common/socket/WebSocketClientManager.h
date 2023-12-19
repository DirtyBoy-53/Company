#ifndef WEBSOCKETCLIENTMANAGER_H
#define WEBSOCKETCLIENTMANAGER_H

#include <QObject>
#include "CWebSocket.h"
#include <websocketclient.h>

struct FrameInfo_S{
    uint64_t uHeadFlag;//0xefefefef80808080
    int32_t uTotalLen;
    int64_t uTimeTm;
    int32_t uNvsLen;
    int32_t uY16Len;
    int32_t uYuvLen;
    int32_t uParamLen;
    int32_t uFormat; //1 yuv422 3 yuv16 5 y16+yuv 7 x16  0xffff 原始数据
    int32_t uUpgrade;//0 不升级  1 > 0 升级进度
    int32_t uScale;//1 正常  2 2倍 4 4倍
    int32_t vlFormat;//1 yuv422 y+u+v
    int32_t vlLen;
    int32_t vlWidth = 1920;
    int32_t vlHeight = 1280;
};

class WebSocketClientManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClientManager(QObject *parent = nullptr);
    ~WebSocketClientManager();
    static WebSocketClientManager* getInstance();

public:
    void init();
    void regFrameCall(WSFRAMECALL call);
    void connectToUrl(const QString &addr, int port);
    void stopConnect();

signals:
    void signalConnected();
    void signalDisconnected();
    void signalRecvVideoFrame(QByteArray frameData, FrameInfo_S info);

private slots:

private:
    void recvFrame(char * pData, int iLen);
    void statusChange(int status);
private:
    CWebSocket *m_webSocket = nullptr;
    uint64_t m_tRecvTm = 0;
    int m_tRecvCount = 0;
    QString m_url = "";
    int m_stop = 0;
    WebSocketClient * m_client = nullptr;
    WSFRAMECALL m_call = nullptr;
};

#endif // WEBSOCKETCLIENTMANAGER_H

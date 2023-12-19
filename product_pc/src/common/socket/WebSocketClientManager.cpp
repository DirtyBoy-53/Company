#include "WebSocketClientManager.h"
#include <QDebug>
#include <QThread>
#include "videoinfo.h"
#include <thread>
#include <websocketclient.h>


WebSocketClientManager::WebSocketClientManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FrameInfo_S>("FrameInfo_S");
    m_client = new WebSocketClient();
    WSFRAMECALL funFrame = std::bind(&WebSocketClientManager::recvFrame, this, std::placeholders::_1, std::placeholders::_2);
    WSSTATUSCALL funStatus =
            std::bind(&WebSocketClientManager::statusChange, this, std::placeholders::_1);
    m_client->setCall(funFrame, funStatus);
}

WebSocketClientManager::~WebSocketClientManager()
{
}

WebSocketClientManager* WebSocketClientManager::getInstance()
{
    static WebSocketClientManager instance;
    return &instance;
}

void WebSocketClientManager::init()
{
}

void WebSocketClientManager::regFrameCall(WSFRAMECALL call)
{
    m_call = call;
}

void WebSocketClientManager::connectToUrl(const QString &addr, int port)
{
    m_client->connect(addr.toStdString(), port);
}


void WebSocketClientManager::recvFrame(char *pData, int iLen)
{
    if(m_call) {
        m_call(pData, iLen);
    }
    FrameInfo_S frameInfo;
    memcpy(&frameInfo, pData, sizeof(frameInfo));
//    qDebug() << frameInfo.uFormat << frameInfo.uHeadFlag << frameInfo.uNvsLen << frameInfo.uParamLen
//             << frameInfo.uTimeTm << frameInfo.uTotalLen << frameInfo.uY16Len << frameInfo.uYuvLen;

    emit signalRecvVideoFrame(QByteArray::fromRawData(pData + 128, iLen - 128), frameInfo);
}

void WebSocketClientManager::statusChange(int status)
{
    if(status == 1) {
        emit signalConnected();
    }else if(status == 2){
        emit signalDisconnected();
    }
}


void WebSocketClientManager::stopConnect()
{
    m_client->disconnect();
}



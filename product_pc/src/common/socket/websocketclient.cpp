#include "websocketclient.h"

#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Thread.h"
#include "Poco/Buffer.h"
#include <thread>
#include <QDebug>

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::SocketStream;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::NetException;


WebSocketClient::WebSocketClient()
{
    g_cahceSize   = 8 * 1024 * 1024;
    g_cacheBUffer = (char *) malloc(g_cahceSize);
    m_status = 0;
    new std::thread(&WebSocketClient::process, this);
}

void WebSocketClient::connect(std::string addr, int port)
{
    QMutexLocker locker(&m_mutex);
    m_addr = addr;
    m_port = port;
    m_status = 1;
}

void WebSocketClient::disconnect()
{
    m_status = 0;
}

void WebSocketClient::setCall(WSFRAMECALL frameCall, WSSTATUSCALL statusCall)
{
    m_frameCall = frameCall;
    m_statusCall = statusCall;
}

void WebSocketClient::process()
{
    int status = 0;
    while(1) {
        status = m_status;
        if(status) {
            connectWs();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void WebSocketClient::connectWs()
{
    std::string addr;

    int port;
    {
        QMutexLocker locker(&m_mutex);
        addr = m_addr;
        port = m_port;
    }
    try
    {
        Poco::Net::ServerSocket ss(port);
        HTTPClientSession cs(addr, ss.address().port());
        HTTPRequest request(HTTPRequest::HTTP_GET, "/ws", HTTPRequest::HTTP_1_1);
        HTTPResponse response;
        WebSocket ws(cs, request, response);
        if(m_statusCall) {
            m_statusCall(1);
        }
        qInfo("connect %s:%d", addr.data(), port);
        int flags = 0;
        int rLen = 0;
        while(m_status == 1) {
            rLen = ws.receiveFrame(g_cacheBUffer, g_cahceSize, flags);
            if((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE) {
                qInfo("disconnect...ws");
                break;
            }
            if(rLen > 0 && m_frameCall) {
                m_frameCall(g_cacheBUffer, rLen);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    catch (WebSocketException& exc)
    {
        qInfo("WebSocketException %d ", exc.code());
    }catch (NetException& exc)
    {
        qInfo("NetException %d ", exc.code());
    }catch (...)
    {
        qInfo("Exception %d ");
    }
    if(m_statusCall) {
        m_statusCall(2);
    }
}

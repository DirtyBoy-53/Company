#ifndef HTTPMANAGER
#define HTTPMANAGER

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include <QObject>
#include <functional>

struct PcbCodeStruct {
    QString code1;
    QString code2;
};

typedef int(*getPcbaCodeFunc)(QList<PcbCodeStruct>& pcba, void* param);
typedef int(*getConnectState)(void* param);

typedef std::function<void(std::string sUri,
                           std::string &sIn,
                           std::string &sOut)> MSG_FUN_HANDLE;


class HttpManager : public Poco::Net::HTTPRequestHandler
{
public:
    HttpManager();
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);


    void Start(int port = 80);
    void Stop();
    void registerGetConnectState(getConnectState func, void* param);
    void registerSetPcbCode(getPcbaCodeFunc func, void* param);

    void setHandler(const MSG_FUN_HANDLE &newHandler);

private:
    Poco::Net::HTTPServer* m_server = nullptr;
    void MethodPost(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    static getPcbaCodeFunc m_getCodeCb;
    static void* m_param1;
    static getConnectState m_connectStateCb;
    static void* m_param2;
    MSG_FUN_HANDLE m_handler = nullptr;
};

#endif // HTTPMANAGER

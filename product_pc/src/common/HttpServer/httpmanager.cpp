#include "httpmanager.h"
#include <iostream>
#include <QFile>
#include <QDebug>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/URI.h"
#include <QDebug>

using namespace std;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPServer;


getPcbaCodeFunc HttpManager::m_getCodeCb = nullptr;
void* HttpManager::m_param1 = nullptr;
getConnectState HttpManager::m_connectStateCb = nullptr;
void* HttpManager::m_param2 = nullptr;
static MSG_FUN_HANDLE g_handler = nullptr;

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &)
    {
        return new HttpManager;
    }
};

HttpManager::HttpManager()
{

}

void HttpManager::Start(int port)
{
    if(m_server == nullptr) {
        m_server = new HTTPServer(new MyRequestHandlerFactory, ServerSocket(port), new HTTPServerParams);
    }

    if(m_server != nullptr) {
        m_server->start();
    }
}

void HttpManager::Stop()
{
    if(m_server != nullptr) {
        m_server->stop();
    }
}

void HttpManager::registerGetConnectState(getConnectState func, void *param)
{
    qDebug() << "registerGetConnectState";
    m_connectStateCb = func;
    m_param2 = param;
}

void HttpManager::registerSetPcbCode(getPcbaCodeFunc func, void *param)
{
    qDebug() << "registerSetPcbCode";
    m_getCodeCb = func;
    m_param1 = param;
}

void HttpManager::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
//    std::string uri;
//    Poco::URI::decode(request.getURI(), uri);
//    std::cout<<"recv uri "<<request.getURI()<<std::endl;
    qInfo("recv %s %s",request.getMethod().data(), request.getURI().data());
//    qInfo()<<"recv-- "<<QByteArray::fromStdString(request.getURI()).toHex();
//    qInfo()<<"recv -- "<<QByteArray::fromStdString(request.getMethod()).toHex();

    if(request.getKeepAlive()) {
        response.setKeepAlive(true);
    }
    if(request.getMethod() == "POST") {
        MethodPost(request, response);
    } else {
        response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        response.setContentType("text/html");
        ostream & out = response.send();

        out << "<font color='blue'>Hello World!</font>";
    }
}

void HttpManager::MethodPost(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    istream& in = request.stream();
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;
    parser.reset();
    result = parser.parse(in);
    std::string uri = request.getURI();
    qInfo()<<"-------------------";
    qInfo() << "http post "
            << QString::fromStdString(uri)
            << ", "
            <<QString::fromStdString(result.toString());
    if(g_handler != nullptr) {
        std::string sOut;
        g_handler(uri,
                  result.toString(),
                  sOut);
        ostream & out = response.send();
        out << sOut;
        return;
    }
    Poco::JSON::Object respJson;
    Poco::JSON::Object::Ptr pObj = result.extract<Poco::JSON::Object::Ptr>();

    int errorCode = 0;
    if(uri == "/api/user/available") {
        if(m_connectStateCb != nullptr) {
            errorCode = m_connectStateCb(m_param2);
        } else {
            errorCode = -99;
        }
        respJson.set("error_code", errorCode);
        respJson.set("msg", "备注");
    } else if(uri == "/api/user/report") {
        Poco::Dynamic::Var ret = pObj->get("items");
        if (ret.isArray()) {
            QList<PcbCodeStruct> list;
             Poco::JSON::Array::Ptr array = ret.extract<Poco::JSON::Array::Ptr>();
             cout << "array size is " << array->size() << endl;
             for(int i=0; i<array->size(); i++) {
                 Poco::JSON::Object::Ptr theObj = array->get(i).extract<Poco::JSON::Object::Ptr>();
                 Poco::Dynamic::Var data1 = theObj->get("data1");
                 Poco::Dynamic::Var data2 = theObj->get("data2");
                 PcbCodeStruct info;
                 info.code1 = QString::fromStdString(data1);
                 info.code2 = QString::fromStdString(data2);
                 list.append(info);
                 cout << "data is " <<data1.toString() << " " << data2.toString() << endl;
             }
             if(m_getCodeCb != nullptr) {
                 errorCode = m_getCodeCb(list, m_param1);
             } else {
                 errorCode = -99;
             }
        }
        respJson.set("error_code", errorCode);
    }

    std::ostringstream oss;
    respJson.stringify(oss);
    ostream & out = response.send();
    out << oss.str();
}

void HttpManager::setHandler(const MSG_FUN_HANDLE &newHandler)
{
    m_handler = newHandler;
    g_handler = newHandler;
}


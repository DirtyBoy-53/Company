#include "HttpClient.h"
#include <qobject.h>
#include <sstream>
#include <ostream>

#include <Poco/Net/WebSocket.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h>
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/URI.h"
#include "Poco/ASCIIEncoding.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/Latin1Encoding.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Timespan.h"
#include "Poco/FileStream.h"
#include "Poco/UnicodeConverter.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Net/MultipartWriter.h"
#include "poco/Net/MessageHeader.h"

using namespace Poco;
using Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPException;
using Poco::Exception;
using Poco::URI;
using Poco::Net::HTMLForm;
using Poco::Net::NetException;
using Poco::StreamCopier;
using Poco::ASCIIEncoding;
using Poco::UTF8Encoding;
using Poco::TextConverter;
using Poco::Latin1Encoding;
using Poco::NumberFormatter;
using Poco::Timespan;
using Poco::FileStream;
using Poco::UnicodeConverter;
using Poco::Net::StringPartSource;
using Poco::JSON::JSONException;
using Poco::Net::MultipartWriter;
using Poco::Net::MessageHeader;

using namespace std;

HttpClient::HttpClient()
{
    m_server = "";
    m_port = 80;
    //m_token = "";
}

HttpClient::HttpClient(string server, int port)
{
    m_server = server;
    m_port = port;
}


HttpClient::~HttpClient()
{

}

void HttpClient::init(const string server, const int port)
{
    m_server = server;
    m_port = port;
}

int HttpClient::post(const string url, const string &postJson, string & recvJson)
{
    try{
        std::stringstream strStream;
        strStream << m_port;
        std::string port = strStream.str();
        URI uri("http://" + m_server + ":" + port + url);
        HTTPClientSession session(uri.getHost(), uri.getPort());
        Timespan timespan(15, 0);
        session.setTimeout(timespan);
        HTTPRequest request(HTTPRequest::HTTP_POST, uri.getPath(), HTTPRequest::HTTP_1_1);
        request.setChunkedTransferEncoding(false);
        request.setContentType("application/json;charset=utf-8");
        request.setContentLength(postJson.length());
        session.sendRequest(request) << postJson;
        qInfo("post json %s", postJson.data());
        HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        session.flushRequest();
        const HTTPResponse::HTTPStatus &status = response.getStatus();
        if (HTTPResponse::HTTPStatus::HTTP_OK == status)
        {
            string recv;
            StreamCopier::copyToString(is, recv);
            recvJson = recv;
            return RET_OK;
        }
        return RET_ERR;
    }
    catch (TimeoutException& ex)
    {
        qInfo("post json %d", 1);
        return RET_ERR;
    }
    catch (NetException& ex)
    {
        qInfo("post json %s", ex.displayText().c_str());
        return RET_ERR;
    }
    catch(...){
        qInfo("post json %d", 3);
        return RET_ERR;
    }
    return RET_ERR;
}

int HttpClient::get(const string & url, string & recvString)
{
    try
    {
        std::stringstream strStream;
        strStream << m_port;
        std::string port = strStream.str();
        URI uri("http://" + m_server + ":" + port + url);
        HTTPClientSession session(uri.getHost(),uri.getPort());
        HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathAndQuery(), HTTPRequest::HTTP_1_1);
        session.sendRequest(request);
        HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        const HTTPResponse::HTTPStatus &status = response.getStatus();
        if (HTTPResponse::HTTPStatus::HTTP_OK == status)
        {
            StreamCopier::copyToString(is, recvString);
            return RET_OK;
        }
        return RET_ERR;
    }
    catch (TimeoutException& ex)
    {
        return RET_ERR;
    }
    catch (NetException& ex)
    {
        return RET_ERR;
    }
    catch (...) {
        return RET_ERR;
    }
    return RET_ERR;
}

int HttpClient::get(const std::string &url, std::map<std::string, std::string> &param, std::string &recvString)
{
    try
    {
        std::stringstream strStream;
        strStream << m_port;
        std::string port = strStream.str();
        URI uri("http://" + m_server + ":" + port + url);
        std::map<std::string, std::string>::iterator iter;
        iter = param.begin();
        while(iter != param.end()) {
            uri.addQueryParameter(iter->first,iter->second);
            iter++;
        }
        HTTPClientSession session(uri.getHost(),uri.getPort());
        qInfo("start request %s", uri.getPathAndQuery().data());
        HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathAndQuery(), HTTPRequest::HTTP_1_1);
        session.sendRequest(request);
        HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        const HTTPResponse::HTTPStatus &status = response.getStatus();
        if (HTTPResponse::HTTPStatus::HTTP_OK == status)
        {
            StreamCopier::copyToString(is, recvString);
            return RET_OK;
        }
        return RET_ERR;
    }
    catch (TimeoutException& ex)
    {
        qInfo("TimeoutException %s", ex.name());
        return RET_ERR;
    }
    catch (NetException& ex)
    {
        qInfo("NetException %s %d %s ", ex.name(), ex.code(),ex.message().data());
        return RET_ERR;
    }
    catch (...) {
        return RET_ERR;
    }
    return RET_ERR;
}

int HttpClient::upload(const string url, string fileName, uint8_t *pData, int iLen, string & recvJson)
{
    std::ostringstream ostr;
    string boundary = MultipartWriter::createBoundary();
    MultipartWriter w(ostr, boundary);
    MessageHeader h;
    h.set("Content-Disposition", "form-data; name=\"bucketName\"");
    w.nextPart(h);
    ostr<< "xuanyuan";
    h.clear();
    h.set("Content-Disposition", "form-data; name=\"file\"; filename=\""+fileName+"\"");
    h.add("Content-Type", "application/octet-stream");
    w.nextPart(h);
    ostr.write((char *)pData, iLen);
    w.close();
    try{
        std::stringstream strStream;
        strStream << m_port;
        std::string port = strStream.str();
        URI uri("http://" + m_server + ":" + port + url);
        HTTPClientSession session(uri.getHost(), uri.getPort());
        Timespan timespan(15, 0);
        session.setTimeout(timespan);
        HTTPRequest request(HTTPRequest::HTTP_POST, uri.getPath(), HTTPRequest::HTTP_1_1);
        request.setChunkedTransferEncoding(false);
        request.setContentType("multipart/form-data; boundary=" + boundary);
        request.setContentLength(ostr.str().size());

        session.sendRequest(request) .write(ostr.str().data(), ostr.str().size());
        HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        session.flushRequest();
        const HTTPResponse::HTTPStatus &status = response.getStatus();
        if (HTTPResponse::HTTPStatus::HTTP_OK == status)
        {
            string recv;
            StreamCopier::copyToString(is, recv);
            recvJson = recv;
            return RET_OK;
        }
        return RET_ERR;
    }
    catch (TimeoutException& ex)
    {
        return RET_ERR;
    }
    catch (NetException& ex)
    {
        return RET_ERR;
    }
    catch(...){
        return RET_ERR;
    }
    return RET_ERR;
}

int HttpClient::download(const std::string url, std::string sToken, uint8_t **pOutData, std::string sbuckName)
{
    try{
        std::stringstream strStream;
        strStream << m_port;
        std::string port = strStream.str();
        URI uri("http://" + m_server + ":" + port + url);
        uri.addQueryParameter("bucketName",sbuckName);
        uri.addQueryParameter("fileName",sToken);
        HTTPClientSession session(uri.getHost(), uri.getPort());
        Timespan timespan(15, 0);
        session.setTimeout(timespan);
        qInfo("get uri %s ", uri.getPathAndQuery().data());
        HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathAndQuery(), HTTPRequest::HTTP_1_1);
        request.setChunkedTransferEncoding(false);

        session.sendRequest(request);
        HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        session.flushRequest();
        const HTTPResponse::HTTPStatus &status = response.getStatus();
        if (HTTPResponse::HTTPStatus::HTTP_OK == status && !response.hasContentLength())
        {
            std::ostringstream ostrstream;
            uint8_t * pData = (uint8_t *) malloc(1024 * 1024);
            while(!is.eof()) {
                int rLen = is.readsome((char *)pData, 1024 * 1024);
                ostrstream.write((char *)pData, rLen);
                if(is.peek() == std::char_traits<char>::eof()) {
                    break;
                }
            }
            free(pData);
            int totalLen = ostrstream.str().size();
            qInfo("recv size %d response size %d %d", ostrstream.str().size(),
                  response.getContentLength(),
                  response.hasContentLength());
            pData = (uint8_t *) malloc(totalLen);
            if(pData) {
                *pOutData = pData;
                memcpy(pData, ostrstream.str().data(), totalLen);
                return totalLen;
            }else {
                qInfo("malloc failed %d ", totalLen);
                *pOutData = nullptr;
                return RET_ERR;
            }

            return RET_ERR;
        }else {
            qInfo("request failed");
        }
        return RET_ERR;
    }
    catch (TimeoutException& ex)
    {
        return RET_ERR;
    }
    catch (NetException& ex)
    {
        return RET_ERR;
    }
    catch(...){
        return RET_ERR;
    }
    return RET_ERR;
}



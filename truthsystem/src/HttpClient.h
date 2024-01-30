#pragma once
#include <iostream>
#include <string>
#include <map>

#define RET_OK 0
#define RET_ERR -1

class HttpClient
{
public:
	HttpClient();
    HttpClient(std::string server, int port);
	~HttpClient();
    void init(const std::string server, const int port);
    int post(const std::string url, const std::string &postJson, std::string &recvJson);
    int get(const std::string &url, std::string &recvString);
    int get(const std::string &url, std::map<std::string,std::string> & param, std::string &recvString);
    int put(const std::string url, uint8_t *pData, int fileSize, std::string &recvString);

private:
    std::string m_server;
	int m_port;
	//string m_token;
};


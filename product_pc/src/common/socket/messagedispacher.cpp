#include "messagedispacher.h"

MessageDispacher::MessageDispacher()
{
}

MessageDispacher::~MessageDispacher()
{

}


MessageDispacher* MessageDispacher::getInstance()
{
    static MessageDispacher instance;
    return &instance;
}

void MessageDispacher::RegisterCallBack(int msgId, MSG_CALLBACK_FUNC func)
{
    if(msgId < MSG_ID_MAX && msgId >= 0) {
        m_callback[msgId].push_back(func);
    }
}

void MessageDispacher::SendMessage(int msgId, int errorCode, QJsonObject msg)
{
    if(msgId < 0 || msgId > MSG_ID_MAX) return;
    std::vector<MSG_CALLBACK_FUNC> &callback = m_callback[msgId];
    for(int i=0; i<callback.size(); i++) {
        callback[i](msg, errorCode);
    }
}

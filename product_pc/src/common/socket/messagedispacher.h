#ifndef MESSAGEDISPACHER_H
#define MESSAGEDISPACHER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <functional>
#include "messageid.h"
#include <comdll.h>

typedef std::function<void(QJsonObject, int)> MSG_CALLBACK_FUNC;

class Q_API_FUNCTION MessageDispacher
{
public:
    explicit MessageDispacher();
    ~MessageDispacher();
    static MessageDispacher* getInstance();
    void SendMessage(int msgId, int errorCode, QJsonObject msg = QJsonObject());
    void RegisterCallBack(int msgId, MSG_CALLBACK_FUNC func);


private:
    std::vector <MSG_CALLBACK_FUNC> m_callback[MSG_ID_MAX];
};

#endif // MESSAGEDISPACHER_H

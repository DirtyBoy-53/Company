#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>
#include <comdll.h>

class Q_API_FUNCTION JsonManager
{
public:
    JsonManager();

    QString splitMsg(int msgId, int action, QJsonObject jsonNode);
    QString splitMsg(int msgId, int action);
    bool recvMsg(QString msg);

private:
    std::map<int, bool> m_idMap;
};

#endif // JSONMANAGER_H

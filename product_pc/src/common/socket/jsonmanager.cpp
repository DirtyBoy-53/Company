#include "jsonmanager.h"

#include <QDebug>
#include <thread>
#include "messagedispacher.h"

JsonManager::JsonManager()
{

}

QString JsonManager::splitMsg(int msgId, int action, QJsonObject jsonNode)
{
    QJsonObject json;
    json.insert("id", msgId);
    json.insert("action", action);
    json.insert("content", QJsonValue(jsonNode));
    return QString(QJsonDocument(json).toJson(QJsonDocument::Compact)).simplified();
}

QString JsonManager::splitMsg(int msgId, int action)
{
    QJsonObject json;
    json.insert("id", msgId);
    json.insert("action", action);
    return QString(QJsonDocument(json).toJson(QJsonDocument::Compact)).simplified();
}

bool JsonManager::recvMsg(QString msg)
{
    if(msg.isEmpty()) {
        return false;
    }

    do{
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(msg.toLocal8Bit().data(), &jsonError);
        if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            break;
        }

        int id=-1;
        int action;
        int retCode = 0;
        QJsonObject object = document.object();

        if(object.contains("id")) {
            id = object.value("id").toInt();
        } else {
            break;
        }
        if(object.contains("action")) {
            action = object.value("action").toInt();
        } else {
            break;
        }
        if(object.contains("retcode")) {
            retCode = object.value("retcode").toInt();
        }

        if(object.contains("content")) {
            QJsonObject nodeObject = object.value("content").toObject();
            MessageDispacher::getInstance()->SendMessage(id, retCode, nodeObject);
        } else {
            MessageDispacher::getInstance()->SendMessage(id, retCode);
        }
    }while(false);

    return true;
}

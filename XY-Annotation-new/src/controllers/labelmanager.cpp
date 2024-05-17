#include "labelmanager.h"

LabelManager::LabelManager(QObject *parent) : QObject(parent)
{
    currentMaxId=0;
}

void LabelManager::checkLabel(QString label) const {
    if (labels.find(label)==labels.end())
        throw "can not find label "+ label +" in label manager";
}

QJsonArray LabelManager::toJsonArray(){
    QJsonArray json;
    for (auto label:labels){
        json.append(label.toJsonObject());
    }
    return json;
}

void LabelManager::fromJsonArray(QJsonArray json)
{
    QList<LabelProperty> items;
    for (int i=0;i<json.size();i++){
        QJsonValue value = json.at(i);
        if (value.isObject()){
            LabelProperty item;
            item.fromJsonObject(value.toObject());
            items.push_back(item);
        }
    }
    for (auto item: items){
        addLabel(item.m_label, item.m_color, item.m_visible, item.m_id);
    }
}

void LabelManager::fromJsonObject(QJsonObject json)
{
    if (json.contains("labels")){
        QJsonValue value = json.value("labels");
        if (value.isArray())
            fromJsonArray(value.toArray());
        else {
//            throw JsonException("value of <labels> is illegal");
        }
    }else{
        //        qDebug()<<"no content <labels> in json";
    }
}

void LabelManager::addLabel(QString label, QColor color, bool visible,int id){
    if (id==-1) id=++currentMaxId; else currentMaxId = std::max(id, currentMaxId);
    labels[label] = LabelProperty(label, color, visible, id);
    emit labelAdded(label, color, visible, id);
    emit labelChanged();
}

void LabelManager::removeLabel(QString label){
    labels.remove(label);
    emit labelRemoved(label);
    emit labelChanged();
}

void LabelManager::setColor(QString label, QColor color){
    checkLabel(label);
    if (labels[label].m_color != color){
        labels[label].m_color = color;
        emit colorChanged(label, color);
        emit labelChanged();
    }
}

void LabelManager::setVisible(QString label, bool visible){
    checkLabel(label);
    if (labels[label].m_visible != visible){
        labels[label].m_visible = visible;
        emit visibelChanged(label, visible);
        emit labelChanged();
    }
}

void LabelManager::allClear()
{
    labels.clear();
    currentMaxId=0;
    emit allCleared();
    //    emit configChanged();
}

//---------------------------------LabelProperty-------------------------------------//

LabelProperty::LabelProperty(QString label, QColor color, bool visible, int id) :
    m_label(label), m_color(color), m_visible(visible),m_id(id) { }

LabelProperty::LabelProperty():m_label(), m_color(), m_visible(true), m_id(-1) {
}

void LabelProperty::operator =(const LabelProperty &label)
{
    this->m_color = label.m_color;
    this->m_id = label.m_id;
    this->m_label = label.m_label;
    this->m_visible = label.m_visible;
}

void LabelProperty::setProperty(QString label, QColor color, bool visible, int id)
{
    m_label = label;
    m_color = color;
    m_visible = visible;
    m_id = id;
}

QJsonObject LabelProperty::toJsonObject(){
    QJsonArray colorJson;
    colorJson.append(m_color.red());
    colorJson.append(m_color.green());
    colorJson.append(m_color.blue());
    QJsonObject json;
    json.insert("label", m_label);
    json.insert("id", m_id);
    json.insert("color", colorJson);
    json.insert("visible", m_visible);
    return json;
}

void LabelProperty::fromJsonObject(QJsonObject json)
{
    if (json.contains("label")){
        QJsonValue value = json.value("label");
        if (value.isString()){
            m_label = value.toString();
        }else{
//            throw JsonException("value of <label> is illegal");
        }
    }else{
//        throw JsonException("no data <label>");
    }

    if (json.contains("color")){
        QJsonValue value = json.value("color");
        if (value.isArray()){
            QJsonArray array = value.toArray();
            if (!array.at(0).isDouble() || !array.at(1).isDouble() || !array.at(2).isDouble()){
//                throw JsonException("value of <color> is illegal");
            }
            int r=static_cast<int>(array.at(0).toDouble());
            int g=static_cast<int>(array.at(1).toDouble());
            int b=static_cast<int>(array.at(2).toDouble());
            m_color = QColor(r,g,b);
        }else{
//            throw JsonException("value of <color> is illegal");
        }
    }else{
//        throw JsonException("no data <color>");
    }

    if (json.contains("visible")){
        QJsonValue value = json.value("visible");
        if (value.isBool()){
            m_visible = value.toBool();
        }else{
//            throw JsonException("value of <visible> is illegal");
        }
    }else{
//        throw JsonException("no data <visible>");
    }

    if (json.contains("id")){
        QJsonValue value = json.value("id");
        if (value.isDouble()){
            m_id = static_cast<int>(value.toDouble());
        }else{
//            throw JsonException("value of <id> is illegal");
        }
    }else{
//        throw JsonException("no data <id>");
    }
}

#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H

#include <QJsonObject>

#include <string>

#include "labelmanager.h"

class JsonException : public std::exception{
public:
    JsonException(std::string message) : msg(message){}
    const char* what() const noexcept{
        return msg.c_str();
    }
private:
    std::string msg;
};

class AnnotationContainer;
class AnnotationItem{
    friend AnnotationContainer;
public:
    AnnotationItem() : label(),id(-1){}
    AnnotationItem(QString label,int id):label(label),id(id){}
    virtual ~AnnotationItem(){}

    QString getLabel() const {return label;}
    int getId() const {return id;}
    virtual QString toStr() const = 0;
    virtual QJsonObject toJsonObject() const{
        QJsonObject json;
        json.insert("label",label);
        json.insert("id",id);
        return json;
    }
    virtual void fromJsonObject(const QJsonObject &json){
        if(json.contains("label")){
            QJsonValue value = json.value("label");
            if(value.isString()){
                label = value.toString();
            }else{
                throw JsonException("value of <label> is illegal");
            }
        }else{
            throw JsonException("no data <label>");
        }
        if (json.contains("id")){
            QJsonValue value = json.value("id");
            if (value.isDouble()){
                id = static_cast<int>(value.toDouble());
            }else{
                throw JsonException("value of <id> is illegal");
            }
        }else{
            throw JsonException("no data <id>");
        }
    }
protected:
    QString label;
    int id;
};

#endif // ANNOTATIONITEM_H

#ifndef CPROPERTYXML_H
#define CPROPERTYXML_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>

class CPropertyXml : public QObject
{
    Q_OBJECT
public:
    explicit CPropertyXml(QObject *parent = 0);

    bool load(QString path);
    bool save(QString path);
    void unload();

    QDomElement addGroup(QString name);
    bool deleteGroup(QString name);

    bool setAttribute(QString group, QString item, QString attr, QString value, bool hasNotCover = true);
    bool getAttribute(QString group, QString item, QString attr, QString& value);
    bool removeAttribute(QString group, QString item, QString attr);

    bool setValue(QString group, QString item, QString key, QString value, bool hasNotCover = true);
    bool getValue(QString group, QString item, QString key, QString& value);
    bool removeValue(QString group, QString item, QString key);

    bool groupExists(QString group);
    bool itemExists(QString group, QString item);

private:
    QDomDocument mDomXml;
    QDomElement  mDomRootElem;

    bool mBoolIsOpen;
    QDomElement findGroup(QString name);

    QString mErrString;
};

#endif // CPROPERTYXML_H

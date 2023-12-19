#include "CPropertyXml.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>


CPropertyXml::CPropertyXml(QObject *parent) : QObject(parent)
{
    mBoolIsOpen = false;
    mErrString.clear();
    mDomXml.setContent(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><root></root>"),true);
    mDomRootElem = mDomXml.firstChildElement("root");
    mBoolIsOpen = true;
}

bool CPropertyXml::load(QString path)
{
    if (mBoolIsOpen) {
        mDomRootElem.clear();
        mDomXml.clear();
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo("加载XML文件失败，文件不存在");

        return false;
    }
    QByteArray allData = file.readAll();
    file.close();
    if (!mDomXml.setContent(allData)) {
        qInfo("load failed ....");
        QFile::remove(path);
        mDomXml.setContent(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><root></root>"),true);
        mDomRootElem = mDomXml.firstChildElement("root");
    }

    mDomRootElem = mDomXml.firstChildElement("root");
    if(mDomRootElem.isNull()) {
        qInfo("file err reset");
        QFile::remove(path);
        mDomXml.setContent(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><root></root>"),true);
        mDomRootElem = mDomXml.firstChildElement("root");
    }
    if (mDomRootElem.isNull()) {
        qInfo("XML 没有root结点");
        mErrString = "XML 没有root结点";
        return false;
    }

    mBoolIsOpen = true;
    return true;
}

bool CPropertyXml::save(QString path)
{
    if (mDomXml.isNull()) {
        qInfo("xml is null");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly |QIODevice::Truncate)) {
        qInfo("open xml is failed");
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("utf-8");
    file.write(mDomXml.toByteArray(4));
//    mDomXml.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();
    qInfo("save %s succ", path.toStdString().data());
    return true;
}

void CPropertyXml::unload()
{
    mDomXml.clear();
}

QDomElement CPropertyXml::addGroup(QString name)
{
    if (mDomRootElem.isNull()) {
        mErrString = "root结点为空";
        return QDomElement();
    }


    QDomElement groupElem = mDomXml.createElement(name);
    groupElem.setAttribute("type", "Group");
    mDomRootElem.appendChild(groupElem);

    return groupElem;
}

QDomElement CPropertyXml::findGroup(QString name)
{
    return mDomRootElem.firstChildElement(name);
}

bool CPropertyXml::deleteGroup(QString name)
{
    QDomElement groupItem = mDomRootElem.firstChildElement(name);
    if (!groupItem.isNull()) {
        mDomRootElem.removeChild(groupItem);
    }

    return true;
}

bool CPropertyXml::setAttribute(QString group, QString item, QString attr, QString value, bool hasNotCover)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        groupItem = addGroup(group);
        if (groupItem.isNull()) {
            return false;
        }
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        itemX = mDomXml.createElement(item);
        if (itemX.isNull()) {
            return false;
        }
        groupItem.appendChild(itemX);
    }

    bool hasAttr = itemX.hasAttribute(attr);
    // 不存在这个属性，或者需要进行覆盖处理就需要设置属性值
    if (!hasAttr || !hasNotCover) {
        itemX.setAttribute(attr, value);
    }

    return true;
}

bool CPropertyXml::getAttribute(QString group, QString item, QString attr, QString &value)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        return false;
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        return false;
    }

    if (itemX.hasAttribute(attr)) {
        value = itemX.attribute(attr);
        return true;
    }

    return false;
}

bool CPropertyXml::removeAttribute(QString group, QString item, QString attr)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        return false;
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        return false;
    }

    if (itemX.hasAttribute(attr)) {
        itemX.removeAttribute(attr);
    }

    return true;
}

bool CPropertyXml::setValue(QString group, QString item, QString key, QString value, bool hasNotCover)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        groupItem = addGroup(group);
        if (groupItem.isNull()) {
            qInfo("group item is null");
            return false;
        }
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        qInfo("item is null create it");
        itemX = mDomXml.createElement(item);
        groupItem.appendChild(itemX);
    }

    QDomNode nodeKey;
    if (itemX.hasChildNodes()) {
        for (QDomNode node = itemX.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (node.nodeName() == key) {
                nodeKey = node;
                break;
            }
        }
    }

    // 不存在这个值，或者需要进行覆盖处理就需要重新设置值
    if (nodeKey.isNull()) {
        nodeKey = mDomXml.createElement(key);
        QDomText text = mDomXml.createTextNode(value);
        nodeKey.appendChild(text);
        itemX.appendChild(nodeKey);
    } else {
        if (!hasNotCover) {
            QDomText text = nodeKey.firstChild().toText();
            nodeKey.removeChild(text);
            text = mDomXml.createTextNode(value);
            nodeKey.appendChild(text);
        }
    }

    return true;
}

bool CPropertyXml::getValue(QString group, QString item, QString key, QString& value)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        qDebug() << "find group failed";
        return false;
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
//        qDebug() << "find item failed: " << item;
        return false;
    }

    if (itemX.hasChildNodes()) {
        for (QDomNode node = itemX.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (node.nodeName() == key) {
                value = node.firstChild().toText().data();
                return true;
            }
        }
    }

    return false;
}

bool CPropertyXml::removeValue(QString group, QString item, QString key)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        return false;
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        return false;
    }

    if (itemX.hasChildNodes()) {
        for (QDomNode node = itemX.firstChild(); !node.isNull(); node = itemX.nextSibling()) {
            if (node.nodeName() == key) {
                itemX.removeChild(node);
                return true;
            }
        }
    }

    return false;
}

bool CPropertyXml::groupExists(QString group)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        return false;
    }

    return true;
}

bool CPropertyXml::itemExists(QString group, QString item)
{
    QDomElement groupItem = findGroup(group);
    if (groupItem.isNull()) {
        return false;
    }

    QDomElement itemX = groupItem.firstChildElement(item);
    if (itemX.isNull()) {
        return false;
    }

    return true;
}

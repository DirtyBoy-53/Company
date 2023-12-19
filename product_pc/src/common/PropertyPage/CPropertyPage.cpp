#include "CPropertyPage.h"
#include <QLayout>

CPropertyPage::CPropertyPage(QWidget *parent) : QObject(parent)
{
    if (parent == 0) {
        qDebug() << "need set a parent widget first";
    }
    mParent = parent;
	
    mMainBrowser = nullptr;
    newPropFrame();
}

CPropertyPage::~CPropertyPage()
{
    //        for (int foo = 0; foo < mPointSet.size(); foo++) {
    //            QObject* pObj = mPointSet.at(foo);
    //            if (pObj != nullptr) {
    //                delete pObj;
    //                pObj = nullptr;
    //            }
    //        }

    mPointSet.clear();

    if (mMainBrowser != nullptr) {
        delete mMainBrowser;
        mMainBrowser = nullptr;
    }
}

QString CPropertyPage::errString()
{
    return mErrString;
}

QString CPropertyPage::itemDescription()
{
    return mDescription;
}

void CPropertyPage::combine()
{
    mMainBrowser->addProperty(mGroupProperty);
}

void CPropertyPage::slotItemChanged(QtBrowserItem *_t1) // 获取对字段的描述
{
    QtProperty* proper = _t1->property();
    QtAbstractPropertyManager* manager = proper->propertyManager();
    mDescription.clear();
    mDescription.append(proper->propertyName());
    mDescription.append(":");
    mDescription.append(manager->property("propertyDesp").toString());
    emit sigSendCurrentDesp(mDescription);
}

void CPropertyPage::addGroupBox(QString name, QString desp)
{
    mGroupManager = new QtGroupPropertyManager(mMainBrowser);
    mGroupProperty = mGroupManager->addProperty(name);
    mGroupManager->setProperty("propertyType", "Group");
    mGroupManager->setProperty("propertyDesp", desp);

    mPointSet.push_back(mGroupManager);
}

void CPropertyPage::addTextEdit(QString name, QString value, QString desp)
{
    QtStringPropertyManager* pTextManager = new QtStringPropertyManager(mGroupManager);
    QtProperty* textProp = pTextManager->addProperty(name);
    pTextManager->setValue(textProp, value);
    pTextManager->setProperty("propertyType", "String");
    pTextManager->setProperty("propertyDesp", desp);
    mGroupProperty->addSubProperty(textProp);

    QtLineEditFactory* pEditFactory = new QtLineEditFactory();
    mMainBrowser->setFactoryForManager(pTextManager, pEditFactory);

    mPointSet.push_back(pTextManager);
    mPointSet.push_back(pEditFactory);
}

void CPropertyPage::addIntSpin(QString name, int value, int min, int max, QString desp)
{
    QtIntPropertyManager* pIntManager = new QtIntPropertyManager(mGroupManager);
    QtProperty* intProp = pIntManager->addProperty(name);
    pIntManager->setValue(intProp, value);
    pIntManager->setMinimum(intProp, min);
    pIntManager->setMaximum(intProp, max);
    pIntManager->setProperty("propertyType", "Int");
    pIntManager->setProperty("propertyDesp", desp);

    QtSpinBoxFactory* pIntFactory = new QtSpinBoxFactory();
    mMainBrowser->setFactoryForManager(pIntManager, pIntFactory);
    mGroupProperty->addSubProperty(intProp);

    mPointSet.push_back(pIntManager);
    mPointSet.push_back(pIntFactory);
}

void CPropertyPage::addDoubleSpin(QString name, double value, double min, double max, QString desp)
{
    QtDoublePropertyManager* pDoubleManager = new QtDoublePropertyManager(mGroupManager);
    QtProperty* doubleProp = pDoubleManager->addProperty(name);
    pDoubleManager->setValue(doubleProp, value);
    pDoubleManager->setProperty("propertyType", "Double");
    pDoubleManager->setProperty("propertyDesp", desp);
    pDoubleManager->setMinimum(doubleProp, min);
    pDoubleManager->setMaximum(doubleProp, max);

    QtDoubleSpinBoxFactory* pDoubleFactory = new QtDoubleSpinBoxFactory();
    mMainBrowser->setFactoryForManager(pDoubleManager, pDoubleFactory);
    mGroupProperty->addSubProperty(doubleProp);

    mPointSet.push_back(pDoubleManager);
    mPointSet.push_back(pDoubleFactory);
}

void CPropertyPage::addCombBox(QString name, QStringList list, QString value, QString desp)
{
    QtEnumPropertyManager* pEnumManager = new QtEnumPropertyManager(mGroupManager);
    QtProperty* enumProp = pEnumManager->addProperty(name);
    pEnumManager->setEnumNames(enumProp, list);
    for (int foo = 0; foo < list.size(); foo++) {
        if (list.at(foo) == value) {
            pEnumManager->setValue(enumProp, foo);
            pEnumManager->setProperty("propertyType", "Enum");
            pEnumManager->setProperty("propertyDesp", desp);
            break;
        }
    }

    QtEnumEditorFactory* pEnumFactory = new QtEnumEditorFactory();
    mMainBrowser->setFactoryForManager(pEnumManager, pEnumFactory);
    mGroupProperty->addSubProperty(enumProp);

    mPointSet.push_back(pEnumManager);
    mPointSet.push_back(pEnumFactory);
}

void CPropertyPage::addFilePath(QString name, QString value, QString desp)
{
    FilePathManager* pFilePath = new FilePathManager();
    QtProperty* pathProperty = pFilePath->addProperty(name);
    pFilePath->setValue(pathProperty, value);
    pFilePath->setProperty("propertyType", "FilePath");
    pFilePath->setProperty("propertyDesp", desp);

    FileEditFactory* pFileFactory = new FileEditFactory(0, true);
    mMainBrowser->setFactoryForManager(pFilePath, pFileFactory);
    mGroupProperty->addSubProperty(pathProperty);

    mPointSet.push_back(pFilePath);
    mPointSet.push_back(pFileFactory);
}

void CPropertyPage::addDirPath(QString name, QString value, QString desp)
{
    FilePathManager* pFilePath = new FilePathManager();
    QtProperty* pathProperty = pFilePath->addProperty(name);
    pFilePath->setValue(pathProperty, value);
    pFilePath->setProperty("propertyType", "FilePath");
    pFilePath->setProperty("propertyDesp", desp);

    FileEditFactory* pFileFactory = new FileEditFactory(0, false);
    mMainBrowser->setFactoryForManager(pFilePath, pFileFactory);
    mGroupProperty->addSubProperty(pathProperty);

    mPointSet.push_back(pFilePath);
    mPointSet.push_back(pFileFactory);
}

void CPropertyPage::addCheckBox(QString name, bool value, QString desp)
{
    QtBoolPropertyManager* boolManager = new QtBoolPropertyManager(mGroupManager);
    QtProperty* boolProp = boolManager->addProperty(name);
    boolManager->setValue(boolProp, value);
    boolManager->setProperty("propertyType", "Bool");
    boolManager->setProperty("propertyDesp", desp);
    mGroupProperty->addSubProperty(boolProp);

    QtCheckBoxFactory* boolFactory = new QtCheckBoxFactory();
    mMainBrowser->setFactoryForManager(boolManager, boolFactory);

    mPointSet.push_back(boolManager);
    mPointSet.push_back(boolFactory);
}

void CPropertyPage::addRect(QString name, QRectF rect, QString desp)
{
    QtVariantPropertyManager* rectManager = new QtVariantPropertyManager(mGroupManager);
    QtVariantProperty* rectProp = rectManager->addProperty(QVariant::RectF, name);
    rectManager->setValue(rectProp, QVariant(rect));
    mGroupProperty->addSubProperty(rectProp);
    rectManager->setProperty("propertyType", "Rect");
    rectManager->setProperty("propertyDesp", desp);

    QtVariantEditorFactory* rectFactory = new QtVariantEditorFactory;
    mMainBrowser->setFactoryForManager(rectManager, rectFactory);

    mPointSet.push_back(rectManager);
    mPointSet.push_back(rectFactory);
}

bool CPropertyPage::saveToXml(QString filePath)
{
    // filePath = "D:/test.xml";
    QDomDocument domXml;
    QDomElement elemRoot = domXml.documentElement();
    if (elemRoot.isNull()) {
	    domXml.setContent(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?><root></root>"),true);
	    elemRoot = domXml.firstChildElement("root");
    }

    QList<QtBrowserItem *> topItems = mMainBrowser->topLevelItems();

    for (int foo = 0; foo < topItems.size(); foo++) {
        QtBrowserItem* item = topItems.at(foo);
        QtProperty* prop = item->property();
        QtGroupPropertyManager* groupMgr = (QtGroupPropertyManager*)prop->propertyManager();
        QString gType = groupMgr->property("propertyType").toString();

        QDomElement elemG = domXml.createElement(prop->propertyName());
        elemG.setAttribute("type", gType);

        QList<QtProperty* > subProp = prop->subProperties();
        for (int fjj = 0; fjj < subProp.size(); fjj++) {
            QtProperty* propItem = subProp.at(fjj);
            QString propName = propItem->propertyName();
            QString propValue = propItem->valueText();
            QtAbstractPropertyManager* manager = propItem->propertyManager();
            QString type = manager->property("propertyType").toString();
            QString desp = manager->property("propertyDesp").toString();
//            qDebug() << "Name: " << propName << "  Value: " << propValue << "  type: " << type << "  Desp: " << desp;

            // 用名字作为结点，类型作为属性，值和描述作为子节点
            // add child node
            QDomElement elemChild = domXml.createElement(propName);
            // add type
            elemChild.setAttribute("type", type);

            if (type == "Int") {
                QtIntPropertyManager* intM = (QtIntPropertyManager*)manager;
//                qDebug() << "min " << intM->minimum(propItem);
//                qDebug() << "max " << intM->maximum(propItem);
                elemChild.setAttribute("minimum", QString::number(intM->minimum(propItem)));
                elemChild.setAttribute("maximum", QString::number(intM->maximum(propItem)));
            }

            if (type == "Double") {
                QtDoublePropertyManager* intM = (QtDoublePropertyManager*)manager;
//                qDebug() << "min " << intM->minimum(propItem);
//                qDebug() << "max " << intM->maximum(propItem);
                elemChild.setAttribute("minimum", QString::number(intM->minimum(propItem),'f', 3));
                elemChild.setAttribute("maximum", QString::number(intM->maximum(propItem),'f', 3));
            }

            if (type == "Enum") {
                QtEnumPropertyManager* intM = (QtEnumPropertyManager*)manager;
//                qDebug() << "EnumNames: " << intM->enumNames(propItem);
                elemChild.setAttribute("EnumNames", intM->enumNames(propItem).join(','));
            }
            // add value
            QDomElement elemValue = domXml.createElement("value");
            QDomText nodeValue = domXml.createTextNode(propValue);
            elemValue.appendChild(nodeValue);
            elemChild.appendChild(elemValue);
            // add desp
            QDomElement elemDesp = domXml.createElement("desp");
            QDomText nodeDesp = domXml.createTextNode(desp);
            elemDesp.appendChild(nodeDesp);
            elemChild.appendChild(elemDesp);

            elemG.appendChild(elemChild);
        }
        elemRoot.appendChild(elemG);
    }
    domXml.appendChild(elemRoot);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        mErrString = filePath + "Open Failed";
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("utf-8");
    domXml.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();

    return true;
}

bool CPropertyPage::loadToUI(QString filePath, QString filters)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);

    QDomDocument domXml;
    domXml.setContent(&file);
    file.close();
    QDomElement domRoot = domXml.documentElement();
    if (domRoot.isNull()) {
        mErrString = "Root node is not exists";
        return false;
    }
    if (domRoot.tagName() != "root") {
        mErrString = "Root node is " + domRoot.tagName() + "not root";
        return false;
    }

    QStringList filterset = filters.split("|");

    newPropFrame();
    QDomElement first = domRoot.firstChildElement();
    qDebug() << first.nodeName();
    while (!first.isNull()) {
        QString rootType = first.attribute("type");
        bool bFiltersOk = false;
        if (filterset.size() > 0) {
            foreach(QString a, filterset) {
                qDebug() << "+++" << a << "+" << first.nodeName();
                if (first.nodeName().compare(a, Qt::CaseInsensitive) == 0) {
                    bFiltersOk = true;
                    break;
                }
            }
        } else {
            bFiltersOk = true;
        }
        if (rootType == "Group") {
            if (bFiltersOk) {
                addGroupBox(first.nodeName());
            } else {
                first = first.nextSiblingElement();
                continue;
            }
        } else {
            mErrString = "first node is not group";
            qDebug() << mErrString;
            return false;
        }

        QDomElement first_node = first.firstChildElement();
        while (!first_node.isNull()) {
            QString name = first_node.nodeName();
            qDebug() << "name: " << first_node.nodeName();
            QString type = first_node.attribute("type");
            qDebug() << "type" << first_node.attribute("type");
            QString value, desp;
            if (first_node.hasChildNodes()) {
                for (QDomNode n = first_node.firstChild(); !n.isNull() ;n = n.nextSibling()) {
                    // QDomNode child= first_node.firstChild();
                    if (n.nodeName() == "value") {
                        QDomText tx = n.firstChild().toText();
                        value = tx.data();
                        qDebug() << "value: " << value;
                    } else if (n.nodeName() == "desp") {
                        QDomText tx = n.firstChild().toText();
                        desp = tx.data();
                        qDebug() << "desp: " << tx.data();
                    }
                }
            }
            if (type == "Double") {
                double min = first_node.attribute("minimum").toDouble();
                double max = first_node.attribute("maximum").toDouble();
                qDebug() << min;
                qDebug() << max;
                addDoubleSpin(name, value.toDouble(), min, max, desp);
            } else if (type == "Int") {
                int min = first_node.attribute("minimum").toInt();
                int max = first_node.attribute("maximum").toInt();
                qDebug() << min;
                qDebug() << max;
                addIntSpin(name, value.toInt(), min, max, desp);
            }else if (type == "Enum") {
                QString enumNames = first_node.attribute("EnumNames");
                qDebug() << enumNames;
                QStringList list = enumNames.split(",");
                addCombBox(name, list, value, desp);
            } else if (type == "String") {
                addTextEdit(name, value, desp);
            } else if (type == "Bool") {
                bool bVal = (value == "True");
                addCheckBox(name, bVal, desp);
            } else if (type == "FilePath") {
                addFilePath(name, value, desp);
            } else if (type == "DirPath") {
                addDirPath(name, value, desp);
            }

            first_node = first_node.nextSiblingElement();
        }
        combine();
        first = first.nextSiblingElement();
    }

    return true;
}

bool CPropertyPage::loadFromXml(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);

    mDomXml.setContent(&file);
    mDomRootElement = mDomXml.documentElement();
    if (mDomRootElement.isNull()) {
        mErrString = "Root node is not exists";
        return false;
    }
    if (mDomRootElement.tagName() != "root") {
        mErrString = "Root node is " + mDomRootElement.tagName() + "not root";
        return false;
    }

    file.close();
    return true;
}

void CPropertyPage::newPropFrame()
{
    if (mMainBrowser) {
        delete mMainBrowser;
        mMainBrowser = nullptr;
    }
    mMainBrowser = new QtTreePropertyBrowser(mParent);
    mMainBrowser->setRootIsDecorated(true);
    mMainBrowser->setHeaderVisible(false);
    mMainBrowser->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
    mMainBrowser->resize(mParent->size());
    mMainBrowser->setStyleSheet("QHeaderView::section \
                                { \
                                    border:none; \
                                } \
                                QTreeView \
                                { \
                                    show-decoration-selected:1; \
                                    alternate-background-color:rgb(233,245,252); \
                                    background:rgb(255,255,255); \
                                    gridline-color:#242424; \
                                    font-size:16px; \
                                } \
                                QTreeView::item \
                                { \
                                   height:35px; \
                                   padding:2px; \
                                } \
");
    //mMainBrowser->show();
    connect( mMainBrowser, &QtTreePropertyBrowser::currentItemChanged, this, &CPropertyPage::slotItemChanged);
}

bool CPropertyPage::getNodeText(QString name, QString type, QString group, QString &value)
{
    if (mDomXml.isNull()) {
        mErrString = "No Open XML";
        return false;
    }

    if (mDomRootElement.isNull()) {
        mErrString = "No Root XML";
        return false;
    }

    QDomElement first = mDomRootElement.firstChildElement();
    while (!first.isNull()) {
        QString gName = first.nodeName();
        QString gType = first.attribute("type");
        if (group.size() > 0) {
            if (gType != "Group" || gName != group) {
                first = first.nextSiblingElement();
                continue;
            }
        }

        QDomElement subElem = first.firstChildElement();
        while (!subElem.isNull()) {
            QString sName = subElem.nodeName();
            QString sType = subElem.attribute("type");

            if (name == sName /*&& type == sType*/) {
                if (!subElem.hasChildNodes()) {
                    mErrString = "No Item";
                    return false;
                }
                for (QDomNode n = subElem.firstChild(); !n.isNull() ;n = n.nextSibling()) {
                    if (n.nodeName() == "value") {
                        QDomText tx = n.firstChild().toText();
                        value = tx.data();
                        qDebug() << "value: " << value;
                        return true;
                    }
                }
            }
            subElem = subElem.nextSiblingElement();
        }
        first = first.nextSiblingElement();
    }

    mErrString = "No Item";
    return false;
}

bool CPropertyPage::getDouble(QString name, double &value)
{
    return getDoubleWithGroup(name, "", value);
}

bool CPropertyPage::getDoubleWithGroup(QString name, QString group, double &value)
{
    QString strValue;
    if (!getNodeText(name, "Double", group, strValue)) {
        return false;
    }

    value = strValue.toDouble();
    return true;
}

bool CPropertyPage::getInt(QString name, int &value)
{
    return getIntWithGroup(name, "", value);
}

bool CPropertyPage::getIntWithGroup(QString name, QString group, int &value)
{
    QString strValue;
    if (!getNodeText(name, "Int", group, strValue)) {
        return false;
    }

    value = strValue.toInt();
    return true;
}

bool CPropertyPage::getString(QString name, QString &value)
{
    return getStringWithGroup(name, "", value);
}

bool CPropertyPage::getStringWithGroup(QString name, QString group, QString &value)
{
    return getNodeText(name, "String", group, value);
}

bool CPropertyPage::getBool(QString name, bool &value)
{
    return getBoolWithGroup(name, "", value);
}

bool CPropertyPage::getBoolWithGroup(QString name, QString group, bool &value)
{
    QString strValue;
    if (!getNodeText(name, "Bool", group, strValue)) {
        return false;
    }

    return ((strValue.compare("true", Qt::CaseInsensitive) == 0) ? true : false);
}


QDomElement CPropertyPage::findGroupBox(QString name)
{
    if (mDomXml.isNull()) {
        mErrString = "No Open XML";
        return QDomElement();
    }

    if (mDomRootElement.isNull()) {
        mErrString = "No Root XML";
        return QDomElement();
    }

    QDomElement first = mDomRootElement.firstChildElement();
    while (!first.isNull()) {
        QString gName = first.nodeName();
        QString gType = first.attribute("type");
        if (gType == "Group" && gName == name) {
            return first;
        }

        first = first.nextSiblingElement();
    }

    mErrString = "No Item";
    return QDomElement();
}


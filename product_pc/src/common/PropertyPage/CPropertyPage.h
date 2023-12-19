#ifndef CPROPERTYPAGE_H
#define CPROPERTYPAGE_H

#include <QObject>
#include <QDebug>

#include <QList>
#include <QDomDocument>
#include <QFile>
#include <QRectF>

#include "qttreepropertybrowser.h"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "filepathmanager.h"
#include "fileeditfactory.h"
#include "qtvariantproperty.h"


#define VARIABLES
#define FUNCTIONS

class CPropertyPage : public QObject
{
    Q_OBJECT
public FUNCTIONS:
    explicit CPropertyPage(QWidget *parent = 0);
    ~CPropertyPage();

    QString errString();
    QString itemDescription();


    /// \brief addGroupBox
    /// \param name 组名
    /// \param desp 对分组进行描述
    /// 构建一个Group，标志一个组的开始
    void addGroupBox(QString name, QString desp = "");

    void addTextEdit(QString name, QString value, QString desp = "");
    void addIntSpin(QString name, int value, int min, int max, QString desp = "");
    void addDoubleSpin(QString name,  double value, double min, double max, QString desp = "");
    void addCombBox(QString name, QStringList list, QString value, QString desp = "");
    void addFilePath(QString name, QString value, QString desp = "");
    void addDirPath(QString name, QString value, QString desp = "");
    void addCheckBox(QString name, bool value, QString desp = "");
    void addRect(QString name, QRectF rect, QString desp = "");

    /// \brief combine()
    /// 对一个组的内容进行一个组合，标志着一个组的完成
    void combine();

    bool saveToXml(QString filePath);
    bool loadToUI(QString filePath, QString filters = "");
    bool loadFromXml(QString filePath);
    void newPropFrame();


    bool getDouble(QString name, double& value);
    bool getDoubleWithGroup(QString name, QString group, double& value);
    bool getInt(QString name, int& value);
    bool getIntWithGroup(QString name, QString group, int& value);
    bool getString(QString name, QString& value);
    bool getStringWithGroup(QString name, QString group, QString& value);
    // 一定要对Bool类型的值进行返回值的判断
    bool getBool(QString name, bool& value);
    bool getBoolWithGroup(QString name, QString group, bool& value);

private FUNCTIONS:
    QDomElement findGroupBox(QString name);
    bool getNodeText(QString name, QString type, QString group, QString& value);



private VARIABLES:
    QtTreePropertyBrowser* mMainBrowser;
    QtGroupPropertyManager* mGroupManager;
    QtProperty* mGroupProperty;

    QVector<QObject* > mPointSet;

    QString mDescription;

    QString mErrString;

    QDomDocument mDomXml;
    QDomElement  mDomRootElement;
    QWidget* mParent;

signals:
    // 对获得的描述传递到终端以进行显示
    void sigSendCurrentDesp(QString discription);

public slots:
    void slotItemChanged(QtBrowserItem * _t1);
};

#endif // CPROPERTYPAGE_H

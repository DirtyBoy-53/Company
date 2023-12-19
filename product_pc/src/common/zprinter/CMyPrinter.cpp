#include "CMyPrinter.h"
#include <QDebug>
#include <QPrinterInfo>
#include <QAxObject>

CMyPrinter::CMyPrinter(QObject *parent) : QObject(parent)
{
    OleInitialize(0);
    mPrintApplication = new QAxObject();
}

QStringList CMyPrinter::printerName()
{
    return QPrinterInfo::availablePrinterNames();
}

QString CMyPrinter::defalutPrinterName()
{
    return QPrinterInfo::defaultPrinterName();
}

QStringList CMyPrinter::items()
{
    if (!mPrintFormat) {
        mErrString = ("打印机未加载");
        return QStringList("");
    }
    QAxObject* subStr = ((QAxObject *)mPrintFormat)->querySubObject("NamedSubStrings");
    if (!subStr) {
        mErrString = ("获取具名数据源失败");
        return QStringList("");
    }
    QString str = subStr->dynamicCall("GetAll(QString, QString)", ":", ";").toString();
    return str.split(";");
}

bool CMyPrinter::load(QString p, QString t, int c, int s)
{
    mErrString.clear();
    QString nameP = "";
    QStringList infor = QPrinterInfo::availablePrinterNames();
    for (int foo = 0;  foo < infor.size(); foo++) {
        QString name = infor.at(foo);
        if (name.contains(p)) {
            qDebug() << name;
            nameP = name;
            break;
        }
    }

    if (nameP.isEmpty()) {
        qDebug() << "printer is null";
        mErrString = ("没有找到匹配的打印机");
        return false;
    }

    ((QAxObject *)mPrintApplication)->setControl("BarTender.Application");
    qDebug() << "BarTender.Application";
    QAxObject *printFormats = ((QAxObject *)mPrintApplication)->querySubObject("Formats");
    if (!printFormats) {
        mErrString = ("获取子接口失败");
        return false;
    }
    qDebug() << "BarTender.Formats";
    qDebug() << t;
    mPrintFormat = printFormats->querySubObject("Open(char*,bool,char*)", t.toStdString().c_str(), false, "");
    if (!mPrintFormat) {
        qDebug() << "get print format failed!";
        mErrString = ("开启模板文件失败");
        return false;
    }
    qDebug() << "Open";
    QAxObject * printSetup = ((QAxObject *)mPrintFormat)->querySubObject("PrintSetup");
    qDebug() << "PrintSetup";
    printSetup->dynamicCall("Printer", p);
    qDebug() << p;
    printSetup->dynamicCall("IdenticalCopiesOfLabel", c);
    qDebug() << c;
    printSetup->dynamicCall("NumberSerializedLabels", s);
    qDebug() << s;
    return true;
}

bool CMyPrinter::setItem(QString key, QString value)
{
    QAxObject* subStr = ((QAxObject *)mPrintFormat)->querySubObject("NamedSubStrings");
    QString str = subStr->dynamicCall("GetAll(QString, QString)", ":", ";").toString();
    if (str.indexOf(key) == -1) {
        qDebug() << "get item key failed";
        mErrString = ("查找key") + key + ("值失败");
        return false;
    }
    ((QAxObject *)mPrintFormat)->dynamicCall("SetNamedSubStringValue(QString,QString)", key, value);
    QString v = ((QAxObject *)mPrintFormat)->dynamicCall("GetNamedSubStringValue(QString)", key).toString();
    if (value.compare(v) != 0) {
        mErrString = ("查找value") + value + ("值失败");
        qDebug() << "get set value: " << v;
        return false;
    }
    return true;
}

// always true
bool CMyPrinter::print()
{
    int result = ((QAxObject *)mPrintFormat)->dynamicCall("PrintOut(bool, bool)", true, false).toInt();
    return true;
}

CMyPrinter::~CMyPrinter()
{
    if(mPrintFormat) {
        ((QAxObject *)mPrintFormat)->dynamicCall("Close(int)", 2);
        delete mPrintFormat;
        mPrintFormat = nullptr;
    }

    if (mPrintApplication) {
        ((QAxObject *)mPrintApplication)->dynamicCall("Quit(int)", 2);
        delete mPrintApplication;
        mPrintApplication = nullptr;
    }
    OleUninitialize();
}

QString CMyPrinter::errString()
{
    return mErrString;
}


#pragma once
#ifndef INISETTINGS_H
#define INISETTINGS_H

#include <QSettings>
#include <QSharedPointer>
#include <QTextCodec>
#include <QCoreApplication>
#include <QFile>

class  IniSettings
{
public:
    QSharedPointer<QSettings> m_iniFile;

public:
    void settings_init(const QString &path); //初始化QSettings对象，并读取ini配置文件,如果不存在配置文件，则创建
    void setValue(const QString &section, const QString &key, const QString &value); //写入配置项（section:节点，key:键名，value:键值）
    void setValueList(const QString &section,const QString & key,const QStringList &value);
    void removeNode(const QString &section);                           //移除节点(包括其中所有的键值)
    void removeKey(const QString &section, const QString &key);               //移除节点中的某个键值
    QString getValue(const QString &section, const QString &key, const QString &defaultValue);  //读配置项
    QStringList getValueList(const QString &section, const QString &key, QString defaultValue = ""); //批量读取
};

#endif // SETTING_H

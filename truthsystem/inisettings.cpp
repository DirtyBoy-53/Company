#include "inisettings.h"

void IniSettings::settings_init(const QString &path)
{
    m_iniFile = QSharedPointer<QSettings>(new QSettings(path, QSettings::IniFormat));
    //确保中文能使用
    //注意此处使用的编码取决于ini文件的编码格式，例如ini使用的是UTF-8则使用UTF-8
    m_iniFile->setIniCodec(QTextCodec::codecForName("UTF-8"));

}

void IniSettings::setValue(const QString &section, const QString &key, const QString &value)
{
    QString path = QString("%1/%2").arg(section).arg(key);
    m_iniFile->setValue(path, value);    //设置键名和键值
}

void IniSettings::setValueList(const QString &section,const QString & key,const QStringList &value)
{
   setValue(section,key,QVariant(value).toString());
}

void IniSettings::removeNode(const QString &section)
{
    m_iniFile->remove(section);
}

void IniSettings::removeKey(const QString &section, const QString &key)
{
    /*QString path = QString("%1/%2").arg(section).arg(key);
    m_iniFile->remove(path);*/
    m_iniFile->beginGroup(section);
    m_iniFile->remove(key);
    m_iniFile->endGroup();
}

QString IniSettings::getValue(const QString &section, const QString &key, const QString &defaultValue)
{
    QString path = QString("%1/%2").arg(section).arg(key);
    QString result = m_iniFile->value(path, defaultValue).toString();
    return result;
}

QStringList IniSettings::getValueList(const QString & section, const QString & key, QString defaultValue)
{
    QVariant variant = m_iniFile->value(QString("%1/").arg(section) + key);
    QStringList list = variant.value<QStringList>();
    return list;
}

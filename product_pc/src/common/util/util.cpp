#include "util.h"
#include "QCoreApplication"
#include <QVector>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <configinfo.h>
#include <QProcess>

namespace Util {

int getFilesCount(QString path, QString fill)
{
    QDir dir;
    if(!dir.exists(path)) {
        return 0;
    }
    dir.setPath(path);

    QStringList filter;
    filter << fill;
    dir.setNameFilters(filter);
    QList<QFileInfo>* fileInfo = new QList<QFileInfo>(dir.entryInfoList(filter));
    return fileInfo->count();
}

QFileInfoList getAllFilesInfo(QString path, QString type)
{
    QDir dir;
    if(dir.exists(path)) {
        dir.setPath(path);

        QStringList filter;
        filter << type;
        dir.setNameFilters(filter);
        return dir.entryInfoList(filter);
    }
}

QString getAllPath(QString dir)
{
    return QCoreApplication::applicationDirPath() + "/" + dir;
}

QString createFileDir(QString filePath)
{
    QDir dir(filePath);
    if (dir.exists(filePath)){
        return filePath;
    }

    QString parentDir = createFileDir(filePath.mid(0,filePath.lastIndexOf('/')));
    QString dirname = filePath.mid(filePath.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if ( !dirname.isEmpty()){
        parentPath.mkpath(dirname);
    }

    return parentDir + "/" + dirname;
}

QStringList splitStr(QByteArray str, QString dot)
{
    QVector<int> vPos;
    int pos = 0;
    QStringList strList;
    while(pos != -1) {
        pos = str.indexOf(dot, pos+dot.size());
        vPos.push_back(pos);
    }
    for(int i=0; i<vPos.size(); i++) {
        if(i != vPos.size()-1) {
            strList.push_back(str.mid(vPos[i]+dot.size(), vPos[i+1]));
        } else {
            strList.push_back(str.right(vPos[i]+dot.size()));
        }
    }
    return strList;
}

QString md5sum(QString filePath)
{
    QByteArray fileData;
    QFile file(filePath);
    if(!file.open(QFile::ReadOnly))
    {
        file.close();
        return "";
    }
    else
    {
        while(!file.atEnd())
        {
            fileData += file.readAll();
        }
    }
    QString md5;
    QByteArray bb;
    QCryptographicHash md(QCryptographicHash::Md5);
    md.addData(fileData);
    bb = md.result();
    md5.append(bb.toHex());
    file.close();
    return md5.toUpper();
}

QString getHostMacAddress()
{
    QString confiMac = ConfigInfo::getInstance()->getValueString("SEMAC");
    if(confiMac.size() > 0) {
        return confiMac;
    }
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        if(nets[i].humanReadableName().contains("以太网")) {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}

quint32 crc32ChkCalc(QString filePath)
{
    QByteArray fileData;
    QFile file(filePath);
    if(!file.open(QFile::ReadOnly))
    {
        file.close();
        return 0x0;
    }
    fileData = file.readAll();
    quint8* data = (quint8*)fileData.data();
    quint32 len = fileData.size();
    quint32 i = 0;
    quint32 crc = 0xFFFFFFFF;
    quint32 datatmp;

    for (i=0; i < len; i++)
    {
        datatmp = data[i];
        datatmp <<= 24;
        for (int j = 0; j < 8; j++ )
        {
            if (((datatmp & 0x80000000) ^ (crc & 0x80000000)) == 0x80000000 )
            {
                crc <<= 1;
                crc ^= 0x4C11DB7;
            }else{
                crc <<= 1;
            }
            datatmp <<= 1;
        }
    }
    return crc;
}

int stringConvert34to10(QString value)
{
    QString ret;
    for(int i=0; i<value.size(); i++) {
        char it = value[i].toLatin1();
        if(it >= '0' && it <= '9') {
            ret.append(it);
        } else if(it >= 'A' && it < 'I') {
            ret.append(it);
        } else if(it >= 'J' && it < 'O') {
            ret.append(it - 1);
        } else if(it >= 'P' && it <= 'Z') {
            ret.append(it - 2);
        }
    }
    bool flag;
    return ret.toInt(&flag, 34);
}

bool testPing(QString ip)
{
    QString strArg = "ping " + ip + " -n 1 -i 2";
    int exitCode = QProcess::execute(strArg);
    if(exitCode == 0) {
        //it's alive
        qDebug() << "shell ping " + ip + " sucessed!";
        return true;
    } else {
        qDebug() << "shell ping " + ip + " failed!";
        //发射IP离线的信号
        return false;
    }

}

}

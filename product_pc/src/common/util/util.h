#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QDir>
#include <comdll.h>

namespace  Util {

Q_API_FUNCTION int  getFilesCount(QString path, QString fill);
Q_API_FUNCTION QString  getAllPath(QString dir="");
Q_API_FUNCTION QStringList  splitStr(QByteArray str, QString dot);
Q_API_FUNCTION QFileInfoList  getAllFilesInfo(QString dir, QString type);
Q_API_FUNCTION QString  createFileDir(QString filePath);
QString md5sum(QString filePath);
QString getHostMacAddress();
quint32 crc32ChkCalc(QString filePath);

int stringConvert34to10(QString value);
bool testPing(QString ip);

}


#endif // UTIL_H

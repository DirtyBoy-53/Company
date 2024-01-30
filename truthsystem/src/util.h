#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QDir>

namespace Util {

    int getFilesCount(QString path, QString fill);
    QString getAllPath(QString dir="");
    QStringList splitStr(QByteArray str, QString dot);
    QFileInfoList getAllFilesInfo(QString dir, QString type);
    QString createFileDir(QString filePath);
    uint64_t getTimeMs();
    void writeRelData(QByteArray bData, QString path, bool iAppend = false);

    void uyvyScaleCenter(uint8_t * pY,
                         uint8_t * pU,
                         uint8_t * pV,
                         uint8_t * pSrcData,
                         int w,
                         int h,
                         int scale,
                         int &sw,
                         int &sh);
}


#endif // UTIL_H

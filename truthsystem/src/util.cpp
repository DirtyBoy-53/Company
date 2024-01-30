#include "util.h"
#include "QCoreApplication"
#include <QVector>
#include <chrono>
#include <QDebug>

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
    return QCoreApplication::applicationDirPath() + dir;
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

uint64_t getTimeMs()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    return (uint64_t)ms.count();
}

void writeRelData(QByteArray bData, QString path, bool iAppend)
{
    QString filePath = Util::getAllPath(path);
    QFileInfo fileInfo(filePath);
    Util::createFileDir(fileInfo.absoluteDir().path());
    QFile file(filePath);
    if(iAppend) {
        file.open(QFile::Append);
    }else {
        file.open(QFile::WriteOnly | QFile::Truncate);
    }
    if(file.isOpen()) {
        file.write(bData);
        file.close();
    }else {
    }
}

void uyvyScaleCenter(uint8_t *pY, uint8_t *pU, uint8_t *pV, uint8_t *pSrcData, int w, int h, int scale, int &sw, int &sh)
{
    int x,y;
    int centerX = w / 2;
    int centerY = h / 2;
    if(scale == 4) {
        sw = w / 4;
        sh = h / 4;
    }else if(scale == 2) {
        sw = w / 2;
        sh = h / 2;
    }else {
       sw =  w;
       sh = h;
    }
    x = (centerX - sw / 2) & 0xfffe;
    y = (centerY - sh / 2) & 0xfffe;
    uint8_t * pSData = pSrcData + y * w * 2 + x * 2;
    int iY = 0, iUV = 0;
    for(int i = 0;i < sh; i ++) {
        for(int j = 0; j < sw * 2; j += 4) {
            uint8_t * pData = pSData  + j + i * w * 2 ;
            pY[iY]     = pData[1];
            pY[iY + 1] = pData[3];
            pU[iUV]    = pData[0];
            pV[iUV]    = pData[2];
            iUV ++;
            iY += 2;
        }
    }
}

}



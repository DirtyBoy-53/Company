#include "mainwindow.h"
#include <QApplication>
#include <QtMessageHandler>
#include <QFile>
#include <gdmessagelogger.h>
#include <configinfo.h>
#include <util.h>
#include <QTime>

//#include "crc16_modbus.h"
//#include "mtfimgcheck.h"

bool AddEnvironmentPath(const QString& strPath)
{
    // 取得环境变量
    QByteArray variable = qgetenv("PATH");
    QList<QByteArray> paths = variable.split(';');
    QByteArray newPathByte = strPath.toLatin1();
    // 查找是否有我们需要的
    int nIndex = paths.indexOf(newPathByte);
    if (nIndex < 0) {
        paths.push_back(newPathByte);
        QByteArray total;
        for (QByteArray& path : paths)	// 合成一个新的
            total += path + ";";
        // 设置系统变量
        qputenv("PATH", total);
        return true;
    }
    return true;
}

//#include <QFile>
int main(int argc, char *argv[])
{

//    uint32_t x = 1043677053;
//    union {
//        float f;
//        uint32_t i;
//    }Z;
//    Z.i = x;
//    qDebug() << Z.f;
//    char retRst[] = {0x01, 0x03, 0x04, 0x29, 0xA9, 0x00, 0x01};
//    if (retRst[2] != 0x04) {
//        qDebug() << "not 4";
//    }

//    ushort d30 = ((retRst[3] << 8) & 0xFF00) | (retRst[4] & 0xFF);
//    ushort d31 = ((retRst[5] << 8) & 0xFF00) | (retRst[6] & 0xFF);


//    ushort status = 0x2008;
//    qDebug("D30: %04X, D31: %04X, %X, %X", d30, d31, d31 & 0x02, d30 & status);
//    if ((d31 & 0x02) == 0x02) {
//        qDebug() << "is warning";
//    }


//    if ((d30 & status) == status) {
//        qDebug() << "isOK";
//    }

//    return 0;

    // ST,NT,+  0.386kg
//    QString str = "ST,NT,-  0.386kg";
//    QStringList list = str.split(",");
//    qDebug() << list.at(2);

//    QString e = list.at(2);
//    int sign = (e.at(0) == '+') ? 1 : -1;
//    e.chop(2);
//    e = e.mid(1);
//    e.simplified();

//    qDebug() << e;
//    float data = e.toFloat() * sign;

//    qDebug() << data;

//    return 1;

//    unsigned char p[6] = {0x01, 0x03, 0xEC, 0xD1, 0x00, 0x1E}; // A1(6) 6B(7) ==> 6BA1(高位-> 低位)
//    ushort x = crc16_modbus(p, 6);
//    qDebug("%X %X %X", x, x&0xFF, (x >> 8) & 0xFF);
//    return 0;

//    mtfImgCheck imgCheck;
//    QFile file("D:/0.y16");
//    QFile fileEx("D:/1.y16");
//    bool retx = fileEx.open(QIODevice::ReadWrite);
//    bool ret = file.open(QIODevice::ReadWrite);
//    if (!ret) {
//        qDebug() << "open failed";
//        return 0;
//    }

//    QByteArray ba = file.readAll();

//    unsigned short minValue = 0, maxValue = 0;
//    imgCheck.Get_Y16_hist((unsigned short*)ba.data(), maxValue, minValue, 640, 512);
//    QString log = QString("IMAGE CHECK max value: %1, min value: %2").arg(maxValue).arg(minValue);
//    qDebug() << log;
//    if (maxValue - minValue < 150) {
//        qDebug() << "failed";
//        return -1;
//    }

//    fileEx.write(ba);
//    fileEx.close();

//    int imgCount[5] = {0};
//    imgCheck.Get_target_count((unsigned short*)ba.data(), imgCount, maxValue, minValue, 640, 512);
//    log = QString("TARGET COUNT 0: %1, 1: %2, 2: %3, 3: %4, 4: %5").arg(imgCount[0]).arg(imgCount[1]).arg(imgCount[2]).arg(imgCount[3]).arg(imgCount[4]);
//    qDebug() << log;
//    bool brst = true;
//    for (int foo = 0; foo < 5; foo++) {
//        if (imgCount[foo] < 1000 || imgCount[foo] > 5000) {
//            brst = false;
//            break;
//        }
//    }
//    if (!brst) {
//        qDebug() << "failed";
//        return -1;
//    }


//    return 0;
    QApplication a(argc, argv);

#ifndef _DEBUG
    initGdMessageLogger();
#endif

    ConfigInfo::getInstance();
//    QFile file(":/qss/stylesheet.qss");
//    if(file.open(QIODevice::ReadOnly)) {
//        QString str = file.readAll();
//        qApp->setStyleSheet(str);
//        file.close();
//    }

//    AddEnvironmentPath(QCoreApplication::applicationDirPath());
    MainWindow w;
    w.login();

    return a.exec();
}

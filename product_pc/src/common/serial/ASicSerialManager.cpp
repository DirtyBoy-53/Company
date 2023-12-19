#include "ASicSerialManager.h"
#include <QRegExp>
#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>


#define BURN_LENGTH 256

ASicSerialManager::ASicSerialManager(QObject *parent)
    : QObject{parent}
{
    m_serial = new CSerialPort();
}

ASicSerialManager* ASicSerialManager::getInstance()
{
    static ASicSerialManager instance;
    return &instance;
}

QString ASicSerialManager::errString()
{
    return mErrString;
}

bool ASicSerialManager::openSerial(const QString &serialName)
{
    SerialParam param;
    param.baudRate    = QSerialPort::Baud115200;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;

    return m_serial->openSerial(serialName, param, true);
}

void ASicSerialManager::closeSerial()
{
    m_serial->closeSerial();
}

bool ASicSerialManager::waitForStartup(int timeout)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        m_serial->sendCommand((unsigned char*)"\r\n", 2);
        if (m_serial->findSerial("SH>", 1000)) {
            return true;
        }
    }

    return false;
}

bool ASicSerialManager::writeSN(QString sn)
{
    int nXor = xOr(sn.size(), sn);
    QString cmd = QString("isp SN_W %1 %2 %3\n").arg(sn.size()).arg(sn).arg(nXor);

    QString retCmp = "55aa11011f0";
    bool bWriteOk = false;
    for (int foo = 0; foo < 4; foo++) {
        QCoreApplication::processEvents();
        m_serial->sendCommandWait((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
        QByteArray read = m_serial->readSerialHold(3000);
        if(read.size() <= 0) {
            continue;
        }
        read = read.replace("\r\n", "***");
        qDebug() << read;

        QRegExp reg("55(\\w){9}(\\*){3}");
        int index = reg.indexIn(read);
        if (index >= 0) {
            bWriteOk = true;
            break;
        } else {
            qDebug() << "regex key failed";
        }
    }

    if (bWriteOk) {
        QString rSn;
        if (!readSN(rSn, sn.size())) {
            mErrString = "Read SN Fail";
            return false;
        }
        qDebug() << sn << " " << rSn;
        if (rSn.compare(sn) == 0) {
            return true;
        } else {
            mErrString = "sn_read differ sn_write";
            return false;
        }
    }

    return false;
}

bool ASicSerialManager::readSN(QString &sn, int len)
{
    QString cmd = "isp SN_R\n";

    bool bReadOk = false;
    for (int foo = 0; foo < 3; foo++) {
        m_serial->sendCommandWait((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
        QByteArray read = m_serial->readSerialHold(1000);
        read = read.replace("\r\n", "***");
        qDebug() << read;
        QString e = "(\\*){3}" +QString::number(len) ;
        qInfo()<<"exp  "<<e;
        QRegExp reg(e);
        int index = reg.indexIn(read);
        if (index >= 0) {
            sn = read.mid(index + 5, len);
            int xr = xOr(len, sn);
            qDebug() << xr;
            return true;
        } else {
            qDebug() << "regex key failed";
        }
    }

    return false;
}

bool ASicSerialManager::readVersion(QString &ver)
{
    QString cmd = "isp SWVER\n";

    for (int foo = 0; foo < 10; foo++) {
//        QByteArray read = m_serial->sendWaitForResp((unsigned char*)cmd.toLocal8Bit().data(), cmd.size(), strlen("+V1.0.12_20230310`"));
        m_serial->sendCommandWait((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
        QByteArray read = m_serial->readSerial(3000);
        if(read.size() <= 0) {
            continue;
        }
        qDebug() << read;

        QRegExp reg("V(\\w{1,})+.(\\w{1,})+.(\\w{1,})+[_.]{1}(\\d){8}");
        int index = reg.indexIn(read);
        if (index >= 0) {
            ver = reg.capturedTexts().at(0);

            return true;
        } else {
            qDebug() << "regex key failed";
        }
    }
    return false;
}

bool ASicSerialManager::readHwVersion(QString &ver)
{
    QString cmd = "isp HW_V\n";

    for (int foo = 0; foo < 10; foo++) {
//        QByteArray read = m_serial->sendWaitForResp((unsigned char*)cmd.toLocal8Bit().data(), cmd.size(), strlen("+V1.0.12_20230310`"));
        m_serial->sendCommandWait((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
        QByteArray read = m_serial->readSerial(3000);
        if(read.size() <= 0) {
            continue;
        }
        qDebug() << read;

        QRegExp reg("HW_V\\[\w+\\]");
        int index = reg.indexIn(read);
        if (index >= 0) {
            ver = reg.capturedTexts().at(0);

            return true;
        } else {
            qDebug() << "regex key failed";
        }
    }
    return false;
}

bool ASicSerialManager::getCrc(QString &sCrc)
{
    QString cmd = "isp downloadfilechk\n";

    for (int foo = 0; foo < 10; foo++) {
        m_serial->sendCommandWait((unsigned char*)cmd.toLocal8Bit().data(), cmd.size());
        QByteArray read = m_serial->readSerial(5000);
        if(read.size() <= 0) {
            continue;
        }
        qDebug() << read;

        QRegExp reg("\\[app:\\w{1,10}\\]\\[boot:\\w{1,10}\\]\\[wc:\\w{1,10}\\]\\[stimg:\\w{1,10}\\]\\[config:\\w{1,10}\\]");
        int index = reg.indexIn(read);
        if (index >= 0) {
            sCrc = reg.capturedTexts().at(0);
            return true;
        } else {
            qDebug() << "regex key failed";
        }
    }
    return false;
}

int ASicSerialManager::getProductMode(int timeout)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        m_serial->sendCommand((unsigned char*)"\r\n", 2);
        QByteArray read = m_serial->readSerialHold(1000);
        qDebug() << "read from asic : " << read;
        if (read.contains("SH>")) {
            return 1;
        }

        if (read.contains("ImgFlag check failed")) {
            return 2;
        }
    }

    return -1;
}

bool ASicSerialManager::enterMbMode(int timeout)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        m_serial->sendCommand((unsigned char*)"\r", 1);
        if (m_serial->findSerial("mb #", 1000)) {
            return true;
        }
    }

    return false;
}

bool ASicSerialManager::enterPtzMode(int timeout)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        QByteArray find;
        find.append((char)0x02);
        find.append((char)0x24);
        find.append((char)0x00);
        find.append((char)0x03);
        m_serial->sendCommand((unsigned char*)"ptz\r", 4);
        if (m_serial->findSerial(find, 500)) {
            return true;
        }
    }

    return false;
}

bool ASicSerialManager::enterDebug(int timeout)
{
    unsigned char btSend[4] = {0x02, 0x5a, 0x00, 0x03};
    unsigned char btSend1[4] = {0x02, 0x14, 0x00, 0x03};

    QByteArray btFind1, btFind2, btFind3;
    btFind1.append((char)0x02);
    btFind1.append((char)0xa5);
    btFind1.append((char)0x00);
    btFind1.append((char)0x03);

    btFind2.append((char)0x02);
    btFind2.append((char)0x24);
    btFind2.append((char)0x00);
    btFind2.append((char)0x03);

    btFind3.append((char)0x02);
    btFind3.append((char)0x05);
    btFind3.append((char)0x00);
    btFind3.append((char)0x03);

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();
        QByteArray read = m_serial->readSerialHold(10);
        qDebug() << "read from asic : " << read;
        if (read.contains(btFind1)) {
            m_serial->sendCommand(btSend, 4);
        } else if (read.contains(btFind2)) {
            m_serial->sendCommand(btSend1, 4);
        } else if (read.contains(btFind3)) {
            return true;
        }
    }

    return false;
}

bool ASicSerialManager::burnFile(QString filePath, long addr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray allData = file.readAll();
    file.close();

    QString fileName = filePath.split("/").last();

    int allLen = allData.size();
    long tempAddr = addr;
    bool isLastFrame = false;
    for (int foo = 0; foo < allLen; foo += BURN_LENGTH) {
        // 取得读取的数据...
        int len = allLen - foo;
        if (len > BURN_LENGTH) {
            len = BURN_LENGTH;
        } else {
            isLastFrame = true;
        }

        sigLogOut("正在烧录" + fileName + " 文件[" + QString::number(foo * 100 / allLen) + "%]");

        unsigned char szData[BURN_LENGTH] = {0x00};
        memcpy(szData, allData.data() + foo, len);

        qDebug() << "burn addr: 0x" << QString::number(tempAddr, 16);
        unsigned char burnAddr[4];
        burnAddr[0] = (tempAddr >> 24) & 0xFF;
        burnAddr[1] = (tempAddr >> 16) & 0xFF;
        burnAddr[2] = (tempAddr >> 8) & 0xFF;
        burnAddr[3] = tempAddr & 0xFF;

        unsigned int chkSum = len / 4;
        for (int f = 0; f < 4; f++) {
            chkSum += burnAddr[f];
        }
        for (int f = 0; f < len; f++) {
            chkSum += szData[f];
        }

        qDebug() << "burn crc: " << chkSum;
        unsigned char szSend[1024] = {0x00};
        szSend[0] = 0x02;
        szSend[1] = 0x18;
        szSend[2] = chkSum & 0xFF;
        szSend[3] = 0x03;
        memcpy(szSend + 4, burnAddr, 4);
        szSend[11] = len / 4;
        memcpy(szSend + 12, szData, len);
        for (int f = 0; f < len + 12; f += 16) {
            qDebug("%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X",
                   szSend[f], szSend[f+1], szSend[f+2], szSend[f+3],
                    szSend[f+4], szSend[f+5], szSend[f+6], szSend[f+7],
                    szSend[f+8], szSend[f+9], szSend[f+10], szSend[f+11],
                    szSend[f+12], szSend[f+13], szSend[f+14], szSend[f+15] );
        }

        qDebug() << "\r";

        QByteArray baFind;
        baFind.append((char)0x02);
        baFind.append((char)0x05);
        baFind.append((char)0x00);
        baFind.append((char)0x03);
        m_serial->sendCommand(szSend, len + 12);
        if (!m_serial->findSerial(baFind, 1000)) {
            return false;
        } else {
            qDebug() << "has get 02 05 00 03";
        }

        tempAddr += len;
    }
    sigLogOut(fileName + " 烧录完成");

    return isLastFrame;

}

bool ASicSerialManager::burnFileEx(long addr)
{
    unsigned char burnAddrX[4];
    burnAddrX[0] = (addr >> 24) & 0xFF;
    burnAddrX[1] = (addr >> 16) & 0xFF;
    burnAddrX[2] = (addr >> 8) & 0xFF;
    burnAddrX[3] = addr & 0xFF;

    unsigned int chkSum = 0;
    for (int f = 0; f < 4; f++) {
        chkSum += burnAddrX[f];
    }

    unsigned char szSend[1024] = {0x00};
    memset (szSend, 0x00, 1024);
    szSend[0] = 0x02;
    szSend[1] = 0x1b;
    szSend[2] = chkSum & 0xFF;
    szSend[3] = 0x03;
    memcpy(szSend + 4, burnAddrX, 4);
    qDebug("%02X%02X%02X%02X %02X%02X%02X%02X",
           szSend[0], szSend[1], szSend[2], szSend[3],
            szSend[4], szSend[5], szSend[6], szSend[7] );

    QByteArray baFind;
    baFind.append((char)0x02);
    baFind.append((char)0x05);
    baFind.append((char)0x00);
    baFind.append((char)0x03);
    m_serial->sendCommand(szSend, 8);
    if (m_serial->findSerial(baFind, 1000)) {
        return true;
    }

    return false;
}

int ASicSerialManager::xOr(int iv, QString data)
{

    QByteArray ba = data.toLocal8Bit();
    int nxOr = iv;
    for (int foo = 0; foo < ba.size(); foo++) {
        unsigned char c = (unsigned char)ba.at(foo);
        nxOr ^= c;
    }

    return nxOr;
}

int ASicSerialManager::c2n(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return (10 + c - 'a');
    }

    if (c >= 'A' && c <= 'F') {
        return (10 + c - 'A');
    }

    return 65535;
}

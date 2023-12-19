#include "CPlcRelative.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QThread>
#include "crc16_modbus.h"

QMutex CPlcRelative::mPlcMutex;

CPlcRelative::CPlcRelative() :
mSerialRead(nullptr),
mSerialWrite(nullptr)
{
    mPlcInitializeOk = false;
    mSerialRead = nullptr;
    mSerialWrite = nullptr;
    mCurrentPosition = PositionErr;
}


bool CPlcRelative::getSerialResource()
{
    QMutexLocker locker(&mPlcMutex);
    if (mSerialRead && mSerialWrite) {
        return true;
    }

    QString crtPath = QCoreApplication::applicationDirPath();
    mDllPath.clear();
    mDllPath.append(crtPath);
    mDllPath.append("/");
    mDllPath.append(DATALINKDLLNAME);

    QFile file(mDllPath);
    if(!file.exists()) {
        mErrString.append("dll file is not exist in ");
        mErrString.append(mDllPath);
        return false;
    }

    mLibSerial.setFileName(mDllPath);
    if (!mLibSerial.load()) {
        mErrString.append(mLibSerial.errorString());
        return false;
    }

    typedef bool (*OBJ)(char*, void**);
    OBJ getObj = (OBJ)mLibSerial.resolve("GetInterface");
    if (!getObj) {
        mErrString.append("get interface 'GetInterface' failed!!!");
        mLibSerial.unload();
        return false;
    }

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&mSerialRead))) {
        mErrString.append("get read interface pointer failed!!!");
        mLibSerial.unload();
        return false;
    }

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&mSerialWrite))) {
        mErrString.append("get write interface pointer failed!!!");
        mLibSerial.unload();
        return false;
    }

    if (mSerialRead && mSerialWrite) {
        mErrString.clear();
        return true;
    }

    mErrString.append("unknow error");
    mLibSerial.unload();
    return false;
}

void  CPlcRelative::freeSerialResource()
{
    QMutexLocker locker(&mPlcMutex);
    if (mSerialRead) {
        //mSerialRead->linkDeInit();
        mSerialRead = nullptr;
    }

    if (mSerialWrite) {
        //mSerialWrite->linkDeInit();
        mSerialWrite = nullptr;
    }

    mLibSerial.unload();
}

QString &CPlcRelative::errorString()
{
    return mErrString;
}

bool CPlcRelative::initResource(QString writeName, int writeBandrate, QString readName, int readBandrate, bool hex)
{
    QMutexLocker locker(&mPlcMutex);
    if (!mPlcInitializeOk) {
        mSerialWrite->linkSetFormat(FormatHex);
        if (!mSerialWrite->linkSerialInit(writeName.toLocal8Bit().data(), writeBandrate)) {
            char szTmp[1024] = {0};
            char* pszTmp = szTmp;
            mSerialWrite->linkGetLastError(pszTmp);
            qDebug() << "PLC Write Serial Init Failed with the error msg: " << szTmp;
            return false;
        }

        mSerialRead->linkSetFormat(FormatHex);
        if (!mSerialRead->linkSerialInit(readName.toLocal8Bit().data(), readBandrate)) {
            char szTmp[1024] = {0};
            char* pszTmp = szTmp;
            mSerialRead->linkGetLastError(pszTmp);
            qDebug() << "PLC Read Serial Init Failed with the error msg: " << szTmp;
            return false;
        }
        mPlcInitializeOk = true;

        qDebug() << "PLC Serial port init ok!!!";
    }
    return mPlcInitializeOk;
}

void CPlcRelative::deInitResource()
{
    QMutexLocker locker(&mPlcMutex);
    if (mSerialRead) {
        mSerialRead->linkDeInit();
    }

    if (mSerialWrite) {
        mSerialWrite->linkDeInit();
    }

    mPlcInitializeOk = false;
}

CPlcRelative *CPlcRelative::instance()
{
    QMutexLocker locker(&mPlcMutex);
    static CPlcRelative plcRelative;
    return &plcRelative;
}

bool CPlcRelative::plcReset()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_RESET, BOOL_TRUE, pRet, iLen);


    // find...
    // 01 10 13 88 00 05 0A 00 06 00 00 00 00 00 00 00 00 F1 DD
    QByteArray header;
    header.append((char)0x01);
    header.append((char)0x10);
    header.append((char)0x13);
    header.append((char)0x88);
    header.append((char)0x00);
    header.append((char)0x05);
    header.append((char)0x0A);
    header.append((char)0x00);
    header.append((char)ACK_RESET_OK);
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x00);
    if (!sendAndFind(QByteArray((char*)cmdRet, iLen), header, 30*1000)) { // 变量可以开放
        return false;
    }

//    if ((unsigned char)read.at(8) != ACK_RESET_OK) {
//        qDebug() << "returned invalid data, return false;" << read.at(8);
//        return false;
//    }

    qDebug() << "fixture reset ok";
    return true;
}

bool CPlcRelative::plcTurnOver()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == PositionTurnOver) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_RAISE, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_RAISE_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture raised ok";

    mCurrentPosition = PositionTurnOver;
    return true;
}

bool CPlcRelative::plcTurnBack()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == PositionTurnBack) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_PLAIN, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_LOWER_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture lower ok";

    mCurrentPosition = PositionTurnBack;
    return true;
}

bool CPlcRelative::plcFixtureUp() // 上升
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == PositionFixtureUp) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_FIXTUREUP, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_UP_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture lower ok";

    mCurrentPosition = PositionFixtureUp;
    return true;
}

bool CPlcRelative::plcFixtureDown() // 下降
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == PositionFixtureDown) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_FIXTUREDOWN, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_DOWN_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture lower ok";

    mCurrentPosition = PositionFixtureDown;
    return true;
}

bool CPlcRelative::plcLightSetWork(bool bLight)
{
    if (!mPlcInitializeOk) {
        return false;
    }
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_WORKING, bLight ? BOOL_TRUE : BOOL_FALSE, pRet, iLen);
    mSerialRead->linkClearBuffer();
    mSerialWrite->linkWrite((char*)pRet, iLen);

    QThread::sleep(1);
    return true;
}

bool CPlcRelative::plcLockStatus(bool& status)
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);

    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_LOCKSTATUS, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) == ACK_LOCK_OK) {
        status = true;
        qDebug() << "lock status get ok: is locking";
        return true;
    } else if ((unsigned char)read.at(8) == ACK_UNLOCK_OK) {
        status = false;
        qDebug() << "lock status get ok: is unlocking";
        return true;
    } else {
        qDebug() << "returned invalid data, return false;";
    }

    qDebug() << "fixture lower ok";
    return false;
}

bool CPlcRelative::plcMoveTo20()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == Position20Temp) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_CALK_POS1, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 15000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture 20° back ok";

    mCurrentPosition = Position20Temp;
    return true;
}

bool CPlcRelative::plcMoveTo20T()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == Position20TieTemp) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_CALK_POS1T, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 15000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture 20° 贴附 back ok";

    mCurrentPosition = Position20TieTemp;
    return true;
}

bool CPlcRelative::plcMoveTo60()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == Position60Temp) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_CALK_POS2, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 25000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture 60° back ok";

    mCurrentPosition = Position60Temp;
    return true;
}

bool CPlcRelative::plcMoveTo60T()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == Position60TieTemp) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_CALK_POS2T, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 15000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture 60° 贴附 back ok";

    mCurrentPosition = Position60TieTemp;
    return true;
}

bool CPlcRelative::plcMoveToOrign()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    QMutexLocker locker(&mPlcMutex);
    if (mCurrentPosition == PositionOrigin) {
        return true;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POSITION_CALK_ORIGIN, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 25000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture origin ok";

    mCurrentPosition = PositionOrigin;
    return true;
}

bool CPlcRelative::plcMoveUp()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_UPDOWN, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_RAISE_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture up ok";

    return true;
}

bool CPlcRelative::plcMoveDown()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_UPDOWN, BOOL_FALSE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 5000)) {
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_LOWER_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture down ok";

    return true;
}

bool CPlcRelative::plcMoveOrign()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    // start send...
    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(PLC_ORIGIN, BOOL_TRUE, pRet, iLen);

    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 25000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }

    qDebug() << "fixture origin ok";

    return true;
}

bool CPlcRelative::plcMoveToAxis(unsigned short pos)
{
    if (!mPlcInitializeOk) {
        return false;
    }

    QByteArray read;
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(pos, BOOL_TRUE, pRet, iLen);
    if (!sendAndRead(QByteArray((char*)cmdRet, iLen), read, 10000)) { // 变量可以开放
        return false;
    }

    if ((unsigned char)read.at(8) != ACK_GOPOS_OK) {
        qDebug() << "returned invalid data, return false;";
        return false;
    }
}

bool CPlcRelative::boxPowerup()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POWERUP, BOOL_TRUE, pRet, iLen);
    mSerialRead->linkClearBuffer();
    mSerialWrite->linkWrite((char*)pRet, iLen);

    return true;
}

bool CPlcRelative::boxPoweroff()
{
    if (!mPlcInitializeOk) {
        return false;
    }
    int iLen = 0;
    unsigned char cmdRet[CMDLEN] = {0x00};
    unsigned char* pRet = cmdRet;
    PLCCommandFormat::formatCommand(POWERUP, BOOL_FALSE, pRet, iLen);
    mSerialRead->linkClearBuffer();
    mSerialWrite->linkWrite((char*)pRet, iLen);

    return true;
}

bool CPlcRelative::sendAndRead(QByteArray send, QByteArray &read, int timeout)
{
    int rstlen = 0;
    char ucRst[128] = {0x00};
    char* pResult = ucRst;

    QString sendStr;
    for (int foo = 0; foo < send.size(); foo++) {
        sendStr += QString::number(send.at(foo), 16);
        sendStr += " ";
    }
    qDebug() << "Send: " << sendStr;

    //qDebug() << send.data();
    mSerialRead->linkClearBuffer();
    mSerialWrite->linkWrite(send.data(), send.size());
    QThread::msleep(2000);
    // find...
    QByteArray header;
    header.append((char)0x01);
    header.append((char)0x10);
    header.append((char)0x13);
    header.append((char)0x88);
    header.append((char)0x00);
    header.append((char)0x05);
    header.append((char)0x0A);

    qDebug() << "header is " << header;
    if (!mSerialRead->linkGet(header.toStdString().c_str(), header.size(), nullptr, RETCMDLEN, pResult, rstlen, timeout)) {
        qDebug() << "has not found " << header << " in data " <<  ucRst;
        return false;
    }
    QByteArray result = QByteArray((char*)ucRst, rstlen);
    qDebug() << "has found " << header << " in data " <<  result << "length is " << rstlen;
//    for (int foo= 0 ; foo < rstlen; foo++) {
//        qDebug("%02X", result.at(foo));
//    }

    if (rstlen < RETCMDLEN) {
        qDebug() << "returned data is shorter , not enough the length 19";
        return false;
    }

    unsigned char szResult[ RETCMDLEN ] = { 0x00 };
    memcpy(szResult, result.data(), RETCMDLEN - 2);
    unsigned short crc  = crc16_modbus(szResult, RETCMDLEN - 2);
    unsigned short crcRet = (((unsigned char)result.at(18) << 8) & 0xFF00) | (unsigned char)result.at(17);
    if (crc != crcRet) {
        qDebug("calc CRC: %04X, returned CRC: %04X", crc, crcRet);
        qDebug() << "CRC Calculate is failed!";
        return false;
    }

    qDebug() << "CRC compared ok";

    read = result;
    return true;
}

bool CPlcRelative::sendAndFind(QByteArray send, QByteArray find, int timeout)
{
    QString sendStr;
    for (int foo = 0; foo < send.size(); foo++) {
        sendStr += QString::number(send.at(foo), 16);
        sendStr += " ";
    }
    qDebug() << "Send: " << sendStr;

    //qDebug() << send.data();
    mSerialRead->linkClearBuffer();
    mSerialWrite->linkWrite(send.data(), send.size());
    QThread::msleep(2000);

    qDebug() << "header is " << find;
    if (mSerialRead->linkFind(find.toStdString().c_str(), find.size(), timeout)) {
        qDebug() << "has found " << find ;
        return true;
    }

    return false;
}

void CPlcRelative::slotPlcMoveTo(int pos)
{
    qDebug() << __FUNCTION__ << " " << QThread::currentThreadId();
    QMutexLocker locker(&mPlcMutex);
    if (pos != mCurrentPosition) {
        switch (pos) {
        case Position20Temp:
            qDebug() << "Ready to 20 black...";
            if (plcMoveTo20()) {
                mCurrentPosition = Position20Temp;
            }
            break;
        case Position60Temp:
            qDebug() << "Ready to 60 black...";
            if (plcMoveTo60()) {
                mCurrentPosition = Position60Temp;
            }
            break;
        case PositionOrigin:
            qDebug() << "Ready to origin...";
            if (plcMoveToOrign()) {
                mCurrentPosition = PositionOrigin;
            }
            break;
        case PositionTurnOver:
            qDebug() << "Ready to turn over...";
            if (plcTurnOver()) {
                mCurrentPosition = PositionTurnOver;
            }
            break;
        case PositionTurnBack:
            qDebug() << "Ready to turn back...";
            if (plcTurnBack()) {
                mCurrentPosition = PositionTurnBack;
            }
            break;
        }
    }

    emit sigCurrentPosition(mCurrentPosition);
}

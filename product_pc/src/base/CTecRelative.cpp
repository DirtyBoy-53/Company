#include "CTecRelative.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

QMutex CTecRelative::mTecMutex;
CTecRelative::CTecRelative()
{
    tecSerialPtr = nullptr;

    mBoolTecHasOpened = false;
    mBoolIsHeating = false;
}

bool CTecRelative::getTecResource()
{
    // QMutexLocker locker(&mTecMutex);
    if (tecSerialPtr) {
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

    if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&tecSerialPtr))) {
        mErrString.append("get read interface pointer failed!!!");
        mLibSerial.unload();
        return false;
    }

    if (tecSerialPtr) {
        mErrString.clear();
        return true;
    }

    mErrString.append("unknow error");
    mLibSerial.unload();
    return false;
}

void CTecRelative::freeTecResource()
{
    // QMutexLocker locker(&mTecMutex);
    if (tecSerialPtr) {
        //tecSerialPtr->linkRelease();
        tecSerialPtr = nullptr;
    }

    mLibSerial.unload();
}

QString CTecRelative::errorString()
{
    return mErrString;
}

bool CTecRelative::initResource(QString name, int bandrate)
{
    // QMutexLocker locker(&mTecMutex);
    if (!tecSerialPtr->linkSerialInit(name.toStdString().c_str(), bandrate)) {
        char szTmp[1024] ={0};
        char* pszTmp = szTmp;
        tecSerialPtr->linkGetLastError(pszTmp);
        qDebug() << "Init TEC Serial Port Failed with the error infor: " << pszTmp;
        return false;
    }
    mBoolTecHasOpened = true;
    mBoolIsHeating = false;
    return mBoolTecHasOpened;
}

void CTecRelative::deInitResource()
{
    // QMutexLocker locker(&mTecMutex);
    if (tecSerialPtr) {
        tecSerialPtr->linkDeInit();
    }
    mBoolTecHasOpened = false;
}

bool CTecRelative::tecStartHeating(double maxTemp)
{
    // qDebug() << __FUNCTION__;
    // QMutexLocker locker(&mTecMutex);
//    if (mBoolIsHeating) {
//        return true;
//    }

    QString strWrite = QString("TC1:TCADJUSTTEMP=%1\r").arg((int)maxTemp);
    tecSerialPtr->linkClearBuffer();
    tecSerialPtr->linkWrite(strWrite.toStdString().c_str(), strWrite.length());
    if (!tecSerialPtr->linkFind("REPLY=1", strlen("REPLY=1"), 5000)) {
        qDebug() << "not found 'REPLY=1'";
        return false;
    }
    qDebug() << strWrite + " OK";

    QThread::msleep(800);
    strWrite.clear();
    strWrite = QString("TC1:TCSW=1\r");
    tecSerialPtr->linkWrite(strWrite.toStdString().c_str(), strWrite.length());
    if (!tecSerialPtr->linkFind("REPLY=1", strlen("REPLY=1"), 5000)) {
        qDebug() << "not found 'REPLY=1'";
        return false;
    }
    qDebug() << strWrite + " OK";

    mBoolIsHeating = true;
    return true;
}

bool CTecRelative::tecStopHeating()
{
    // QMutexLocker locker(&mTecMutex);
//    if (!mBoolIsHeating) {
//        return true;
//    }

    QString strWrite = QString("TC1:TCADJUSTTEMP=37\r");
    tecSerialPtr->linkClearBuffer();
    tecSerialPtr->linkWrite(strWrite.toStdString().c_str(), strWrite.length());
    if (!tecSerialPtr->linkFind("REPLY=1", strlen("REPLY=1"), 5000)) {
        qDebug() << "not found 'REPLY=1'";
        return false;
    }
    qDebug() << strWrite + " OK";

    QThread::msleep(800);
    strWrite.clear();
    strWrite = QString("TC1:TCSW=1\r");
    tecSerialPtr->linkWrite(strWrite.toStdString().c_str(), strWrite.length());
    if (!tecSerialPtr->linkFind("REPLY=1", strlen("REPLY=1"), 5000)) {
        qDebug() << "not found 'REPLY=1'";
        return false;
    }
    qDebug() << strWrite + " OK";

    mBoolIsHeating = false;
    return true;
}

// TC1:TCACTUALTEMP?\r
double CTecRelative::tecGetCurrentTempture()
{
    //qDebug() << __FUNCTION__;
    // QMutexLocker locker(&mTecMutex);
    qDebug() << __FUNCTION__;
    char szRet[128] = {0};
    char* pRet = szRet;
    int iLen = 0;
    QString strWrite = QString("TC1:TCACTUALTEMP?\r");
    tecSerialPtr->linkWrite(strWrite.toStdString().c_str(), strWrite.length());
    if (!tecSerialPtr->linkGet("TCACTUALTEMP=", strlen("TCACTUALTEMP="), "\r", 1, pRet, iLen, 3000)) {
        qDebug() << "not get temprature result...";
        return -999.0f;
    }

    qDebug() << "get current temprature: " << pRet;
    char* p = strstr(pRet, "=");
    if (p) {
        qDebug() << atof(p + 1);

        return atof(p + 1);
    }

    return -1;
}

//void CTecRelative::slotHeating(double target)
//{
//    QMutexLocker locker(&mTecMutex);
//    if (tecStartHeating(target)) {
//        emit sigHeatingOk();
//    } else {
//        mErrString = "heating fail";
//    }
//}

//void CTecRelative::slotUnHeating()
//{
//    QMutexLocker locker(&mTecMutex);
//    if (tecStopHeating()) {
//        emit sigUnHeatingOk();
//    } else {
//        mErrString = "stop heating fail";
//    }
//}

//void CTecRelative::slotGetTemp(QString name)
//{
//    QMutexLocker locker(&mTecMutex);
//    double temp = tecGetCurrentTempture();
//    if (temp != -999.0f) {
//        emit sigCrtTemp(temp);
//    } else {
//        mErrString = "get tmp fail";
//    }
//}

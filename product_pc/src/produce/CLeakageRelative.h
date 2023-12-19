#ifndef CLEAKAGERELATIVE_H
#define CLEAKAGERELATIVE_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QLibrary>
#include <QThread>
#include <QCoreApplication>

#include "IDataLinkTransport.h"
#include "crc16_modbus.h"
#include <QFile>
#include <QDebug>

typedef struct{
    unsigned char id;
    unsigned char way; // 10 write, 03 read, 05 one bit
    unsigned char addr[2];
    unsigned char dlen[2];
    unsigned char len;
}AIR_STRUCT_HEADER;

enum ParamAddrInput{
    PI_ChooseProg = 0x6000,
    PI_SetPullAirTime,
    PI_SetPressureStableTime,
    PI_SetTestTime,
    PI_SetReleaseAirTime = 0x6009,
    PI_SetPressureUnit = 0x6035,
    PI_SetMinPressureFill = 0x6032,
    PI_SetMaxPressureFill = 0x6033,
    PI_SetSettingPressure = 0x6042,
    PI_SetVolumnUnit = 0x60A1,
    PI_SetTestVolumnValue = 0x6014,
    PI_SetTestPassTarget = 0x603C,
    PI_SetTestRefPassTarget = 0x603E
};

typedef struct{
    int AirProgId;
    int AirTestMode;
    int AirInstStatus;
    int AirAlarmStatus;
    int AirTestStep;
    double AirPressure;
    int AirPressureUnit;
    double AirLeakage;
    int AirLeakageUnit;
}ParamOutAir;

class CLeakageRelative : public QObject
{
    Q_OBJECT
public:
    CLeakageRelative() {
		mLeakageAddr = 255;
        airSerialHandle = nullptr;
	}
    // sysInit
    bool getResource()
    {
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

        if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&airSerialHandle))) {
            mErrString.append("get read interface pointer failed!!!");
            mLibSerial.unload();
            return false;
        }

        if (airSerialHandle) {
            mErrString.clear();
            return true;
        }

        mErrString.append("unknow error");
        mLibSerial.unload();
        return false;
    }

    void freeResource()
    {
        if (airSerialHandle) {
            airSerialHandle->linkRelease();
            airSerialHandle = nullptr;
        }

        mLibSerial.unload();
    }

    QString errorString()
    {
        return mErrString;
    }

    bool airInit(QString name, int bandrate = 9600)
    {
        if (!airSerialHandle->linkSerialInit(name.toStdString().c_str(), bandrate, 8, 1, 0, 1)) {
            char szTmp[1024] ={0};
            char* pszTmp = szTmp;
            airSerialHandle->linkGetLastError(pszTmp);
            qDebug() << "Init Serial Port Failed with the error infor: " << pszTmp;
            mErrString = pszTmp;
            return false;
        }
        airSerialHandle->linkSetFormat(FormatHex);
        mBoolHasOpened = true;
        return mBoolHasOpened;
    }

    void airDeInit()
    {
        if (!mBoolHasOpened) return;

        if (airSerialHandle) {
            airSerialHandle->linkDeInit();
        }
        mBoolHasOpened = false;
    }

    bool setAddress(int addr)
	{
		if (addr <= 0 || addr > 255) {
			return false;
		}
		mLeakageAddr = addr;
        return true;
	}

    bool airStart()
    {
        if (!mBoolHasOpened) return false;
        unsigned char cmd[8] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x05;
        cmd[2] = 0x00;
        cmd[3] = 0x01;
        cmd[4] = 0xFF;
        cmd[5] = 0x00;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[6] = crc & 0xFF;
        cmd[7] = (crc >> 8) & 0xFF;
        airSerialHandle->linkWrite((char*)cmd, 8);
        return airSerialHandle->linkFind((char*)cmd, 8);
	}
    bool airReset()
    {
        if (!mBoolHasOpened) return false;
        unsigned char cmd[8] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x05;
        cmd[2] = 0x00;
        cmd[3] = 0x00;
        cmd[4] = 0xFF;
        cmd[5] = 0x00;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[6] = crc & 0xFF;
        cmd[7] = (crc >> 8) & 0xFF;
        airSerialHandle->linkWrite((char*)cmd, 8);
        return airSerialHandle->linkFind((char*)cmd, 8);
    }

    bool airChangeProgram(int progId)
    {
        if (!mBoolHasOpened) return false;
        int prog = progId - 1;
        unsigned char cmd[11] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x10;
        cmd[2] = 0x02;
        cmd[3] = 0x00;
        cmd[4] = 0x00;
        cmd[5] = 0x01;
        cmd[6] = 0x02;
        cmd[7] = prog & 0xFF;
        cmd[8] = (prog >> 8) & 0xFF;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[9] = crc & 0xFF;
        cmd[10] = (crc >> 8) & 0xFF;
        airSerialHandle->linkWrite((char*)cmd, 11);
        unsigned char szReturn[128] = {0x00};
        char* psz = (char*)szReturn;
        int nlen = 0;
        if (!airSerialHandle->linkGet((char*)cmd, 2, nullptr, 8, psz, nlen)){
            return false;
        }

        if (szReturn[5] == 0x01) {
            return true;
        }

        return false;
    }

    template<typename T>
    bool airSetInputParam(ParamAddrInput addr, T data, int size, int cmpid = 0x02)
    {
        unsigned char* pdata = new unsigned char[size];
        memset(pdata, 0x00, size);

        for (int foo = 0; foo < size; foo++) {
            pdata[foo] = (data >> (foo * 0x08)) & 0xFF;
        }

        QByteArray baSend = formatString(0x10, addr, size, pdata);
        if (pdata) {
            delete []pdata;
        }
        qDebug() << "send: " << baSend;
        QByteArray baGet;
        if (sendAndRead(baSend, baGet, 5000)) {
            if (baGet.at(5) == cmpid) {
                return true;
            }
        }

        mErrString = "设置参数失败";
        return false;
    }

    bool airReadRealResult(ParamOutAir& param)
    {
        if (!mBoolHasOpened) return false;
        if (nullptr == airSerialHandle) {
            qDebug() << "serial is nullptr";
            return false;
        }
        unsigned char cmd[8] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x03;
        cmd[2] = 0x00;
        cmd[3] = 0x30;
        cmd[4] = 0x00;
        cmd[5] = 0x0D;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[6] = crc & 0xFF;
        cmd[7] = (crc >> 8) & 0xFF;

        qDebug() << "00";
        airSerialHandle->linkWrite((char*)cmd, 8);
        unsigned char szReturn[128] = {0x00};
        char* psz = (char*)szReturn;
        int nlen = 0;
        qDebug() << "01";
        if (!airSerialHandle->linkGet((char*)cmd, 2, nullptr, 31, psz, nlen)){
            return false;
        }

        param.AirProgId = szReturn[3] | (szReturn[4] << 8);
        param.AirTestMode = szReturn[5] | (szReturn[6] << 8);
        param.AirInstStatus = szReturn[7] | (szReturn[8] << 8);
        param.AirAlarmStatus = szReturn[9] | (szReturn[10] << 8);
        param.AirTestStep = szReturn[11] | (szReturn[12] << 8);
        param.AirPressure = szReturn[13] | (szReturn[14] << 8) | (szReturn[15] << 16) | (szReturn[16] << 24);
        param.AirPressureUnit = szReturn[17] | (szReturn[18] << 8) | (szReturn[19] << 16) | (szReturn[20] << 24);
        param.AirLeakage = szReturn[21] | (szReturn[22] << 8) | (szReturn[23] << 16) | (szReturn[24] << 24);
        param.AirLeakageUnit = szReturn[25] | (szReturn[26] << 8) | (szReturn[27] << 16) | (szReturn[28] << 24);

        qDebug() << "Program Id: " << param.AirProgId;
        qDebug() << "Test Mode: " << param.AirTestMode;
        qDebug() << "Inst Status: " << param.AirInstStatus;
        qDebug() << "Alarm Status: " << param.AirAlarmStatus;
        qDebug() << "Test Step: " << param.AirTestStep;
        qDebug() << "Test Pressure: " << param.AirPressure;
        qDebug() << "Test PressureUnit: " << param.AirPressureUnit;
        qDebug() << "Test Leakage: " << param.AirLeakage;
        qDebug() << "Test LeakageUnit: " << param.AirLeakageUnit;
        qDebug() << "02";
        return true;
    }


    bool airReadCycleStatus(unsigned char &step, unsigned char &status)
    {
        if (!mBoolHasOpened) return false;
        if (airSerialHandle == nullptr) {
            qDebug() << "serial nulltr";
            return false;
        }
        qDebug() << "03";
        unsigned char cmd[8] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x03;
        cmd[2] = 0x22;
        cmd[3] = 0x04;
        cmd[4] = 0x00;
        cmd[5] = 0x02;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[6] = crc & 0xFF;
        cmd[7] = (crc >> 8) & 0xFF;

        airSerialHandle->linkWrite((char*)cmd, 8);
        unsigned char szReturn[128] = {0x00};
        char* psz = (char*)szReturn;
        int nlen = 0;
        qDebug() << "04";
        if (!airSerialHandle->linkGet((char*)cmd, 2, nullptr, 9, psz, nlen)){
            return false;
        }

        qDebug() << "05";
        status = szReturn[3];
        step = szReturn[5];
        qDebug("get status: %02X, step: %02X");

        qDebug() << "06";
        return true;
    }

    bool airReadLastResult(ParamOutAir& param)
    {
        if (!mBoolHasOpened) return false;
        unsigned char cmd[8] = {0x00};
        cmd[0] = mLeakageAddr;
        cmd[1] = 0x03;
        cmd[2] = 0x00;
        cmd[3] = 0x11;
        cmd[4] = 0x00;
        cmd[5] = 0x0C;
        unsigned short crc =crc16_modbus(cmd, 6);
        cmd[6] = crc & 0xFF;
        cmd[7] = (crc >> 8) & 0xFF;

        airSerialHandle->linkWrite((char*)cmd, 8);
        unsigned char szReturn[128] = {0x00};
        char* psz = (char*)szReturn;
        int nlen = 0;
        if (!airSerialHandle->linkGet((char*)cmd, 2, nullptr, 29, psz, nlen)){
            return false;
        }

        param.AirProgId = szReturn[3] | (szReturn[4] << 8);
        param.AirTestMode = szReturn[5] | (szReturn[6] << 8);
        param.AirInstStatus = szReturn[7] | (szReturn[8] << 8);
        param.AirAlarmStatus = szReturn[9] | (szReturn[10] << 8);
        param.AirPressure = szReturn[11] | (szReturn[12] << 8) | (szReturn[13] << 16) | (szReturn[14] << 24);
        param.AirPressureUnit = szReturn[15] | (szReturn[16] << 8) | (szReturn[17] << 16) | (szReturn[18] << 24);
        param.AirLeakage = szReturn[19] | (szReturn[20] << 8) | (szReturn[21] << 16) | (szReturn[22] << 24);
        param.AirLeakageUnit = szReturn[23] | (szReturn[24] << 8) | (szReturn[25] << 16) | (szReturn[26] << 24);

        qDebug() << "Program Id: " << param.AirProgId;
        qDebug() << "Test Mode: " << param.AirTestMode;
        qDebug() << "Test Status: " << param.AirInstStatus;
        qDebug() << "Alarm Status: " << param.AirAlarmStatus;
        qDebug() << "Test Pressure: " << param.AirPressure ;
        qDebug() << "Test PressureUnit: " << param.AirPressureUnit;
        qDebug() << "Test Leakage: " << param.AirLeakage;
        qDebug() << "Test LeakageUnit: " << param.AirLeakageUnit;

        return true;
    }

private:
    bool sendAndRead(QByteArray w, QByteArray &f, int timeout = 5000)
    {
        QByteArray ba;
        ba.append(w.at(0));
        ba.append(w.at(1));
        airSerialHandle->linkWrite(w.data(), w.size());
        char ret[16] = {0};
        char* pRet = (char*)ret;
        int nlen = 0;
        if(!airSerialHandle->linkGet(ba.data(), ba.size(), nullptr, 8, pRet, nlen, timeout)) {
            mErrString = ("获取指令返回值失败");
            return false;
        }

        unsigned short crc = crc16_modbus((unsigned char*)ret, nlen - 2);
        unsigned char crc1 = crc & 0xff;
        unsigned char crc2 = (crc >> 8) & 0xff;
        bool brst = (crc1 == (unsigned char)ret[nlen - 2]);
        brst &= (crc2 == (unsigned char)ret[nlen - 1]);
        if (false == brst) {
            mErrString = ("返回数据CRC校验失败");
            return false;
        }

        f = QByteArray(pRet, nlen);
        return true;
    }

    QByteArray formatString(int way, unsigned short addr, unsigned char len, unsigned char* dataX)
    {
        AIR_STRUCT_HEADER data;
        data.id = mLeakageAddr;
        data.way = way;
        data.addr[1] = addr & 0xFF;
        data.addr[0] = (addr >> 8) & 0xFF;
        int dlen = len / 2;
        data.dlen[1] = dlen & 0xFF;
        data.dlen[0] = (dlen >> 8) & 0xFF;
        data.len = len;
        unsigned char* pdata = new unsigned char[len];
        memset (pdata, 0x00, len);
        memcpy(pdata, dataX, len);

        QByteArray ba = QByteArray((char*)&data, sizeof(data));
        ba.append((char*)pdata, len);
        unsigned short crc = crc16_modbus((unsigned char*)ba.data(), sizeof(data) + len);
        unsigned char szCrc[2] = {0};
        szCrc[0] = crc & 0xFF;
        szCrc[1] = (crc >> 8) & 0xFF;
        ba.append((char*)szCrc, 2);

        delete []pdata;
        return ba;
    }

private:
	int mLeakageAddr;
    IDataLinkTransport* airSerialHandle;
    QLibrary mLibSerial;
    bool mBoolHasOpened;
    QString mDllPath;
    QString mErrString;
};

#endif // CLEAKAGERELATIVE_H

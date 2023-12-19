#include "CSerialDirect.h"
#include <QFile>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

CSerialDirect::CSerialDirect() : m_handle(nullptr)
{
    getHandle();
}

CSerialDirect::~CSerialDirect()
{
    freeHandle();
}

bool CSerialDirect::getHandle()
{
    m_isOpened = false;

    QString dllPath = currentDirectoryPath() + "/";
    dllPath.append(DATALINKDLLNAME);

    if(!QFile::exists(dllPath)) {
        m_errString.append("dll file is not exist in ");
        m_errString.append(dllPath);
        return false;
    }

    m_lib.setFileName(dllPath);
    if (!m_lib.load()) {
        m_errString = m_lib.errorString();
        return false;
    }

    do {
        typedef bool (*OBJ)(char*, void**);
        OBJ getObj = (OBJ)m_lib.resolve("GetInterface");
        if (!getObj) {
            m_errString = "get interface 'GetInterface' failed!!!";
            break;
        }

        if(!getObj((char*)IID_Serial_Win32, reinterpret_cast<void**>(&m_handle))) {
            m_errString = "get IID_Serial_Win32 interface pointer failed!!!";
            break;
        }

        if (m_handle) {
            m_errString.clear();
            return true;
        }
    }while(0);

    m_lib.unload();
    return false;
}

void CSerialDirect::freeHandle()
{
    if (m_handle) {
        m_handle->linkRelease();
        m_handle = nullptr;
    }

    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}

QString CSerialDirect::errorString()
{
    return m_errString;
}

QString CSerialDirect::currentDirectoryPath()
{
    return QCoreApplication::applicationDirPath();
}

bool CSerialDirect::open(QString name, int bandrate, bool isHex)
{
    if (!m_handle) {
        m_errString = "nullptr handle! initialize the HDLC handle first";
        return false;
    }
    if (name.isEmpty()) {
        m_errString = "参数错误";
        return false;
    }
    if (!m_handle->linkSerialInit(name.toStdString().c_str(), bandrate)) {
        char szTmp[1024] ={0};
        char* pszTmp = szTmp;
        // m_handle->linkGetLastError(pszTmp);
        m_errString = "open error: ";
        m_errString.append(pszTmp);
        qDebug() << "开启串口失败 " << name;
        return false;
    }
    m_handle->linkSetFormat((isHex ? FormatHex : FormatString));

    qDebug() << "开启串口OK " << name;
    m_isOpened = true;
    return true;
}

void CSerialDirect::close()
{
    if (m_isOpened) {
        m_handle->linkDeInit();
    }
    m_isOpened = false;

    QThread::msleep(1000);
}

bool CSerialDirect::write(unsigned char cmd[], int len)
{
    if (!m_isOpened) return false;
    m_handle->linkClearBuffer();
    m_handle->linkWrite((char*)cmd, len);

    return true;
}

bool CSerialDirect::find(unsigned char cmd[], int len, int timeout)
{
    if (!m_isOpened) return false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();

        if (m_handle->linkFind((char*)cmd, len, 500)) {
            return true;
        }

        QThread::msleep(50);
    }

    return false;
}

bool CSerialDirect::get(char *start, int slen, char *end, int elen, char *&pRst, int &len, int timeout)
{
    if (!m_isOpened) return false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();

        if (m_handle->linkGet(start, slen, end, elen, pRst, len, 500)) {
            return true;
        }

        QThread::msleep(50);
    }
}

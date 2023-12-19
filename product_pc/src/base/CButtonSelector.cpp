#include "CButtonSelector.h"
#include <QFile>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>

CButtonSelector::CButtonSelector()
{
//    if (!getHandle()) {
//        qDebug() <<
//        //QMessageBox::information(this, "错误", "<font size='26' color='red'>初始化按钮选择器错误</font>");
//    }
}

CButtonSelector::~CButtonSelector()
{
    //freeHandle();
}

bool CButtonSelector::getHandle()
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

void CButtonSelector::freeHandle()
{
    if (m_handle) {
        m_handle->linkRelease();
        m_handle = nullptr;
    }

    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}

QString CButtonSelector::errorString()
{
    return m_errString;
}

QString CButtonSelector::currentDirectoryPath()
{
    return QCoreApplication::applicationDirPath();
}

bool CButtonSelector::open(QString name, int bandrate)
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
        return false;
    }
    m_handle->linkSetFormat(FormatString);

    m_isOpened = true;
    return true;
}

void CButtonSelector::close()
{
    if (m_isOpened) {
        m_handle->linkDeInit();
    }
    m_isOpened = false;
    QThread::msleep(600);
}

bool CButtonSelector::isOK(int timeout)
{
    m_handle->linkClearBuffer();
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();

        if (m_handle->linkFind("OK", strlen("OK"), 500)) {
            return true;
        }

        QThread::msleep(50);
    }

    return false;
}

bool CButtonSelector::isNG(int timeout)
{
    m_handle->linkClearBuffer();
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();

        if (m_handle->linkFind("NG", strlen("NG"), 500)) {
            return true;
        }

        QThread::msleep(50);
    }

    return false;
}

BS_Status CButtonSelector::getStatus(int timeout)
{
    qDebug() << __FUNCTION__ << __threadid();
    m_handle->linkClearBuffer();
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        QCoreApplication::processEvents();

        if (m_handle->linkFind("NG", strlen("NG"), 500)) {
            return BS_StatusNG;
        } else if (m_handle->linkFind("OK", strlen("OK"), 500)) {
            return BS_StatusOK;
        }

        QThread::msleep(10);
    }

    return BS_StatusNone;
}


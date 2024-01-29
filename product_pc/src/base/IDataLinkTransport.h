#ifndef  _INTERFACE_DATALINK_TRANSPORT_H_
#define _INTERFACE_DATALINK_TRANSPORT_H_
#include "common_struct.h"

#define interface struct

const char IID_Serial[] = "Interface.DataLink.Serial";
const char IID_Serial_Win32[] = "Interface.DataLink.Serial.Win32";
const char IID_Socket[] = "Interface.DataLink.Socket";
const char IID_WebSocket[] = "Interface.DataLink.WebSocket";
#define DATALINKDLLNAME "DataLinkTransportV3.dll"

interface IDataLinkTransport
{
    virtual bool linkSocketInit(const char* pLocal, const char* pName, int nPort) = 0;
    /// @brief: linSerialInit, serial port open ...
    /// @param: pName, serial port name, such as 'COM1'
    /// @param: nBandrate, like 115200, 9600, 1500000
    /// @param: nDataBits, like 5,6,7,8
    /// @param: nStopBits, like 1 - oneBits, 2 - oneHalfBits, 3 - twoBits
    /// @param: nFlowCtrl, 0 - no control, 1 - hardware control, 2 - software control
    /// @param: nParity, 0 - no parity, 1 - even, 2 - odd
    virtual bool linkSerialInit(const char* pName, int nBandrate = 115200, int nDataBits = 8, int nStopBits = 1, int nFlowCtrl = 0, int nParity = 0) = 0;
    virtual void linkDeInit() = 0;
    virtual void linkSetFormat(DataStreamFormat format = FormatString) = 0;
	
	// common interface
    virtual void linkWrite(const char* pData, int iLen) = 0;
    virtual bool linkFind(const char* pFind, int iLen, int nTimeout = 5000) = 0;
    virtual bool linkGet(const char* pStart, int iStart, const char* pEnd, int iEnd, char* &pData, int &iretlen, int nTimeout = 5000) = 0;

    virtual void linkClearBuffer() = 0;
    virtual void linkRelease() = 0;

    virtual void linkGetLastError(char* & pError) = 0;
};

// 加载组件举例
/*
#include <QLibrary>
IDataLinkTransport* m_linkTransport = nullptr;
QLibrary m_libTransport;
bool load()
{
    QString path = QCoreApplication::applicationDirPath() + "/";
    path += DATALINKDLLNAME;
    m_libTransport.setFileName(path);
    if (m_libTransport.load()) {

        typedef bool (*GETINTERFACE)(char*, void**);

        GETINTERFACE getInterface = (GETINTERFACE)m_libTransport.resolve("GetInterface");
        if (getInterface) {
            if (getInterface((char*)IID_Serial_Win32, (void**)&m_linkTransport)) {
                return true;
            }
        }
    } else {
        qDebug() << m_libTransport.errorString();
    }

    return false;
}

void unLoad()
{
    if (m_linkTransport) {
        m_linkTransport->linkRelease();
    }
    m_linkTransport = nullptr;

    m_libTransport.unload();
}
*/
#endif //_INTERFACE_DATALINK_TRANSPORT_H_

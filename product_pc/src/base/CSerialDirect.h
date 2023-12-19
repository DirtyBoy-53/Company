#ifndef CSERIALDIRECT_H
#define CSERIALDIRECT_H

#include <QObject>
#include <QLibrary>
#include "IDataLinkTransport.h"

class CSerialDirect
{
public:
    CSerialDirect();
    ~CSerialDirect();
    // sysInit
    bool getHandle();
    void freeHandle();
    QString errorString();
    QString currentDirectoryPath();

    bool open(QString name, int bandrate = 9600, bool isHex = false);
    void close();

    bool write(unsigned char cmd[], int len);
    bool find(unsigned char cmd[], int len, int timeout);
    bool get(char *start, int slen, char* end, int elen, char* &pRst, int &len, int timeout = 5000);

    bool isOpened() {
        return m_isOpened;
    }
protected:
    bool m_isOpened;

private:
    IDataLinkTransport* m_handle;
    QString m_errString;
    QLibrary m_lib;

};

#endif // CSERIALDIRECT_H

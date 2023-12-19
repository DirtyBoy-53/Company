#ifndef CBUTTONSELECTOR_H
#define CBUTTONSELECTOR_H

#include <QWidget>
#include <QLibrary>
#include "IDataLinkTransport.h"

typedef enum {
    BS_StatusNone = -1,
    BS_StatusOK = 0,
    BS_StatusNG
}BS_Status;

class CButtonSelector
{
public:
    CButtonSelector();
    ~CButtonSelector();

    bool getHandle();
    void freeHandle();
    QString errorString();
    QString currentDirectoryPath();

    bool open(QString name, int bandrate = 9600);
    void close();

    bool isOK(int timeout = 3600000);
    bool isNG(int timeout = 3600000);
    BS_Status getStatus(int timeout = 3600000);

private:
    IDataLinkTransport* m_handle;
    QString  m_errString;
    QLibrary m_lib;

    bool m_isOpened;
};

#endif // CBUTTONSELECTOR_H

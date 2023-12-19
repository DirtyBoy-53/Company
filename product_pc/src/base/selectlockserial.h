#ifndef SELECTLOCKSERIAL_H
#define SELECTLOCKSERIAL_H

#include "cserialport.h"

class SelectLockSerial
{
public:
    SelectLockSerial();
    int openSelectLockSerial(const QString& com);
    void closeSelectLockSerial();
    int checkLock();

private:
     CSerialPort* m_seleckSerial;
};

#endif // SELECTLOCKSERIAL_H

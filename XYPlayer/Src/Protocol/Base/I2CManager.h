#ifndef IICPROTOCOL_H
#define IICPROTOCOL_H
#include <iostream>
#include <QObject>

class I2CManager
{
public:
    I2CManager();
    bool send(int ch, int regAddr, const uint16_t value, int devAddr=0x66);
    QByteArray read(int ch, int regAddr, uint16_t size, int devAddr=0x67);

    bool readIspParam(int ch, int ispId, int totalLen, QByteArray &data);
    bool writeIspParam(int ch, int ispId, QByteArray data);
};

#endif // IICPROTOCOL_H

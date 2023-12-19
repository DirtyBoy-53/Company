#ifndef IOSERIALMANAGER_H
#define IOSERIALMANAGER_H

#include <QObject>
#include "cserialport.h"

enum IoSerialState {
    IO_SERIAL_STATE_INVALID = -1,
    IO_SERIAL_STATE_OPEN = 0,
    IO_SERIAL_STATE_CLOSE = 1, // 盖板到位
    IO_SERIAL_STATE_NO_PRODUCE = 2,
    IO_SERIAL_STATE_ERROR = 3,
    IO_SERIAL_STATE_LEFT = 4,
    IO_SERIAL_STATE_RIGHT = 5,
    IO_SERIAL_STATE_FRONT = 6,
    IO_SERIAL_STATE_BACK = 7,
    IO_SERIAL_STATE_PRODUCT_ON = 8, // 盖板到位
};

enum OperaterStatusID{
    OID_GetStatusIN1 = -11,
    OID_GetStatusIN2 = -12,
    OID_GetStatusIN3 = -13,
    OID_GetStatusIN4 = -14,
    OID_GetStatusOUT1 = -1,
    OID_GetStatusOUT4 = -4,
    OID_Out1 = 1, // 拉高或者拉低OUT1
    OID_Out4 = 4, // 拉高或者拉低OUT4
};

class IoSerialManager : public QObject
{
    Q_OBJECT
public:
    IoSerialManager();
    ~IoSerialManager();

    bool openSerial(const QString& com);
    void closeSerial();
    bool getIoState();

signals:
    void signalUpdateIoState(const int state);

public slots:
    void slotRecvSerialData(QByteArray data);
    void getIoDirection();
    void slotOperate(int code, bool status);

private:
    void initSerial();

    CSerialPort* m_serial;
    QByteArray m_cache;
    int m_state = IO_SERIAL_STATE_CLOSE;
};

#endif // IOSERIALMANAGER_H

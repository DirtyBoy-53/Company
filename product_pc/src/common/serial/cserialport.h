#ifndef CSERIALPORT_H
#define CSERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <comdll.h>
#include <QThread>
#include <QMutex>

struct  SerialParam
{
    int baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::FlowControl flowControl;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
};

class Q_API_FUNCTION CSerialPort : public QObject
{
    Q_OBJECT
public:
    CSerialPort();
    ~CSerialPort();

public slots:
    bool openSerial(const QString& serialName, const SerialParam& param, bool isSync = false);
    void closeSerial();
    int sendCommand(unsigned char* data, const int len);
    int sendCommandWait(unsigned char* data, const int len);

    QByteArray sendWaitForResp(unsigned char *data, const int len, const int respLen);
    QByteArray sendWaitForRespEx(unsigned char *data, const int len, QByteArray header, const int respLen);
    QByteArray readSerial(const int timeout = 6000);
    QByteArray readSerialHold(const int timeout = 1000, int needLen = -1);
    bool findSerial(QByteArray find, int timeout = 2000);
    QByteArray read(int len);

signals:
    void signalOpenResult(const bool result);
    void signalReadSerialData(QByteArray data);

private:
    void onReadData();
    QByteArray m_readCache;
    QSerialPort* m_serialPort = nullptr;
};

#endif // CSERIALPORT_H

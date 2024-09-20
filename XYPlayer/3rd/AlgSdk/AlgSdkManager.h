#ifndef ALGSDKMANAGER_H
#define ALGSDKMANAGER_H

#include <QObject>
#include <exception>
#include <thread>
#include <QProcess>

#include "Singleton.hpp"
#include "YStruct.h"
QT_FORWARD_DECLARE_CLASS(Window)

class CAlgSdkException : public std::exception
{
public:
    CAlgSdkException(const std::string &msg) : m_msg(msg.c_str()) {}
    const char* what() const throw() { return m_msg.c_str(); }

private:
    std::string m_msg;
};

struct FrameRecord{
    int count = 0;
    uint64_t tm = 0;
    int fps = 0;
};

class AlgSdkManager : public QObject, public Singleton<AlgSdkManager>
{
    Q_OBJECT
public:
    AlgSdkManager();
    bool open();
    bool init();
    void close();
    Window *window() const;
    void setWindow(Window *window);
    bool writeCameraConf(QString conf, int group, int channel);

    bool writeReg(uint8_t board_id, uint8_t ch_id, uint8_t i2c_add, uint16_t reg_addr, uint8_t value, uint16_t type);
private:
    Window *m_window{nullptr};
    std::thread*    m_thread_open{nullptr};
    std::thread*    m_thread_init{nullptr};
    QProcess        m_process;
    bool            m_closeFlag{false};
};

class ConsoleControl : public QObject{
    Q_OBJECT
public:
    ConsoleControl(QObject *obj=nullptr);
    ~ConsoleControl();

    bool writeCameraConf(QString conf, int group, int channel);
private:
    QProcess*           m_process{nullptr};
    QString             m_msg;
};


class IICControl{
public:
    IICControl() = default;
    // ch_id(通道号): 0-15, i2c_add: 0x****, reg_addr: 0x****,  type: 0x1616,  board_id(表示级联数量): 0-1,
    bool writeRegByIIC( uint8_t ch_id, uint16_t reg_addr, uint16_t value, uint16_t i2c_add = 0x66, uint16_t type = 0x1616, uint8_t board_id = 0);
    bool write( uint8_t ch_id, uint16_t reg_addr, uint16_t value, uint16_t type = 0x1616, uint16_t i2c_add = 0x66, uint8_t board_id = 0);
    // ch_id(通道号): 0-15, i2c_add: 0x****, reg_addr: 0x****,  type: XP05:0x1616 other:0x1632,  board_id(表示级联数量): 0-1,
    readReg_S readRegByIIC(uint8_t ch_id,  uint16_t reg_addr, uint16_t size = 4, uint16_t i2c_add = 0x67, uint16_t type = 0x1632, uint8_t board_id = 0);
};
#endif // ALGSDKMANAGER_H

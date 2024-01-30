#ifndef INTERACTIONBYPCIE_H
#define INTERACTIONBYPCIE_H

#include <iostream>
#include <QProcess>
#include <QString>
#include "singleton.h"
class CInteractionByPcie:public Singleton<CInteractionByPcie>
{
public:
    CInteractionByPcie();
    bool writeReg(uint8_t board_id, uint8_t ch_id, uint8_t i2c_add, uint16_t reg_addr, uint8_t value, uint16_t type);
    bool readReg(uint8_t board_id, uint8_t ch_id, uint16_t i2c_add, uint16_t reg_addr, uint16_t type, unsigned char check);
    void sendMsgByConsole(QString curText,int group,int channel);
private:
    QProcess m_pythonProcess;
};

#endif // CInteractionByPcie

#include "plccommandformat.h"
#include <string>
#include <QDebug>
#include "crc16_modbus.h"


PLCCommandFormat::PLCCommandFormat()
{

}

unsigned char *PLCCommandFormat::formatCommand(unsigned short addr, unsigned short value, unsigned char* & cmdret, int &cmdlen)
{
    unsigned char cmd[CMDLEN] = {0x00};
    memset (cmd, 0x00, CMDLEN);

    cmd[0] = 0x01;
    cmd[1] = 0x05;
    cmd[2] = (addr >> 8) & 0x00FF;
    cmd[3] = addr & 0x00FF;
    cmd[4] = (value >> 8) & 0x00FF;
    cmd[5] = value & 0x00FF;

    unsigned short crc = crc16_modbus(cmd, 6);
    qDebug("Calculate Command CRC is: %X", crc);
    cmd[6] = crc & 0x00FF;
    cmd[7] = (crc >> 8) & 0x00FF;

    memcpy(cmdret, cmd, CMDLEN);
    cmdlen = CMDLEN;

    return cmd;
}

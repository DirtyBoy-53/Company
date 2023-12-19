#include "crc16_modbus.h"
#include <QDebug>


/*
* 函数名 :CRC16
* 描述 : 计算CRC16
* 输入 : puchMsg---数据地址,usDataLen---数据长度
* 输出 : 校验值
*/
unsigned short crc16_modbus(unsigned char *puchMsg, unsigned char usDataLen)
{
    unsigned short uCRC = 0xffff;//CRC寄存器

    for(unsigned char num = 0; num < usDataLen; num++){
        uCRC = (*puchMsg++) ^ uCRC;//把数据与16位的CRC寄存器的低8位相异或，结果存放于CRC寄存器。
        for (unsigned char x = 0; x < 8; x++) {	//循环8次
            if (uCRC & 0x0001) {	//判断最低位为：“1”
                uCRC = uCRC >> 1;	//先右移
                uCRC = uCRC ^ 0xA001;	//再与0xA001异或
            } else {	//判断最低位为：“0”
                uCRC = uCRC >> 1;	//右移
            }
        }
    }

    qDebug() << uCRC;
    return uCRC;//返回CRC校验值
}

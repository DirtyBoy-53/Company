#ifndef CRC16_MODBUS_H
#define CRC16_MODBUS_H

/*
* 函数名 :CRC16
* 描述 : 计算CRC16
* 输入 : puchMsg---数据地址,usDataLen---数据长度
* 输出 : 校验值
*/
unsigned short crc16_modbus(unsigned char *puchMsg, unsigned char usDataLen);


#endif // CRC16_MODBUS_H

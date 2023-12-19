#ifndef PLCCOMMANDFORMAT_H
#define PLCCOMMANDFORMAT_H

#define CMDLEN 8

const unsigned short POSITION_45CM_0     = 0x2550;
const unsigned short POSITION_45CM_15N = 0x2552;
const unsigned short POSITION_45CM_30N = 0x2554;
const unsigned short POSITION_45CM_15P = 0x2556;
const unsigned short POSITION_45CM_30P = 0x2558;

const unsigned short POSITION_50CM_0     = 0x255A;
const unsigned short POSITION_50CM_15N = 0x255C;
const unsigned short POSITION_50CM_30N = 0x255E;
const unsigned short POSITION_50CM_15P = 0x2560;
const unsigned short POSITION_50CM_30P = 0x2562;

const unsigned short POSITION_60CM_0     = 0x2938;
const unsigned short POSITION_60CM_15N = 0x293A;
const unsigned short POSITION_60CM_30N = 0x293C;
const unsigned short POSITION_60CM_15P = 0x293E;
const unsigned short POSITION_60CM_30P = 0x2940;

const unsigned short POSITION_70CM_0     = 0x2942;
const unsigned short POSITION_70CM_15N = 0x2944;
const unsigned short POSITION_70CM_30N = 0x2946;
const unsigned short POSITION_70CM_15P = 0x2948;
const unsigned short POSITION_70CM_30P = 0x294A;

const unsigned short POSITION_80CM_0     = 0x3CC0;
const unsigned short POSITION_80CM_15N = 0x3CC2;
const unsigned short POSITION_80CM_30N = 0x3CC4;
const unsigned short POSITION_80CM_15P = 0x3CC6;
const unsigned short POSITION_80CM_30P = 0x3CC8;

const unsigned short POSITION_90CM_0     = 0x3CCA;
const unsigned short POSITION_90CM_15N = 0x3CCC;
const unsigned short POSITION_90CM_30N = 0x3CCE;
const unsigned short POSITION_90CM_15P = 0x3CD0;
const unsigned short POSITION_90CM_30P = 0x3CD2;

const unsigned short POSITION_100CM_0     = 0x2456;
const unsigned short POSITION_100CM_15N = 0x2457;
const unsigned short POSITION_100CM_30N = 0x2458;
const unsigned short POSITION_100CM_15P = 0x2459;
const unsigned short POSITION_100CM_30P = 0x245A;


// 校K
const unsigned short POSITION_CALK_ORIGIN = 0x248C; // 上下料位
const unsigned short POSITION_CALK_POS1 = 0x2488; // POS1 20°K位置
const unsigned short POSITION_CALK_POS2 = 0x248A; // POS2 60°K位置
const unsigned short POSITION_CALK_POS1T = 0x248E; // POS1贴合点位
const unsigned short POSITION_CALK_POS2T = 0x2490; // POS1贴合点位

const unsigned short POSITION_RAISE = 0x20C9; // 托盘夹具90度上抬
const unsigned short POSITION_PLAIN = 0x20CA; // 托盘夹具放平
const unsigned short POSITION_FIXTUREUP = 0x20CC; // TEC分离气缸抬起
const unsigned short POSITION_FIXTUREDOWN = 0x20CD; // TEC分离气缸降落

const unsigned short PLC_LOCKSTATUS = 0x20D2; // 盖子锁定状态
const unsigned short PLC_WORKING = 0x2384; // 点亮绿灯

const unsigned short PLC_RESET = 0x205B;
const unsigned short PLC_STOP = 0x203D;

// 内参
const unsigned short PLC_ORIGIN = 0x23E8;
const unsigned short PLC_UPDOWN = 0x20CE;

const unsigned short POWERUP = 0x30FE;

const unsigned short BOOL_TRUE = 0xFF00;
const unsigned short BOOL_FALSE = 0x0000;
const unsigned short VALUE_0 = 0x0000;
const unsigned short VALUE_1 = 0x0001;

const unsigned short PRODUCT_XD01A  = 0x2D16;
const unsigned short PLC_GONEXT = 0x1450; // VALUE1;

const unsigned short ACK_GOPOS_OK = 0x02;
const unsigned short ACK_DEV_ERR = 0x03;
const unsigned short ACK_RESET_OK = 0x06;
const unsigned short ACK_RAISE_OK = 0x07;
const unsigned short ACK_LOWER_OK = 0x08;
const unsigned short ACK_UP_OK = 0x09;
const unsigned short ACK_DOWN_OK = 0x10;
const unsigned short ACK_LOCK_OK = 0x11;
const unsigned short ACK_UNLOCK_OK = 0x12;

class PLCCommandFormat
{
public:
    PLCCommandFormat();

    static unsigned char * formatCommand(unsigned short addr, unsigned short value, unsigned char* & cmdret, int &cmdlen);
};

#endif // PLCCOMMANDFORMAT_H

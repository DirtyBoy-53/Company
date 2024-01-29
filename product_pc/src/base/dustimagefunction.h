#ifndef DUSTIMAGEFUNCTION_H
#define DUSTIMAGEFUNCTION_H

#include "CSerialDirect.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

//状态读取
#define D800_START          (0x01<<0)   //启动
#define D800_STOP           (0x01<<1)   //停止
#define D800_RESET          (0x01<<2)   //复位
#define D800_URGENT_STOP    (0x01<<3)   //急停
#define D800_NG_RESET       (0x01<<4)   //NG复位按钮
#define D800_PRODUCT_DEC    (0x01<<5)   //产品有无检测
#define D800_MOLD_FORWARD   (0x01<<6)   //模具前检测
#define D800_MOLD_BACK      (0x01<<7)   //模具后检测

#define D801_CLAMPING_ORIGINAL  (0x01<<0)   //锁模气缸原位
#define D801_CLAMPING_WORK      (0x01<<1)   //锁模气缸工作位
#define D801_FLIP_ORIGINAL      (0x01<<2)   //翻转气缸原位
#define D801_FLIP_WORK          (0x01<<3)   //翻转气缸工作位
#define D801_ROTATE_ORIGINAL    (0x01<<4)   //旋转气缸原位
#define D801_ROTATE_WORK        (0x01<<5)   //旋转气缸工作位
#define D801_CLAMPING_START     (0x01<<6)   //锁模开启按钮

#define D802_CLAMPING_VALVE     (0x01<<0)   //锁模电磁阀
#define D802_FLIP_VALVE         (0x01<<1)   //翻转电磁阀
#define D802_ROTATE_VALVE       (0x01<<2)   //旋转电磁阀
#define D802_OK_LIGHT           (0x01<<3)   //OK指示灯
#define D802_NG_LIGHT           (0x01<<4)   //NG指示灯

#define D803_STATUS             (0x01<<0)   //工装连接状态


//控制输出
#define BASE_ADDR       (0x0320)            //控制寄存器基地址
#define READ_ADDR       (BASE_ADDR)         //状态读取
#define CLAMPING_ADDR   (BASE_ADDR+0xA)       //锁模
#define FLIP_ADDR       (BASE_ADDR+0xB)       //翻转
#define ROTATE_ADDR     (BASE_ADDR+0xC)       //旋转
#define GREEN_ADDR      (BASE_ADDR+0xD)       //绿灯
#define RED_ADDR        (BASE_ADDR+0xE)       //红灯
#define STATUS_ADDR     (BASE_ADDR+0xF)       //状态-连接/断开

//D803  联机反馈
namespace DustImageFunctionName {

enum data_e{ ON=0x01, OFF=0x00, READ=0x04};

enum register_e{ D800, D801, D802, D803};

enum fun_num_e{
    read_coil=0x01,
    read_input=0x02,
    read_keep_reg=0x03,
    read_input_reg=0x04,
    write_singl_coil=0x05,
    write_singl_reg=0x06,
    write_mul_coil=0x0F,
    write_mul_reg=0x10,
};
}
using namespace DustImageFunctionName;
class DustImageFunction : public CSerialDirect
{
public:

    DustImageFunction();
    bool clamping_control(data_e data);     //锁模开启/关闭控制
    bool flip_control(data_e data);         //气缸翻转/回翻控制
    bool rotate_control(data_e data);       //气缸旋转/回旋控制
    bool green_control(data_e data);        //绿灯开启/关闭控制
    bool red_control(data_e data);          //红灯开启/关闭控制
    bool status_control(data_e data);       //工装连机/脱机控制

    bool is_status(const register_e &reg,int addr, int timeout = 1500);
    bool is_not_status(const register_e &reg,int addr, int timeout = 1500);
    int32_t read_status(const register_e &reg, int timeout = 1500);

private:
    void print_hex(unsigned char *p, int len);
    uint32_t crc16_modbus_rtu(uint8_t *buf,int len);
    bool get_buf(uint8_t *buf,int bLen,uint32_t addr,data_e data,fun_num_e funNum);
    QString register_to_string(const register_e reg) const;
};
#endif // DUSTIMAGEFUNCTION_H

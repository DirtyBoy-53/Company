#ifndef LEAKAGEPLC_H
#define LEAKAGEPLC_H

#include "CSerialDirect.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

#define D30_LEAKAGE_CHECK_OK            (0x01)      //气检OK
#define D30_LEAKAGE_CHECK_NG            (0x01 << 1) //气检NG
#define D30_TURNTABLE_MOVE_CHECK    (0x01 << 2)     //解锁动点检测
#define D30_TURNTABLE_ORIGIN_CHECK  (0x01 << 3)     //解锁原点检测
#define D30_CHASSIS_RISE            (0x01 << 4)     //定位上升检测
#define D30_CHASSIS_FALL            (0x01 << 5)     //定位下降检测
#define D30_TOPPING_FALL            (0x01 << 6)     //下压检测
#define D30_TOPPING_RISE            (0x01 << 7)     //上抬检测
#define D30_TURNTABLE_ONPOS         (0x01 << 8)     //转盘到位检测
#define D30_DUT_ISON                (0x01 << 9)     //产品检测
#define D30_DUT_ISRIGHT             (0x01 << 10)    //产品防错
#define D30_FAILDUT_CHECK           (0x01 << 11)    // 不良品盒在位信号
#define D30_TURNTABLE_MOVE_CHECK2   (0x01 << 12)    //解锁动点检测2
#define D30_TURNTABLE_ORIGIN_CHECK2 (0x01 << 13)    //解锁原点检测2

#define D31_PLC_ISRUNNING (0x01)                    //设备自动运行中
#define D31_PLC_ISWARNING (0x02)                    //报警中
#define D31_PLC_PC_ONLINE (0x04)                    //在线/离线PC

/*
PC给PLC命令
命令                                           返回
01 06 00 28 00 02 88 03              01 06 00 28 00 02 88 03    解锁
01 06 00 28 00 01 C8 02              01 06 00 28 00 01 C8 02    上锁
01 06 00 28 00 04 08 01              01 06 00 28 00 04 08 01    定位上升
01 06 00 28 00 08 08 04              01 06 00 28 00 08 08 04    定位下降
01 06 00 28 00 10 08 0E              01 06 00 28 00 10 08 0E    检测下压
01 06 00 28 00 20 08 1A              01 06 00 28 00 20 08 1A    检测上抬
01 06 00 28 00 40 08 32              01 06 00 28 00 40 08 32    启动气检

PC读PLC状态
01 03 00 1E 00 02 A4 0D

注：读D30和D31地址值，符合8421规则，具体看WPS表格。
*/

class LeakagePLC : public CSerialDirect
{
public:
    LeakagePLC();

    bool plcTurntableUnlock() // 转盘解锁
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "转盘解锁";

        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x02, 0x88, 0x03};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }

        QThread::msleep(500);
        return isStatus(D30_TURNTABLE_MOVE_CHECK | D30_TURNTABLE_MOVE_CHECK2);
    }
    bool plcTurntableLock() // 转盘上锁
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "转盘上锁";

        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x01, 0xC8, 0x02};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }

        QThread::msleep(500);
        return isStatus(D30_TURNTABLE_ORIGIN_CHECK | D30_TURNTABLE_ORIGIN_CHECK2);
    }

    bool plcChassisRise() // 底盘上升
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "底盘上升";

        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x04, 0x08, 0x01};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }

        QThread::msleep(500);
        return isStatus(D30_CHASSIS_RISE, 2000);
    }
    bool plcChassisFall() // 底盘下降
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "底盘下降";
        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x08, 0x08, 0x04};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }

        QThread::msleep(500);
        return isStatus(D30_CHASSIS_FALL, 2000);
    }

    bool plcCappingFall() // 顶盖下降
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "顶盖下降";
        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x10, 0x08, 0x0E};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }
        QThread::msleep(1000);

        return isStatus(D30_TOPPING_FALL, 8000);
    }
    bool plcCappingRise() // 顶盖上升
    {
        if (!m_isOpened) {
            return false;
        }

        qDebug() << "顶盖上升";
        int len = 8;
        uchar cmd[] = {0x01, 0x06, 0x00, 0x28, 0x00, 0x20, 0x08, 0x1A};
        write(cmd, len);
        if (!find(cmd, len, 2000)) {
            return false;
        }

        QThread::msleep(1000);
        return isStatus(D30_TOPPING_RISE, 8000);
    }

    bool plcTurnTableLockedAndOnPos() // 产品转到位
    {
        qDebug() << "转盘到位检测，以及2个原点检测";
        return isStatus(D30_TURNTABLE_ORIGIN_CHECK | D30_TURNTABLE_ORIGIN_CHECK2 | D30_TURNTABLE_ONPOS, 200);
    }

    bool plcDutOnPos() // 产品放到位
    {
        qDebug() << "产品放置和产品正确性检测";
        //return isStatus(D30_DUT_ISON | D30_DUT_ISRIGHT);
        return isStatus(D30_DUT_ISON, 200);
    }

    bool plcTurnTableOnPos(bool isCheckOn = true)
    {
        qDebug() << "转盘到位检测";
        if (isCheckOn) {
            return isStatus(D30_TURNTABLE_ONPOS, 100);
        }

        return isNotStatus(D30_TURNTABLE_ONPOS, 100);
    }


    bool plcNGBoxPullOut()
    {
        qDebug() << "NG盒子在位检测";
        return isNotStatus(D30_FAILDUT_CHECK, 100);
    }


private:
    bool isStatus(ushort status, int timeout = 1500)
    {
        if (!m_isOpened) {
            return false;
        }
        int len = 8;
        uchar cmd[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x02, 0xA4, 0x0D};

        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < timeout) {
            QCoreApplication::processEvents();
            write(cmd, len);

            char retRst[32] = {0x00};
            char* pRst = (char*)retRst;
            int requreLen = 2 + 1 + 4 + 2; // 2字节头部 + 1字节长度 + 4字节数据 + 2字节CRC
            if (!get((char*)cmd, 2, nullptr, requreLen, pRst, len)) {
                qDebug() << ">> 状态获取返回数据失败";
                continue;
            }

            if (retRst[2] != 0x04) {
                qDebug() << ">> 状态获取返回数据长度不对";
                continue;
            }

            ushort d30 = ((pRst[3] << 8) & 0xFF00) | (pRst[4] & 0x00FF);
            ushort d31 = ((pRst[5] << 8) & 0xFF00) | (pRst[6] & 0x00FF);
            qDebug("D30: %x, D31: %x", d30, d31);
            if ((d31 & D31_PLC_ISWARNING) == D31_PLC_ISWARNING) {
                qDebug() << ">> D31 返回报警信息";
                return false;
            }

            if ((d30 & status) == status) {
                qDebug() << "比对 完成";
                return true;
            }
            qDebug() << ">> D30 返回信息不匹配";
            qDebug("need status: %02x",status);
            // QThread::msleep(500);
        }
        return false;
    }


    bool isNotStatus(ushort status, int timeout = 1500)
    {
        if (!m_isOpened) {
            return false;
        }
        int len = 8;
        uchar cmd[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x02, 0xA4, 0x0D};

        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < timeout) {
            QCoreApplication::processEvents();
            write(cmd, len);
            QThread::msleep(50);

            char retRst[32] = {0x00};
            char* pRst = (char*)retRst;
            int requreLen = 2 + 1 + 4 + 2; // 2字节头部 + 1字节长度 + 4字节数据 + 2字节CRC
            if (!get((char*)cmd, 2, nullptr, requreLen, pRst, len)) {
                qDebug() << ">> 状态获取返回数据失败";
                continue;
            }

            if (retRst[2] != 0x04) {
                qDebug() << ">> 状态获取返回数据长度不对";
                continue;
            }

            ushort d30 = ((pRst[3] << 8) & 0xFF00) | (pRst[4] & 0x00FF);
            ushort d31 = ((pRst[5] << 8) & 0xFF00) | (pRst[6] & 0x00FF);
            qDebug("D30: %x, D31: %x", d30, d31);
            if ((d31 & D31_PLC_ISWARNING) == D31_PLC_ISWARNING) {
                qDebug() << ">> D31 返回报警信息";
                return false;
            }

            if (((d30 ^ status) & status) == status) {
                qDebug() << "比对 完成";
                return true;
            }
            qDebug() << ">> D30 返回信息不匹配";
            qDebug("need status: %02x",status);
            // QThread::msleep(500);
        }
        return false;
    }
};

#endif // LEAKAGEPLC_H

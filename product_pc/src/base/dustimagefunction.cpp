#include "dustimagefunction.h"

DustImageFunction::DustImageFunction(){}
void DustImageFunction::print_hex(unsigned char* p, int len)
{

    QString strHex{""};
    for (int foo = 0; foo < len; foo++) {
        strHex += "0x" + QString::number(p[foo], 16);
        strHex += " ";
    }

    if (strHex.size() > 0)
        qDebug() << strHex;

}
bool DustImageFunction::clamping_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,CLAMPING_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取锁模/解锁buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取锁模/解锁buf返回失败");
        return false;
    }
    switch (data) {
    case data_e::ON :   return is_status(D801,D801_CLAMPING_WORK);          break;
    case data_e::OFF :  return is_not_status(D801,D801_CLAMPING_ORIGINAL);  break;
    default:                                                                break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::flip_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,FLIP_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取翻转/回翻buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取翻转/回翻buf返回失败");
        return false;
    }
    switch (data) {
    case data_e::ON :   return is_status(D801,D801_FLIP_WORK);              break;
    case data_e::OFF :  return is_not_status(D801,D801_FLIP_ORIGINAL);      break;
    default:                                                                break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::rotate_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,ROTATE_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取旋转/回旋buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取旋转/回旋buf返回失败");
        return false;
    }
    switch (data) {
    case data_e::ON :   return is_status(D801,D801_ROTATE_WORK);            break;
    case data_e::OFF :  return is_not_status(D801,D801_ROTATE_ORIGINAL);    break;
    default:                                                                break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::green_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,GREEN_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取开启/关闭绿灯buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取开启/关闭绿灯buf返回失败");
        return false;
    }
    switch (data) {
    case data_e::ON :   return is_status(D802,D802_OK_LIGHT);           break;
    case data_e::OFF :  return is_not_status(D802,D802_OK_LIGHT);       break;
    default:                                                            break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::red_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,RED_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取开启/关闭红灯buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取开启/关闭红灯buf返回失败");
        return false;
    }
    switch (data) {
    case data_e::ON :   return is_status(D802,D802_NG_LIGHT);           break;
    case data_e::OFF :  return is_not_status(D802,D802_NG_LIGHT);       break;
    default:                                                            break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::status_control(data_e data)
{
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,STATUS_ADDR,data,write_singl_reg);
    if(!ret) {
        qDebug("获取连/脱机buf失败");
        return false;
    }
    write(buf, BLEN);
    qDebug("发送控制buf:");
    print_hex(buf,BLEN);
    if(!find(buf, BLEN, 2000)) {
        qDebug("获取连/脱机buf返回失败");
        return false;
    }

    switch (data) {
        case data_e::ON :   return is_status(D803, D803_STATUS);         break;
        case data_e::OFF :  return is_not_status(D803,D803_STATUS);     break;
        default:                                                        break;
    }
    return false;
#undef BLEN
}

bool DustImageFunction::is_status(const register_e &reg, int addr, int timeout)
{
    int32_t value = read_status(reg,timeout);
    if(value == -1){
        qDebug() << "获取结果错误";
        return false;
    }
    if ((addr & value) == addr) {
        qDebug() << "比对 完成";
        return true;
    }
    qDebug() << ">> 返回信息不匹配";
    qDebug("need status: 0x%02x",addr);
    return false;
}

bool DustImageFunction::is_not_status(const register_e &reg, int addr, int timeout)
{
    int32_t value = read_status(reg,timeout);
    if(value == -1){
        qDebug() << "获取结果错误";
        return false;
    }
    if (((value ^ addr) & addr) == addr) {
        qDebug() << "比对 完成";
        return true;
    }
    qDebug() << ">> 返回信息不匹配";
    qDebug("need status: 0x%02x",addr);
    return false;
}

int32_t DustImageFunction::read_status(const register_e &reg,int timeout)
{
    if (!m_isOpened){
        qDebug("串口未开启");
        return -1;
    }
#define BLEN    8
    uchar buf[BLEN];
    bool ret = get_buf(buf,BLEN,READ_ADDR,READ,read_keep_reg);
    if(!ret) {
        qDebug("获取状态buf失败");
        return -1;
    }
    write(buf, BLEN);
    qDebug("发送状态查询buf:");
    print_hex(buf,BLEN);

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeout) {
        char retRst[32]{0};
        int retLen{0};
        char* pRst = (char*)retRst;
        int requreLen = 13; // 1字节ID + 1字节功能号 + 1字节数据长度 + 2字节D800数据 + 2字节D801数据 + 2字节D802数据 + 2字节D803数据 + 2字节CRC
        if (!get((char*)buf, 2, nullptr, requreLen, pRst, retLen,500)) {
            qDebug() << ">> 状态获取返回数据失败";
            continue;
        }

        if (retRst[2] != 0x08) {
            qDebug() << ">> 状态获取返回数据长度不对";
            continue;
        }
        qDebug() << ">> 返回状态数据:";
        print_hex((uint8_t*)pRst,retLen);
        int32_t value{-1};
        switch (reg) {
        case register_e::D800:
            value = ((pRst[3] << 8) & 0xFF00) | (pRst[4] & 0x00FF);
            break;
        case register_e::D801:
            value = ((pRst[5] << 8) & 0xFF00) | (pRst[6] & 0x00FF);
            break;
        case register_e::D802:
            value = ((pRst[7] << 8) & 0xFF00) | (pRst[8] & 0x00FF);
            break;
        case register_e::D803:
            value = ((pRst[9] << 8) & 0xFF00) | (pRst[10] & 0x00FF);
            break;
        default:
            value = -1;
            break;
        }
        QString str = QString("Register-%1:0x%2").arg(register_to_string(reg)).arg(value);
        qDebug() << str;
        return value;
    }
    qDebug() << ">> 未匹配到合适的数据";
    return -1;
#undef BLEN
}


uint32_t DustImageFunction::crc16_modbus_rtu(uint8_t *buf, int len)
{
    uint32_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint32_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else {
                crc >>= 1;
            }
        }
    }
    crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
    return crc;
}

bool DustImageFunction::get_buf(uint8_t *buf, int bLen, uint32_t addr, data_e data, fun_num_e funNum)
{
    int idx{0};
    buf[idx++] = 0x01;
    buf[idx++] = funNum;
    buf[idx++] = (uint8_t)((addr&0xFF00)>>8);
    buf[idx++] = (uint8_t)((addr&0x00FF));
    buf[idx++] = 0x00;
    buf[idx++] = data;
    uint32_t crc = crc16_modbus_rtu(buf,idx);
    buf[idx++] = (uint8_t)((crc&0xFF00)>>8);
    buf[idx++] = (uint8_t)((crc&0x00FF));

    if(bLen != bLen) return false;
    return true;
}

QString DustImageFunction::register_to_string(const register_e reg) const
{
    switch(reg){
    case register_e::D800: return "D800"; break;
    case register_e::D801: return "D801"; break;
    case register_e::D802: return "D802"; break;
    case register_e::D803: return "D803"; break;
    default : return QString(); break;
    }
}

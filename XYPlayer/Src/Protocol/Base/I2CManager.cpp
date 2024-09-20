#include "I2CManager.h"
#include <QDebug>
#include <QThread>

#include "AlgSdkManager.h"
#include "IICAddr.h"

I2CManager::I2CManager()
{

}

bool I2CManager::send(int ch, int regAddr, const uint16_t value, int devAddr)
{
    IICControl IICCtrl;
    return  IICCtrl.writeRegByIIC(ch, regAddr, value, devAddr);
}

QByteArray I2CManager::read(int ch, int regAddr, uint16_t size, int devAddr)
{
    IICControl IICCtrl;
    QByteArray data;

    readReg_S read_s = IICCtrl.readRegByIIC(ch, regAddr, size, devAddr);
    if(read_s.ret){
        for(auto i = 0;i < read_s.len;++i){
            data.append(read_s.value[i]);
        }
    }
    return data;
}

bool I2CManager::readIspParam(int ch, int ispId, int totalLen, QByteArray &data)
{
    qDebug("readIspParam ch:%d ispId:%d totalLen:%d", ch, ispId, totalLen);
    IICControl IICCtrl;
    bool ret{false};
    readReg_S read_s;
    ret = IICCtrl.writeRegByIIC(ch, SYSTEM_CONTROL_ALG_TYPE, ispId);
    QThread::msleep(500);
    if(!ret) return false;
    ret = IICCtrl.writeRegByIIC(ch, SYSTEM_CONTROL_SR_UNLOCK, 0x8012); //最高位：0:写 1:读 低8位：寄存器地址0x12 算法参数地址
    QThread::msleep(500);

    read_s = IICCtrl.readRegByIIC(ch, SPECIAL_CONFIG_ISP_ALG, totalLen);

    ret = IICCtrl.writeRegByIIC(ch, SYSTEM_CONTROL_SR_READ_END, 1);//读完成 回写1
    if(!ret) return false;
    for(auto i = 0;i < read_s.len;++i){
        data.append(read_s.value[i]);
    }
    qDebug() << data.toHex();
    return read_s.ret;
    
}

bool I2CManager::writeIspParam(int ch, int ispId, QByteArray data)
{
    IICControl IICCtrl;
    bool ret{false};
    uint16_t raw{0};
    ret = IICCtrl.write(ch, 0x12, ispId, data.size());
    if(!ret) return false;
    for(auto i = 0;i < data.size();i++){
        raw = (uint8_t)data.at(i);
        ret = IICCtrl.write(ch, 0x12, raw, data.size());
        if(!ret) return false;
    }
    return ret;
}

#ifndef ELECTRONICSCALE_H
#define ELECTRONICSCALE_H

#include <QObject>
#include "CSerialDirect.h"


// 说明：
// 电子秤的指令为，QP - 去皮，GL - 归零，QZ - 称重
// 而且称重只会获取到一次重量
class ElectronicScale : public CSerialDirect
{
public:
    ElectronicScale();
    ~ElectronicScale();
    bool getHeaveyQZ(float & heavy);
    bool setQP();
    bool setGL();
};

#endif // ELECTRONICSCALE_H

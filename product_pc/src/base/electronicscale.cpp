#include "electronicscale.h"

ElectronicScale::ElectronicScale()
{

}

ElectronicScale::~ElectronicScale()
{

}

bool ElectronicScale::getHeaveyQZ(float & heavy)
{
    if (!isOpened()) {
        return false;
    }

    write((unsigned char*)"QZ", 2);
    char szBuffer[32] = {0};
    char* pBuffer = (char*)szBuffer;
    int nBufLen = 0;
    bool ret = get((char*)"ST", 2, (char*)"kg", 2, pBuffer, nBufLen);
    // ST,NT,+  0.386kg
    if (ret && nBufLen > 10) {
        QString result = QString::fromLocal8Bit(pBuffer);
        QStringList list = result.split(",");
        if (list.size() != 3) {
            return false;
        }

        QString key = list.at(2);
        float sign = (key.at(0) == '+') ? 1 : -1;
        key.chop(2); // 去除kg字样
        key = key.mid(1); // 去除符号字样
        key.simplified(); // 去除左右空格

        heavy = key.toFloat() * sign;

        return true;
    }

    return false;
}

bool ElectronicScale::setQP()
{
    if (!isOpened()) {
        return false;
    }

    write((unsigned char*)"QP", 2);
    return true;
}

bool ElectronicScale::setGL()
{
    if (!isOpened()) {
        return false;
    }

    write((unsigned char*)"GL", 2);
    return true;
}

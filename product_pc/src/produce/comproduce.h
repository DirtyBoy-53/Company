#ifndef COMPRODUCE_H
#define COMPRODUCE_H

#include "baseproduce.h"

class ComProduce : public BaseProduce
{
public:
    ComProduce();

    void step0();
    void stepend();
    void checkNext();
    void getSnCode();

public:
    void getSn();

public slots:
    void slotStartWorkBtnClicked();
    void slotStopWorkBtnClicked();
    void slotConnectBtnClicked();
    void slotsStoped();

private:
    bool m_isFirst = true;
};

#endif // COMPRODUCE_H

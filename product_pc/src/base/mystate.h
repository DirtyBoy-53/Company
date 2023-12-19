#ifndef MYSTATE_H
#define MYSTATE_H

#include <QState>
#include <functional>

typedef std::function<void(void)> STATUSFUN;
class MyState : public QState
{
public:
    MyState();
    MyState(STATUSFUN func);

    void process();
private:
    STATUSFUN m_func = nullptr;
};

#endif // MYSTATE_H

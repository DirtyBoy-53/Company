#include "mystate.h"

MyState::MyState()
{

}

MyState::MyState(STATUSFUN func)
{
    m_func = func;
}

void MyState::process()
{
    if(m_func) {
        m_func();
    }
}

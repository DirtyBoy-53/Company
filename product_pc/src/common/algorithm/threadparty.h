#ifndef THREADPARTY_H
#define THREADPARTY_H
#include <singleton.h>

class ThreadParty:public Singleton<ThreadParty>
{
public:
    ThreadParty();
    void Evaluate_HorStripe(double * value1,
                            double * value2,
                            double * value3,
                            uint16_t * pus_src,
                            int n_width,
                            int n_height);
};

#endif // THREADPARTY_H

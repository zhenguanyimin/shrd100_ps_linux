#include "ThreadBase.h"

class SpectrumThread : public ThreadBase
{
    DECLARE_THREAD(SpectrumThread)
    DECLARE_TIMER1()
public:
protected:
    void _InitQueue(uint16_t& num) override;
    void _StartTimer() override;
private:
};

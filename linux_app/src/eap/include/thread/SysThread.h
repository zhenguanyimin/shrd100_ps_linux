#include "ThreadBase.h"

class SysThread : public ThreadBase
{
    DECLARE_THREAD(SysThread)
    DECLARE_TIMER1()
    DECLARE_TIMER2()
public:
protected:
    void _InitQueue(uint16_t& num) override;
    void _StartTimer() override;
private:
};

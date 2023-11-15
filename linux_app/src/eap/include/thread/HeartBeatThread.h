#include "ThreadBase.h"

class HeartBeatThread : public ThreadBase
{
    DECLARE_THREAD(HeartBeatThread)
    DECLARE_TIMER1()
    DECLARE_TIMER2()
    DECLARE_TIMER3()
public:
protected:
    void _InitQueue(uint16_t& num) override;
    void _StartTimer() override;
private:
};

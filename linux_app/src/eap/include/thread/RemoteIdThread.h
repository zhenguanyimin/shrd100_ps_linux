#include "ThreadBase.h"

class RemoteIdThread : public ThreadBase
{
    DECLARE_THREAD(RemoteIdThread)
    DECLARE_TIMER1()
    DECLARE_TIMER2()
public:
protected:
    void _InitQueue(uint16_t& num) override;
    void _StartTimer() override;
private:
};

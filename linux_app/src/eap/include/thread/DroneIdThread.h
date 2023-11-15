#include "ThreadBase.h"

class DroneIdThread : public ThreadBase
{
    DECLARE_THREAD(DroneIdThread)
    DECLARE_TIMER1()
    DECLARE_TIMER2()
public:
protected:
    void _InitQueue(uint16_t& num) override;
    void _StartTimer() override;
private:
};

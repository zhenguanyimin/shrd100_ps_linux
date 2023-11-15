#include "ThreadBase.h"

class HealthDetectThread : public ThreadBase
{
    DECLARE_THREAD(HealthDetectThread)
    DECLARE_TIMER1()
public:
    void StartDelayTimer(uint8_t timerCode) override;
protected:
    void _StartTimer() override;
private:
};

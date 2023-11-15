#include "ThreadBase.h"

class DroneSnifferThread : public ThreadBase
{
    DECLARE_THREAD(DroneSnifferThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};

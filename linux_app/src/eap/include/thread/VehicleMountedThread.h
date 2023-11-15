#include "ThreadBase.h"

class VehicleMountedThread : public ThreadBase
{
    DECLARE_THREAD(VehicleMountedThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};


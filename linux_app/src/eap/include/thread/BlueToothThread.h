#include "ThreadBase.h"

class BlueToothThread : public ThreadBase
{
    DECLARE_THREAD(BlueToothThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};


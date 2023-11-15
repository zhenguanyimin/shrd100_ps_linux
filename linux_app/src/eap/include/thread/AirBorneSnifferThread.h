#include "ThreadBase.h"

class AirBorneSnifferThread : public ThreadBase
{
    DECLARE_THREAD(AirBorneSnifferThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};

#include "ThreadBase.h"

class AlinkThread : public ThreadBase
{
    DECLARE_THREAD(AlinkThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};

#include "ThreadBase.h"

class FrequencyThread : public ThreadBase
{
    DECLARE_THREAD(FrequencyThread)
public:
protected:
    void _InitQueue(uint16_t& num) override;
private:
};

#include "ThreadBase.h"

class RespMsgThread : public ThreadBase
{
    DECLARE_THREAD(RespMsgThread)
public:
    void _SaveInfoForRemote() override;
protected:
    void _InitQueue(uint16_t& num) override;
    void _OnCommand(const Thread_Msg& msg) override;
};

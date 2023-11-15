#include "ThreadBase.h"

#define CLI_MAX_OUTPUT_SIZE	1024

class CliThread : public ThreadBase
{
    DECLARE_THREAD(CliThread)
public:
    char* GetOutputString() { return _outputString; }
    static void ShowRegisteredCmds();
protected:
    void _InitQueue(uint16_t& num) override;
    void _OnCommand(const Thread_Msg& msg) override;
private:
    char _outputString[CLI_MAX_OUTPUT_SIZE];
};

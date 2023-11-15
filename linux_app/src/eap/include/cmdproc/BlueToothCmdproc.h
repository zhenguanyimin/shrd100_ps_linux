#include "CmdprocBase.h"
class BlueToothCmdproc : public CmdprocBase
{
public:
    BlueToothCmdproc();
    ~BlueToothCmdproc();
protected:
    void _BlueToothSetNameAndSecret(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
	void _RegisterCmd() override;
private:

};


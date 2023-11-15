#include "AirBorneSnifferModule.h"
#include "AirBorneSnifferCmdproc.h"
#include "AirBorneSnifferThread.h"
#include <sys/stat.h>

AirBorneSnifferModule::AirBorneSnifferModule(uint8_t coreId)
    : ModuleReal(coreId)
{
}

uint8_t AirBorneSnifferModule::GetModuleId()
{
    return EAP_MODULE_ID_AIRBORNE_SNIFFER;
}

CmdprocBase* AirBorneSnifferModule::_CreateCmdproc()
{
    CmdprocBase* cmdproc = new AirBorneSnifferCmdproc();
    cmdproc->SetRecvBuf(AIRBORNE_PARAM_LEN + sizeof(Thread_Msg_Head));
    cmdproc->SetSendBuf(AIRBORNE_RESULT_LEN + sizeof(Thread_Msg_Head));

    return cmdproc;
}

ThreadBase* AirBorneSnifferModule::_CreateThread()
{
    return ADD_THREAD(AirBorneSnifferThread, _coreId);
}

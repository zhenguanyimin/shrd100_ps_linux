#include "DroneSnifferModule.h"
#include "DroneSnifferCmdproc.h"
#include "DroneSnifferThread.h"

DroneSnifferModule::DroneSnifferModule(uint8_t coreId)
    : ModuleReal(coreId)
{
}

uint8_t DroneSnifferModule::GetModuleId()
{
    return EAP_MODULE_ID_DRONE_SNIFFER;
}

CmdprocBase* DroneSnifferModule::_CreateCmdproc()
{
    CmdprocBase* cmdproc = new DroneSnifferCmdproc(EAP_DRONE_SNIFFER_NUM);
    cmdproc->SetRecvBuf(DRONEID_SNIFFER_PARAM_LEN + sizeof(Thread_Msg_Head));
    cmdproc->SetSendBuf(DRONEID_SNIFFER_RESULT_LEN + sizeof(Thread_Msg_Head));
    return cmdproc;
}

ThreadBase* DroneSnifferModule::_CreateThread()
{
    return ADD_THREAD(DroneSnifferThread, _coreId);
}

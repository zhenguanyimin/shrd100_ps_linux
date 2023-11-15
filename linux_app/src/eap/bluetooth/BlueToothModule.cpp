#include "BlueToothModule.h"
#include "BlueToothCmdproc.h"
#include "BlueToothThread.h"

#define BLUETOOTH_DATA_LEN 100

BlueToothModule::BlueToothModule(uint8_t coreId)
    : ModuleReal(coreId)
{
}

uint8_t BlueToothModule::GetModuleId()
{
    return EAP_MODULE_ID_BLUETOOTH;
}

CmdprocBase* BlueToothModule::_CreateCmdproc()
{
    CmdprocBase* cmdproc = new BlueToothCmdproc();
    cmdproc->SetRecvBuf(BLUETOOTH_DATA_LEN + sizeof(Thread_Msg_Head));
    cmdproc->SetSendBuf(BLUETOOTH_DATA_LEN + sizeof(Thread_Msg_Head));

    return cmdproc;
}

ThreadBase* BlueToothModule::_CreateThread()
{
    return ADD_THREAD(BlueToothThread, _coreId);
}



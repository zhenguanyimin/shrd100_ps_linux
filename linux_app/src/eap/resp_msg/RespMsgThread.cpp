#include "RespMsgThread.h"

DEFINE_THREAD(RespMsgThread, RESP_MSG)

void RespMsgThread::_InitQueue(uint16_t& num)
{
    num = 100;
}

void RespMsgThread::_OnCommand(const Thread_Msg& msg)
{
    bool isSameProcess = (EapGetSys().GetProcessId() == EAP_GET_PROCESSID(msg.head.to));
    if (!isSameProcess)
    {
        //EAP_LOG_ERROR("Error processid!(local:%d msg:%d)", EapGetSys().GetProcessId(), EAP_GET_PROCESSID(msg.to));
        return;
    }
    EapGetSys().GetThreadMgr().SendLocalRespMsg(msg);
}

void RespMsgThread::_SaveInfoForRemote()
{
    if (nullptr == _queue) return;

    eap_set_resp_queuehandle_for_remote(EapGetSys().GetProcessId(), _queue->GetHandle());
}
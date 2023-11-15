#include "SysBase.h"
#include "HalBase.h"
#include "CmdprocBase.h"
#include "StubFunc.h"
#include "AlinkCmdproc.h"

extern "C"
{
    extern void stub_clear_remote_processid();
    extern void stub_clear_remote_queuehandle();
    extern void stub_clear_remote_resp_queuehandle();
}

Stub_DrvData g_stubHalInDatas[100] = {0};
uint32_t g_stubHalInDatasNum = 0;
Stub_DrvData g_stubHalOutDatas[100] = {0};
uint32_t g_stubHalOutDatasNum = 0;
Drv_Drone_Freq g_stubDroneFreq = {0, 0};

SysBase* g_sys = nullptr;
SysBase& EapGetSys()
{
    return *g_sys;
}

SysBase* sysBackup = nullptr;
Hal_Func codeProcFunsBackup[EAP_DRVCODE_MAX] = {0};
Cmdproc_Func cmdProcFunsBackup[EAP_CMD_MAX] = {0};
uint16_t cmd2ThreadIdsBackup[EAP_CMD_MAX] = {0};
uint16_t cmd2ProcessThreadIdsBackup[EAP_CMD_MAX] = {0};
Rpt_DroneInfo g_stubOutputList;
int g_stubOutputListNum = 0;

static void _ClearStubHalInDatas()
{
    for (uint32_t i = 0; i < g_stubHalInDatasNum; ++i)
    {
        EAP_DELETE_ARRAY(g_stubHalInDatas[i].buf);
    }
    g_stubHalInDatasNum = 0;
}

static void _ClearStubHalOutDatas()
{
    for (uint32_t i = 0; i < g_stubHalOutDatasNum; ++i)
    {
        EAP_DELETE_ARRAY(g_stubHalOutDatas[i].buf);
    }
    g_stubHalOutDatasNum = 0;
}

void ClearDatas()
{
    _ClearStubHalInDatas();
    _ClearStubHalOutDatas();
    Stub_SetDroneFreq(0, 0);
    g_stubOutputListNum = 0;
}

void Stub_Backup()
{
    memcpy((void*)codeProcFunsBackup, (void*)HalBase::GetCodeProcFuns(), sizeof(codeProcFunsBackup));
    memcpy((void*)cmdProcFunsBackup, (void*)CmdprocBase::GetCmdProcFuns(), sizeof(cmdProcFunsBackup));
    memcpy(cmd2ThreadIdsBackup, CmdprocBase::GetCmd2ThreadIds(), sizeof(cmd2ThreadIdsBackup));
    memcpy(cmd2ProcessThreadIdsBackup, CmdprocBase::GetCmd2ProcessThreadIds(), sizeof(cmd2ProcessThreadIdsBackup));
    sysBackup = g_sys;
}

void Stub_Restore()
{
    HalBase::SetCodeProcFuns(codeProcFunsBackup);
    CmdprocBase::SetCmdProcFuns(cmdProcFunsBackup);
    CmdprocBase::SetCmd2ThreadIds(cmd2ThreadIdsBackup);
    CmdprocBase::SetCmd2ProcessThreadIds(cmd2ProcessThreadIdsBackup);
    g_sys = sysBackup;
}

void Stub_ReInit(SysBase* newSys)
{
    Stub_Backup();
    CmdprocBase::Clear();
    HalBase::Clear();
    stub_clear_remote_processid();
    stub_clear_remote_queuehandle();
    stub_clear_remote_resp_queuehandle();
    g_sys = newSys;
    if (nullptr != g_sys) g_sys->Init();
    ClearDatas();
}

uint8_t* Stub_GetAlinkBuf(uint8_t* buf)
{
    alink_msg_t* msg = (alink_msg_t*)buf;
    return msg->buffer;
}

uint32_t Stub_GetAlinkBufLen(uint32_t bufLen)
{
    return bufLen - sizeof(alink_msg_head_t) - sizeof(uint16_t) - sizeof(uint16_t);
}

bool Stub_CheckRptInfoHeadByAlink(uint8_t* buf, uint8_t msgId, uint16_t msgLen)
{
    alink_msg_t* msg = (alink_msg_t*)buf;
    if (msg == nullptr || msg->msg_head.msgid != msgId) return false;
    if (msg->msg_head.len_lo != (msgLen & 0x00FF)) return false;
    if (msg->msg_head.len_hi != ((msgLen & 0xFF00) >> 8)) return false;
    return true;
}

bool Stub_CheckDupBoradcastResponse(uint8_t* buf, uint32_t protocol, uint16_t devType)
{
    alink_msg_t* msg = (alink_msg_t*)buf;
    AlinkDupBroadcastResp *response = (AlinkDupBroadcastResp *)msg->buffer;
    if (response == nullptr) return false;
    if (response->protocol != protocol) return false;
    if (response->type != devType) return false;
    return true;
}

bool Stub_CheckRptTargetNum(uint8_t* buf, uint8_t num)
{
    alink_msg_t* msg = (alink_msg_t*)buf;
    if (msg)
        return msg->buffer[EAP_SN_LENGTH] == num;
    else
        return false;
}
void Stub_SetDroneFreq(int freq, int freqOffset)
{
    g_stubDroneFreq.sigFreq = freq;
    g_stubDroneFreq.freqOffset = freqOffset;
}

extern "C"
{
    
    void Stub_SetOutputList(Rpt_DroneInfo* output)
    {
        memcpy(&g_stubOutputList, output, sizeof(Rpt_DroneInfo));
        g_stubOutputListNum = 1;
    }
    
}
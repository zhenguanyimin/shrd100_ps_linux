#include "CmdprocBase.h"
#include "eap_sys_cmds.h"
#include "SysBase.h"
#include "DataRecorder.h"
#include "MsgRecorderFormatter.h"
#include "HalBase.h"
#include "ModuleReal.h"

Cmdproc_Func CmdprocBase::s_cmdProcFuns[EAP_CMD_MAX] = {0};
uint16_t CmdprocBase::s_cmd2ThreadIds[EAP_CMD_MAX] = {0};
uint16_t CmdprocBase::s_cmd2ProcessThreadIds[EAP_CMD_MAX] = {0};
atomic<uint32_t> CmdprocBase::s_seqNo(0);
DataRecorder* CmdprocBase::s_recorder = nullptr;
bool CmdprocBase::s_isStoreCmd = true;
bool CmdprocBase::s_ctrlIsStoreCmd = true;
mutex* CmdprocBase::s_ignoreCmdMtx = nullptr;
set<uint16_t> CmdprocBase::s_ignoreCmds;
bool CmdprocBase::s_isIgnoreCmd = false;

CmdprocBase::CmdprocBase()
    : _threadId(UINT16_INVALID), _module(nullptr)
{
    _recvBufLen = sizeof(Thread_Msg);
    _recvBuf = new uint8_t[_recvBufLen];
    _sendBufLen = sizeof(Thread_Msg);
    _sendBuf = new uint8_t[_sendBufLen];
    _curCmdType = UINT8_INVALID;
    _curCmdCode = UINT16_INVALID;
}

CmdprocBase::~CmdprocBase()
{
    EAP_DELETE_ARRAY(_recvBuf);
    EAP_DELETE_ARRAY(_sendBuf);
}

void CmdprocBase::Init()
{
    if (nullptr == s_ignoreCmdMtx) s_ignoreCmdMtx = new mutex();
    s_recorder = DataRecorder::Instance();
#ifndef __UNITTEST__
    s_recorder->SetRecordFileName("/var/log", "eap_cmd_data", "log");
    s_recorder->SetFileMaxItemNum(1000);
    s_recorder->SetFileMaxSize(10 << 20);
#else
    CmdprocBase::DisableStoreCmd();
    HalBase::DisableStoreCmd();
//    s_recorder->SetRecordFileName("d:", "eap_cmd_data", "log");
//    s_recorder->SetFileMaxItemNum(5);
//    s_recorder->SetFileMaxSize(20 << 10);
#endif
    HalBase::SetDataRecorder(s_recorder);
}

void CmdprocBase::SetSendBuf(uint32_t bufLen)
{
    if (bufLen != _sendBufLen)
    {
        EAP_DELETE_ARRAY(_sendBuf);
        _sendBufLen = bufLen;
        if (0 != _sendBufLen) _sendBuf = new uint8_t[_sendBufLen];
    }
}

void CmdprocBase::SetRecvBuf(uint32_t bufLen)
{
    if (bufLen != _recvBufLen)
    {
        EAP_DELETE_ARRAY(_recvBuf);
        _recvBufLen = bufLen;
        if (0 != _recvBufLen) _recvBuf = new uint8_t[_recvBufLen];
    }
}

void CmdprocBase::Register(uint16_t cmdCode, Cmdproc_Func cmdFun, const CLI_Command_Definition_t *const cliRegister)
{
    if (cmdCode >= EAP_CMD_MAX)
    {
        EAP_LOG_ERROR("register cmdCode(%d) error!", cmdCode);
        return;
    }
    if (0 != s_cmd2ProcessThreadIds[cmdCode])
    {
        EAP_LOG_ERROR("s_cmd2ProcessThreadIds(cmdCode:%d) is not 0!", cmdCode);
        return;
    }
    s_cmdProcFuns[cmdCode] = cmdFun;
    s_cmd2ThreadIds[cmdCode] = _threadId;
    if (nullptr != cliRegister)
    {
        FreeRTOS_CLIRegisterCommand(cliRegister);
    }
}

void CmdprocBase::ProxyRegister(uint16_t cmdCode, uint16_t threadId, const CLI_Command_Definition_t *const cliRegister)
{
    if (cmdCode >= EAP_CMD_MAX)
    {
        EAP_LOG_ERROR("register cmdCode(%d) error!", cmdCode);
        return;
    }
    if (0 != s_cmd2ThreadIds[cmdCode])
    {
        EAP_LOG_ERROR("s_cmd2ThreadIds(cmdCode:%d) is not 0!", cmdCode);
        return;
    }
    s_cmd2ProcessThreadIds[cmdCode] = threadId;
    if (nullptr != cliRegister)
    {
        FreeRTOS_CLIRegisterCommand(cliRegister);
    }
}

void CmdprocBase::_StoreMsg(MsgRecorderFormatter& formatter)
{
    if (nullptr == s_recorder) return;
    Thread_Msg* msg = (Thread_Msg*)_recvBuf;
    formatter.Write(_recvBuf, sizeof(Thread_Msg_Head) + msg->head.length);
    s_recorder->Save(formatter);
}

void CmdprocBase::Dispatch()
{
    MsgRecorderFormatter* formatter = nullptr;
    s_isStoreCmd = s_ctrlIsStoreCmd;
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (s_isStoreCmd)
    {
        formatter = new MsgRecorderFormatter(inMsg->head.cmdCode);
        if (nullptr != formatter) formatter->Begin();
    }
    _curCmdType = inMsg->head.cmdType;
    _curCmdCode = inMsg->head.cmdCode;
    EapGetSys().GetThreadMgr().IncreCmdRecvReqStat(_curCmdCode);
    _Dispatch();
    _curCmdType = UINT8_INVALID;
    _curCmdCode = UINT16_INVALID;
    if (s_isStoreCmd)
    {
        if (nullptr != formatter)
        {
            formatter->End();
            _StoreMsg(*formatter);
        }
    }
}

bool CmdprocBase::IsTimerMsg()
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (nullptr == inMsg) return false;
    return (EAP_CMDTYPE_TIMER_1 <= inMsg->head.cmdType && inMsg->head.cmdType <= EAP_CMDTYPE_TIMER_8);
}

bool CmdprocBase::IsStopMsg()
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (nullptr == inMsg) return false;
    return (EAP_CMDTYPE_STOPMODULE == inMsg->head.cmdType);
}

bool CmdprocBase::IsRunHealthDetectMsg()
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (nullptr == inMsg) return false;
    return (EAP_CMDTYPE_RUN_HEALTH == inMsg->head.cmdType);
}

bool CmdprocBase::_IsModuleMsg()
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (nullptr == inMsg) return false;
    return (EAP_CMDTYPE_MODULE == inMsg->head.cmdType);
}

void CmdprocBase::_Dispatch()
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    if (_IsIgnoreCmd(_curCmdCode)) return;
    if (IsTimerMsg())
    { 
        _OnTimer(inMsg->head.cmdType);
        return;
    }
    if (IsStopMsg())
    {
        _OnStopModule();
        return;
    }
    if (!_IsModuleMsg()) return;

    if (_curCmdCode >= EAP_CMD_MAX)
    {
        EAP_LOG_ERROR("_Dispatch cmdCode(%d) error!", _curCmdCode);
        return;
    }
    if (s_cmd2ThreadIds[_curCmdCode] != _threadId)
    {
        EAP_LOG_ERROR("_Dispatch cmdCode(%d) error(selfThreadId:%d, cmdThreadId:%d)!", _curCmdCode, _threadId, s_cmd2ThreadIds[_curCmdCode]);
        return;
    }
    Thread_Msg* outMsg = (Thread_Msg*)_sendBuf;
    Cmdproc_Data inData;
    Cmdproc_Data outData;
    inData.dataLen = inMsg->head.length;
    inData.data = inMsg->buf;
    inData.maxLen = (_recvBufLen >= sizeof(Thread_Msg_Head)) ? (_recvBufLen - sizeof(Thread_Msg_Head)) : 0;
    outData.dataLen = 0;
    outData.data = outMsg->buf;
    outData.maxLen = (_sendBufLen >= sizeof(Thread_Msg_Head)) ? (_sendBufLen - sizeof(Thread_Msg_Head)) : 0;
    (this->*(s_cmdProcFuns[_curCmdCode]))(inData, outData);
    if (outData.dataLen > 0 && EAP_CMD_REQUEST == inMsg->head.reqType)
    {
        _SendResp(outData.dataLen);
    }
}

void CmdprocBase::_SendResp(uint32_t dataLen)
{
    Thread_Msg* inMsg = (Thread_Msg*)_recvBuf;
    Thread_Msg* outMsg = (Thread_Msg*)_sendBuf;
    _FillRespHead(inMsg->head, dataLen, outMsg->head);
    EapGetSys().GetThreadMgr().SendRespMsg(*outMsg);
}

void CmdprocBase::_FillRespHead(const Thread_Msg_Head& reqHead, uint32_t dataLen, Thread_Msg_Head& respHead)
{
    respHead.from = EAP_GET_LOC(EapGetSys().GetProcessId(), _threadId);
    respHead.to = reqHead.from;
    respHead.reqType = EAP_CMD_RESPONSE;
    respHead.cmdType = reqHead.cmdType;
    respHead.cmdCode = reqHead.cmdCode;
    respHead.seqNo = reqHead.seqNo;
    respHead.length = dataLen;
}

uint8_t* CmdprocBase::_GetSendDataBuf()
{
    if (_sendBufLen < sizeof(Thread_Msg_Head)) return nullptr;
    return _sendBuf + sizeof(Thread_Msg_Head);
}

uint32_t CmdprocBase::_GetSendDataMaxLen()
{
    if (_sendBufLen < sizeof(Thread_Msg_Head)) return 0;
    return _sendBufLen - sizeof(Thread_Msg_Head);
}

bool CmdprocBase::IsMatched(const Thread_Msg_Head& reqHead, const Thread_Msg_Head& respHead)
{
    if (reqHead.from != respHead.to) return false;
    if (reqHead.to != respHead.from) return false;
    if (EAP_CMD_REQUEST != reqHead.reqType || EAP_CMD_RESPONSE != respHead.reqType) return false;
    if (reqHead.cmdType != respHead.cmdType) return false;
    if (reqHead.cmdCode != respHead.cmdCode) return false;
    if (reqHead.seqNo != respHead.seqNo) return false;
    return true;
}

void CmdprocBase::SetThreadId(uint16_t threadId)
{ 
    _threadId = threadId;
    _RegisterCmd();
}

uint32_t CmdprocBase::_GetToLoc(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX)
    {
        EAP_LOG_ERROR("Query Recv Thread by cmdCode(%d) error!", cmdCode);
        return 0;
    }
    if (0 != s_cmd2ThreadIds[cmdCode])
    {
        return EAP_GET_LOC(EapGetSys().GetProcessId(), s_cmd2ThreadIds[cmdCode]);
    }
    else
    {
        if (0 != s_cmd2ProcessThreadIds[cmdCode])
        {
            int16_t processId = eap_get_remote_processid(s_cmd2ProcessThreadIds[cmdCode]);
            return EAP_GET_LOC(processId, s_cmd2ProcessThreadIds[cmdCode]);
        }
    }
    return 0;
}

void CmdprocBase::FillReqHead(Thread_Msg_Head& head, uint16_t from, uint16_t cmdCode, uint32_t dataLen)
{
    head.from = EAP_GET_LOC(EapGetSys().GetProcessId(), from);
    head.to = _GetToLoc(cmdCode);
    head.reqType = EAP_CMD_REQUEST;
    head.cmdType = EAP_CMDTYPE_MODULE;
    head.cmdCode = cmdCode;
    head.seqNo = ++s_seqNo;
    head.length = dataLen;
}

void CmdprocBase::FillMsgHead(Thread_Msg_Head& head, uint16_t from, uint16_t to, uint32_t dataLen)
{
    head.from = EAP_GET_LOC(EapGetSys().GetProcessId(), from);
    head.to = EAP_GET_LOC(EapGetSys().GetProcessId(), to);
    head.cmdCode = UINT16_INVALID;
    head.seqNo = 0;
    head.length = dataLen;
}

void CmdprocBase::Clear()
{
    memset(s_cmdProcFuns, 0, sizeof(s_cmdProcFuns));
    memset(s_cmd2ThreadIds, 0, sizeof(s_cmd2ThreadIds));
    memset(s_cmd2ProcessThreadIds, 0, sizeof(s_cmd2ProcessThreadIds));
}

void CmdprocBase::SetCmdProcFuns(Cmdproc_Func* funcs)
{
    memcpy(s_cmdProcFuns, funcs, sizeof(s_cmdProcFuns));
}

void CmdprocBase::SetCmd2ThreadIds(uint16_t* threadIds)
{
    memcpy(s_cmd2ThreadIds, threadIds, sizeof(s_cmd2ThreadIds));
}

void CmdprocBase::SetCmd2ProcessThreadIds(uint16_t* threadIds)
{
    memcpy(s_cmd2ProcessThreadIds, threadIds, sizeof(s_cmd2ProcessThreadIds));
}

void CmdprocBase::ShowInfo(uint8_t level)
{
    ShowKeyInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_threadId: " << _threadId;
    cout << EAP_BLANK_PREFIX(level) << "_recvBufLen: " << _recvBufLen;
    cout << EAP_BLANK_PREFIX(level) << "_recvBuf: " << (uint64_t)_recvBuf;
    cout << EAP_BLANK_PREFIX(level) << "_sendBufLen: " << _sendBufLen;
    cout << EAP_BLANK_PREFIX(level) << "_sendBuf: " << (uint64_t)_sendBuf;
    cout << EAP_BLANK_PREFIX(level) << "_curCmdType: " << (uint16_t)_curCmdType;
    cout << EAP_BLANK_PREFIX(level) << "_curCmdCode: " << _curCmdCode;
    if (nullptr != _module)
    {
        cout << EAP_BLANK_PREFIX(level) << "_module: ";
        _module->ShowKeyInfo(level + 1);
    }
}

void CmdprocBase::ShowKeyInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    EAP_OUT_THIS_PTR(level);
}

void CmdprocBase::_DoShowModuleInfo()
{
    if (nullptr != _module)
    {
        _module->ShowInfo(2);
    }
}

void CmdprocBase::_DoShowCmd2Fun(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX) return;
    cout << "\n  " << right << setw(4) << cmdCode;
    cout << " | " << left << setw(40) << ConvertStr(cmdCode);
    cout << " | " << right << setw(14) << reinterpret_cast<void*>(s_cmdProcFuns[cmdCode]);
    cout << " | " << right << setw(8) << s_cmd2ThreadIds[cmdCode];
    cout << " | " << right << setw(9) << s_cmd2ProcessThreadIds[cmdCode];
}

void CmdprocBase::_DoShowCmd2FunTitle()
{
    cout << "\n" << "  No.  | CmdCode                                  | Func           | ThreadId | PThreadId";
}

void CmdprocBase::ShowRegisterInfo()
{
    cout << hex;
    cout << "\n" << "s_seqNo: " << s_seqNo;
    cout << "\n" << "s_isStoreCmd: " << s_isStoreCmd;
    cout << "\n" << "s_ctrlIsStoreCmd: " << s_ctrlIsStoreCmd;
    cout << "\n" << "s_isIgnoreCmd: " << s_isIgnoreCmd;
    cout << "\n" << "s_ignoreCmds: [";
    for (auto cmd : s_ignoreCmds)
    {
        cout << cmd << ", ";
    }
    if (s_ignoreCmds.size() != 0)
    {
        cout << EAP_OUT_BACKSPACE;
    }
    cout << "]\n" << "CmdMap: ";
    _DoShowCmd2FunTitle();
    for (int i = 0; i < EAP_CMD_MAX; ++i)
    {
        _DoShowCmd2Fun(i);
    }
}

string CmdprocBase::ConvertStr(uint16_t cmdCode)
{
    EAP_GET_CMD_STR(cmdCode);
    return "unknown";
}

void CmdprocBase::AddIgnoreCmd(uint16_t cmdCode)
{
    if (nullptr == s_ignoreCmdMtx) return;
    unique_lock<mutex> lock(*s_ignoreCmdMtx);
    s_ignoreCmds.emplace(cmdCode);
    s_isIgnoreCmd = true;
    cout << " \n AddIgnoreCmd success.";
}

void CmdprocBase::DelIgnoreCmd(uint16_t cmdCode)
{
    if (nullptr == s_ignoreCmdMtx) return;
    unique_lock<mutex> lock(*s_ignoreCmdMtx);
    s_ignoreCmds.erase(cmdCode);
    if (s_ignoreCmds.size() == 0) s_isIgnoreCmd = false;
    cout << " \n DelIgnoreCmd success.";
}

bool CmdprocBase::_IsIgnoreCmd(uint16_t cmdCode)
{
    if (!s_isIgnoreCmd) return false;
    unique_lock<mutex> lock(*s_ignoreCmdMtx);
    return s_ignoreCmds.find(cmdCode) != s_ignoreCmds.end();
}

extern "C"
{
    void EnableStoreCmd(uint8_t type)
    {
        if (type != 1 && type != 2 && type != 0xFF)
        {
            cout << "\nParam is invalid!(1: cmd, 2:drv, 255: all)";
            return;
        }
        if (1 == type) CmdprocBase::EnableStoreCmd();
        else if (2 == type) HalBase::EnableStoreCmd();
        else
        {
            CmdprocBase::EnableStoreCmd();
            HalBase::EnableStoreCmd();
        }
        cout << "\nSuccess!";
    }

    void DisableStoreCmd(uint8_t type)
    {
        if (type != 1 && type != 2 && type != 0xFF)
        {
            cout << "\nParam is invalid!(1: cmd, 2:drv, 255: all)";
            return;
        }
        if (1 == type) CmdprocBase::DisableStoreCmd();
        else if (2 == type) HalBase::DisableStoreCmd();
        else
        {
            CmdprocBase::DisableStoreCmd();
            HalBase::DisableStoreCmd();
        } 
        cout << "\nSuccess!";
    }

    void ShowCmdprocRegisterInfo()
    {
        CmdprocBase::ShowRegisterInfo();
    }

    void AddIgnoreCmd(uint16_t cmdCode)
    {
        CmdprocBase::AddIgnoreCmd(cmdCode);
    }

    void DelIgnoreCmd(uint16_t cmdCode)
    {
        CmdprocBase::DelIgnoreCmd(cmdCode);
    }
}

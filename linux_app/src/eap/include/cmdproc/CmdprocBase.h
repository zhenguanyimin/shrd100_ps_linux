#pragma once

#include <atomic>
#include <string>
#include <set>
#include <mutex>
#include "ThreadBase.h"
#include "FreeRTOS_CLI.h"

using namespace std;

typedef enum
{
	TCP_LINK_C2  	= (0),
	TYPEC_LINK_C2   = (1),
	MAX_LINK_C2_NUM = (2),
}link_c2_t;


#define ADD_THREAD_WITH_CMDPROC(threadclass, coreId, cmdprocclass) \
{ \
    ThreadBase* thread = ADD_THREAD(threadclass, 0); \
    CmdprocBase* cmdproc = new cmdprocclass(); \
    thread->SetCmdproc(cmdproc); \
    cmdproc->SetThreadId(thread->GetId()); \
}

class ModuleReal;
class CmdprocBase;
typedef void (CmdprocBase::*Cmdproc_Func)(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);

class DataRecorder;
class MsgRecorderFormatter;

class CmdprocBase
{
public:
    CmdprocBase();
    virtual ~CmdprocBase();
    virtual void SetSendBuf(uint32_t bufLen);
    void SetRecvBuf(uint32_t bufLen);
    uint8_t* GetRecvBuf() { return _recvBuf; }
    uint32_t GetRecvBufLen() { return _recvBufLen; }
	uint32_t GetSendBufLen() { return _sendBufLen; }
    void SetThreadId(uint16_t threadId);
    uint16_t GetThreadId() { return _threadId; }
    void Dispatch();
    void Register(uint16_t cmdCode, Cmdproc_Func cmdFun, const CLI_Command_Definition_t *const cliRegister = nullptr);
    void SetModule(ModuleReal* module) { _module = module; }
    bool IsTimerMsg();
    bool IsStopMsg();
    bool IsRunHealthDetectMsg();
    virtual void ShowInfo(uint8_t level);
    void ShowKeyInfo(uint8_t level);
    static void ProxyRegister(uint16_t cmdCode, uint16_t threadId, const CLI_Command_Definition_t *const cliRegister = nullptr);
    static void Clear();
    static Cmdproc_Func* GetCmdProcFuns() { return s_cmdProcFuns; }
    static void SetCmdProcFuns(Cmdproc_Func* funcs);
    static uint16_t* GetCmd2ThreadIds() { return s_cmd2ThreadIds; }
    static void SetCmd2ThreadIds(uint16_t* threadIds);
    static uint16_t* GetCmd2ProcessThreadIds() { return s_cmd2ProcessThreadIds; }
    static void SetCmd2ProcessThreadIds(uint16_t* threadIds);
    static void FillReqHead(Thread_Msg_Head& head, uint16_t from, uint16_t cmdCode, uint32_t dataLen);
    static void FillMsgHead(Thread_Msg_Head& head, uint16_t from, uint16_t to, uint32_t dataLen);
    static bool IsMatched(const Thread_Msg_Head& reqHead, const Thread_Msg_Head& respHead);
    static void Init();
    static void EnableStoreCmd() { s_ctrlIsStoreCmd = true; }
    static void DisableStoreCmd() { s_ctrlIsStoreCmd = false; }
    static void ShowRegisterInfo();
    static string ConvertStr(uint16_t cmdCode);
    static void AddIgnoreCmd(uint16_t cmdCode);
    static void DelIgnoreCmd(uint16_t cmdCode);
protected:
    void _Dispatch();
    virtual void _RegisterCmd() = 0;
    virtual void _OnTimer(uint8_t timerType){}
    virtual void _OnStopModule(){}
    static uint32_t _GetToLoc(uint16_t cmdCode);
    void _SendResp(uint32_t dataLen);
    void _FillRespHead(const Thread_Msg_Head& reqHead, uint32_t dataLen, Thread_Msg_Head& respHead);
    uint8_t* _GetSendDataBuf();
    uint32_t _GetSendDataMaxLen();
    void _StoreMsg(MsgRecorderFormatter& recorder);
    bool _IsModuleMsg();
    void _DoShowModuleInfo();
    static void _DoShowCmd2FunTitle();
    static void _DoShowCmd2Fun(uint16_t cmdCode);
    static bool _IsIgnoreCmd(uint16_t cmdCode);
    uint16_t _threadId;
    ModuleReal* _module;
    uint8_t* _recvBuf;
    uint32_t _recvBufLen;
    uint8_t* _sendBuf;
    uint32_t _sendBufLen;
    uint8_t _curCmdType;
    uint16_t _curCmdCode;
    static DataRecorder* s_recorder;
private:
    static set<uint16_t> s_ignoreCmds;
    static mutex* s_ignoreCmdMtx;
    static bool s_isIgnoreCmd;
    static Cmdproc_Func s_cmdProcFuns[EAP_CMD_MAX];    
    static uint16_t s_cmd2ThreadIds[EAP_CMD_MAX];
    static uint16_t s_cmd2ProcessThreadIds[EAP_CMD_MAX]; // use for inter process comm
    static atomic<uint32_t> s_seqNo;
    static bool s_isStoreCmd;
    static bool s_ctrlIsStoreCmd;
};

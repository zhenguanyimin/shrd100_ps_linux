#pragma once

#include "eap_os_adapter.h"

#define THREAD_MAX      50
#define THREAD_ID_MAX   200
#define PROCESS_ID_MAX  4

class ThreadBase;

class ThreadMgr
{
public:
    ThreadMgr();
    void CreateThreads();
    void ReCreateThread(uint16_t threadId);
    uint8_t GetThreadCnt() { return _threadCnt; }
    void ClearThreads();
    uint16_t GetThreadIndex(uint16_t threadId);
    bool IsAllThreadsInited();
    ThreadBase* GetThread(uint16_t threadId);
    void AddThread(ThreadBase& thread);
    void DeleteThread(uint16_t threadId);
    int SendReqMsg(const Thread_Msg& msg);
    void SendRespMsg(const Thread_Msg& msg);
    void SendLocalRespMsg(const Thread_Msg& msg);
    void SendAndWaitResp(const Thread_Msg& reqMsg, uint8_t* respData, uint32_t &respLen);
    void ShowAllThreadsMsgStats();
    void ShowOneThreadMsgStats(uint16_t threadId);
    void ShowAllThreadsInfo();
    void BeginDetectRunHealth();
    void EndDetectRunHealth();
    void StopModules();
    void StartDelayTimer(uint16_t threadId, uint8_t timerCode);
    void ClearCmdStat(uint16_t cmdCode);
    void ClearAllCmdStat();
    void ShowCmdStat(uint16_t cmdCode);
    void ShowAllCmdStat();
    void IncreCmdRecvReqStat(uint16_t cmdCode);
    void IncreCmdRecvRespStat(uint16_t cmdCode);
    void Pause(uint16_t threadId);
    void Idling(uint16_t threadId);
    void Resume(uint16_t threadId);
protected:
    bool _IsSameProcess(const Thread_Msg_Head& msg)
    {
        return EAP_GET_PROCESSID(msg.to) == EAP_GET_PROCESSID(msg.from);
    }
    int _StoreMsg(const Thread_Msg& msg);
    uint32_t _GetRemoteHandle(int16_t threadId);
    uint32_t _GetRemoteRespHandle(int16_t processId);
    void _StatSyncReqMsg(uint16_t threadId);
    void _StatSyncReqRemoteMsg(uint16_t threadId);
    void _StatSyncRespMsg(uint16_t threadId);
    void _StatSyncRespRemoteMsg(uint16_t threadId);
    bool _IsSyncMsg(const Thread_Msg& msg);
    void _IncreCmdSendReqStat(uint16_t cmdCode);
    void _IncreCmdSendRespStat(uint16_t cmdCode);
    void _StatSendReqMsg(const Thread_Msg& msg);
    void _StatSendRespMsg(const Thread_Msg& msg);
    void _DoShowCmdStat(uint16_t cmdCode);
    void _DoShowCmdStatTitle();
private:
    ThreadBase* _threads[THREAD_MAX];//key: index
    uint16_t _threadId2Index[THREAD_ID_MAX]; // key: threadid
    uint32_t _threadId2Remote[THREAD_ID_MAX];
    uint32_t _processId2Remote[PROCESS_ID_MAX];
    uint8_t _threadCnt;
    Cmd_Stats _cmdSendRecvStats[EAP_CMD_MAX];
};

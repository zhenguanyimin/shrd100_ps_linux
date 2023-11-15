#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "eap_os_adapter.h"
#include "ThreadQueue.h"
#include "ThreadMgr.h"
#include "SysBase.h"

using namespace std;

enum 
{
    THREAD_STATE_INIT = 0,
    THREAD_STATE_WORKING,
    THREAD_STATE_PAUSE,
    THREAD_STATE_IDLING
};

enum
{
    THREAD_RUN_WELL = 0,
    THREAD_RUN_TESTING,
    THREAD_RUN_BAD
};

typedef void( *Thread_Entry )();

class ThreadQueue;
class CmdprocBase;
class SysBase;

class ThreadMsgStats
{
public:
    ThreadMsgStats();
    void IncreSyncReqMsg() { ++_syncReqMsgs; }
    void IncreSyncRespMsg() { ++_syncRespMsgs; }
    void IncreSyncReqRemoteMsg() { ++_syncReqRemoteMsgs; }
    void IncreSyncRespRemoteMsg() { ++_syncRespRemoteMsgs; }
    void IncreRecvMsg() { ++_recvMsgs; }
    void IncreTimerMsg() { ++_timerMsgs; ++_recvMsgs; }
    void IncreConsumedMsg(bool isTimerMsg)
    {
        ++_consumedRecvMsgs;
        if (isTimerMsg) ++_consumedTimerMsgs;
    }
    void Show(uint16_t remainMsg);
    static void IncreAsyncSendRemoteMsg() { ++s_asyncSendRemoteMsgs; }
    static uint32_t GetAsyncSendRemoteMsg() { return s_asyncSendRemoteMsgs; }
    static void ShowTitle();
protected:
private:
    uint32_t _syncReqMsgs;
    uint32_t _syncRespMsgs;
    uint32_t _syncReqRemoteMsgs;
    uint32_t _syncRespRemoteMsgs;
    uint32_t _recvMsgs;
    uint32_t _consumedRecvMsgs;
    uint32_t _timerMsgs;
    uint32_t _consumedTimerMsgs;
    static uint32_t s_asyncSendRemoteMsgs;
};

class ThreadBase
{
public:
    ThreadBase();
    virtual ~ThreadBase();
    virtual void PreInit();
    virtual int32_t Init(){ return EAP_SUCCESS; }
    virtual bool Run();
    void SetStackSize(int32_t stackSize) { _stackSize = stackSize; }
    int32_t GetStackSize() { return _stackSize; }
    void SetPrior(int8_t prior) { _prior = prior; }
    int8_t GetPrior() { return _prior; }
    void SetId(uint16_t id) { _id = id; }
    uint16_t GetId() { return _id; }
    void SetEntry(Thread_Entry entry) { _entry = entry; }
    Thread_Entry GetEntry() { return _entry; }
    void SetName(string name) { _name = name; }
    string GetName() { return _name;}
    void SetState(int8_t state) { _state = state; }
    int8_t GetState() { return _state; }
    uint64_t GetHandle() { return _handle; }
    void SetHandle(uint64_t handle) { _handle = handle; }
    void SendTimerMsg2Self(uint8_t timerCode);
    ThreadQueue* GetQueue() { return _queue; }
    void SetCmdproc(CmdprocBase* cmdproc) { _cmdproc = cmdproc; }
    CmdprocBase* GetCmdproc() { return _cmdproc; }
    uint32_t GetSleepMs() { return _sleepMs; }
    void SetSleepMs(uint32_t sleepMs) { _sleepMs = sleepMs; }
    int StoreMsg(const Thread_Msg& msg);
    void SetCoreId(int8_t coreId) { _coreId = coreId; }
    int8_t GetCoreId() { return _coreId; }
    virtual void _SaveInfoForRemote();
    void SendAndWaitResp(const Thread_Msg& reqMsg, uint8_t* respData, uint32_t &respLen);
    void RecvRespMsg(const Thread_Msg& respMsg);
    Thread_Msg GetRespMsg() { return _respMsg; }
    void StatSyncReqMsg();
    void StatSyncReqRemoteMsg();
    void StatSyncRespMsg();
    void StatSyncRespRemoteMsg();
    void ShowMsgStats();
    void BeginDetectRunHealth();
    void EndDetectRunHealth();
    void ShowInfos();
    virtual void StartDelayTimer(uint8_t timerCode){}
    uint8_t GetRunHealthState() { return _runHealthState; }
    virtual void ShowInfo(uint8_t level);
    void Pause();
    void Idling();
    void Resume();
    void Stop();
    void Create();
    static void ShowInfosTitle();
    static void ShowMsgStatsTitle();
protected:
    bool _IsMatched(const Thread_Msg& reqMsg);
    virtual void _InitQueue(uint16_t& num);
    virtual void _StartTimer(){}
    virtual void _OnCommand(const Thread_Msg& msg){}
    void _DetectRunHealthSuccess();
    string _ShowState();
    string _ShowRunHealth();
    void _StatRecvRespMsg();
    static void _ShowThreadName();
    static void _ShowInfosTitle();
    ThreadQueue* _queue;
    CmdprocBase* _cmdproc;
private:
    int32_t _ReceiveMsg(void *data);
    void _NotifySync();
    int32_t _stackSize;
    int8_t _prior;
    int8_t _state;
    uint16_t _id;
    string _name;
    uint32_t _sleepMs;
    int8_t _coreId;
    Thread_Entry _entry;
    uint64_t _handle;
    uint8_t _runHealthState;

    mutex _respMtx;
    condition_variable _respCv;
    bool _isResponsed;
    Thread_Msg _respMsg;
    ThreadMsgStats _msgStats;
};

extern SysBase& EapGetSys();

#define ADD_THREAD(classname, coreId)                                                                                                                           \
    classname::AddSelf(coreId)

#define ADD_THREAD_EX(classname, threadId, threadName, stackSize, prior, coreId)                                                                                \
    classname::AddSelf(threadId, threadName, stackSize, prior, coreId)

#define MODIFY_THREAD(classname, coreId)                                                                                                                        \
    ADD_THREAD(classname, coreId)

#define MODIFY_THREAD_EX(classname, threadId, threadName, stackSize, prior, coreId)                                                                             \
    ADD_THREAD_EX(classname, threadId, threadName, stackSize, prior, coreId)

#define DELETE_THREAD(classname)                                                                                                                                \
    classname::DeleteSelf()

#define THREAD_TEMPLATE(classname, thread_entry)                                                                                                                \
static void thread_entry()                                                                                                                                      \
{                                                                                                                                                               \
    ThreadBase* thread = classname::Instance();                                                                                                                 \
    if (nullptr == thread)                                                                                                                                      \
    {                                                                                                                                                           \
        EAP_LOG_ERROR("Thread %s not existed!", #thread_entry);                                                                                                 \
        return;                                                                                                                                                 \
    }                                                                                                                                                           \
    do                                                                                                                                                          \
    {                                                                                                                                                           \
        if (THREAD_STATE_INIT == thread->GetState())                                                                                                            \
        {                                                                                                                                                       \
            if (EAP_SUCCESS != thread->Init())                                                                                                                  \
            {                                                                                                                                                   \
                EAP_LOG_ERROR("Thread %d init error!", thread->GetId());                                                                                        \
                break;                                                                                                                                          \
            }                                                                                                                                                   \
            thread->SetState(THREAD_STATE_WORKING);                                                                                                             \
        }                                                                                                                                                       \
        else if (THREAD_STATE_INIT != thread->GetState())                                                                                                       \
        {                                                                                                                                                       \
            while (thread->Run())                                                                                                                               \
            {                                                                                                                                                   \
                if (0 != thread->GetSleepMs()) eap_thread_delay(thread->GetSleepMs());                                                                          \
            }                                                                                                                                                   \
            break;                                                                                                                                              \
        }                                                                                                                                                       \
    } while (1);                                                                                                                                                \
    eap_os_delete_thread(thread->GetHandle());                                                                                                                  \
}

#define DECLARE_THREAD(classname)                                                                                                                               \
    DECLARE_SINGLETON(classname)                                                                                                                                \
public:                                                                                                                                                         \
    static ThreadBase* AddSelf(int8_t coreId = -1);                                                                                                             \
    static ThreadBase* AddSelf(uint32_t threadId, string threadName, int32_t stackSize, int8_t prior, int8_t coreId);                                           \
    static void DeleteSelf();

#define DEFINE_THREAD(classname, threadname)                                                                                                          \
DEFINE_SINGLETON(classname)                                                                                                                                     \
extern "C"                                                                                                                                                      \
{                                                                                                                                                               \
    THREAD_TEMPLATE(classname, classname##Entry)                                                                                                                \
}                                                                                                                                                               \
ThreadBase* classname::AddSelf(int8_t coreId)                                                                                                                   \
{                                                                                                                                                               \
    return AddSelf(EAP_THREAD_ID_##threadname, #classname, EAP_STACK_SIZE_##threadname, EAP_THREAD_PRIOR_##threadname, coreId);                                                                        \
}                                                                                                                                                               \
ThreadBase* classname::AddSelf(uint32_t threadId, string threadName, int32_t stackSize, int8_t prior, int8_t coreId)                                            \
{                                                                                                                                                               \
    ThreadBase* thread = classname::Instance();                                                                                                                 \
    if (nullptr == thread)                                                                                                                                      \
    {                                                                                                                                                           \
        EAP_LOG_ERROR("Thread %s not existed!", thread->GetId());                                                                                               \
        return nullptr;                                                                                                                                         \
    }                                                                                                                                                           \
    thread->SetId(threadId);                                                                                                                                    \
    thread->SetStackSize(stackSize);                                                                                                                            \
    thread->SetPrior(prior);                                                                                                                                    \
    thread->SetName(threadName);                                                                                                                                \
    thread->SetEntry(classname##Entry);                                                                                                                         \
    thread->SetCoreId(coreId);                                                                                                                                  \
    EapGetSys().GetThreadMgr().AddThread(*thread);                                                                                                              \
    return thread;                                                                                                                                              \
}                                                                                                                                                               \
void classname::DeleteSelf()                                                                                                                                    \
{                                                                                                                                                               \
    ThreadBase* thread = classname::Instance();                                                                                                                 \
    if (nullptr != thread)                                                                                                                                      \
    {                                                                                                                                                           \
        EapGetSys().GetThreadMgr().DeleteThread(thread->GetId());                                                                                               \
    }                                                                                                                                                           \
}

#define TIMER_TEMPLATE(classname, timer_entry, timer_id)                                                                                                        \
static void timer_entry()                                                                                                                                       \
{                                                                                                                                                               \
    ThreadBase* thread = classname::Instance();                                                                                                                 \
    if (nullptr == thread)                                                                                                                                      \
    {                                                                                                                                                           \
        EAP_LOG_ERROR("Thread %s not existed!", thread->GetId());                                                                                               \
        return;                                                                                                                                                 \
    }\
    thread->SendTimerMsg2Self(timer_id);                                                                                                                        \
}

#define DECLARE_TIMER(timer_id)                                                                                                                                 \
public:                                                                                                                                                         \
    void _StartTimer##timer_id();                                                                                                                                  \
    void _StartTimer##timer_id(string inName, uint64_t periodMs, bool isCycle);

#define DECLARE_TIMER1()    DECLARE_TIMER(1)
#define DECLARE_TIMER2()    DECLARE_TIMER(2)
#define DECLARE_TIMER3()    DECLARE_TIMER(3)
#define DECLARE_TIMER4()    DECLARE_TIMER(3)
#define DECLARE_TIMER5()    DECLARE_TIMER(5)
#define DECLARE_TIMER6()    DECLARE_TIMER(6)
#define DECLARE_TIMER7()    DECLARE_TIMER(7)
#define DECLARE_TIMER8()    DECLARE_TIMER(8)


#define DEFINE_TIMER(classname, timer_id, period_ms, is_Cycle)                                                                                      \
extern "C"                                                                                                                                                      \
{                                                                                                                                                               \
    TIMER_TEMPLATE(classname, timer_entry##timer_id, EAP_CMDTYPE_TIMER_##timer_id)                                                                                                  \
}                                                                                                                                                               \
void classname::_StartTimer##timer_id()                                                                                                                            \
{                                                                                                                                                               \
    _StartTimer##timer_id(EAP_STR(classname##timer_id), period_ms, is_Cycle);                                                                                                        \
}                                                                                                                                                               \
void classname::_StartTimer##timer_id(string inName,uint64_t periodMs, bool isCycle)                                                                              \
{                                                                                                                                                               \
	int32_t *pvTimerID = NULL;                                                                                                                                  \
	pvTimerID = (int32_t *)1;     \
	eap_create_timer(inName.c_str(), periodMs, ((isCycle) ? 1 : 0), (void *)pvTimerID, timer_entry##timer_id);                         \
}

#define DEFINE_TIMER1(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 1, period_ms, is_Cycle)  
#define DEFINE_TIMER2(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 2, period_ms, is_Cycle)  
#define DEFINE_TIMER3(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 3, period_ms, is_Cycle)  
#define DEFINE_TIMER4(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 4, period_ms, is_Cycle)  
#define DEFINE_TIMER5(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 5, period_ms, is_Cycle)  
#define DEFINE_TIMER6(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 6, period_ms, is_Cycle)  
#define DEFINE_TIMER7(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 7, period_ms, is_Cycle)  
#define DEFINE_TIMER8(classname, period_ms, is_Cycle) DEFINE_TIMER(classname, 8, period_ms, is_Cycle)  

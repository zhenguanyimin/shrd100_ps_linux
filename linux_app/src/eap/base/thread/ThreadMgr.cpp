#include <string.h>
#include <strstream>

#include "ThreadMgr.h"
#include "ThreadBase.h"
#include "CmdprocBase.h"


ThreadMgr::ThreadMgr()
{
    _threadCnt = 0;
    memset(_threads, 0, sizeof(_threads));
    memset(_threadId2Index, 0, sizeof(_threadId2Index));
    memset(_threadId2Remote, 0, sizeof(_threadId2Remote));
    memset(_processId2Remote, 0, sizeof(_processId2Remote));
    memset(_cmdSendRecvStats, 0, sizeof(_cmdSendRecvStats));
}

void ThreadMgr::ReCreateThread(uint16_t threadId)
{
    ThreadBase *thread = GetThread(threadId);
    if (nullptr == thread) return;
    thread->Pause();
    thread->Create();
    thread->Resume();
}

void ThreadMgr::CreateThreads()
{
    int32_t i = 0;
    for (; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;

        thread->PreInit();
    }

    for (i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
         thread->Create();
    }
}

void ThreadMgr::ClearThreads()
{
    memset(_threadId2Index, 0, sizeof(_threadId2Index));
    memset(_threads, 0, sizeof(_threads));
    _threadCnt = 0;
}

uint16_t ThreadMgr::GetThreadIndex(uint16_t threadId)
{
    if (threadId >= THREAD_ID_MAX) return EAP_FAIL;

    return _threadId2Index[threadId];
}

bool ThreadMgr::IsAllThreadsInited()
{
    for (int32_t i = 0; i < _threadCnt; ++i)
    {
        if (nullptr == _threads[i]) continue;
        if (THREAD_STATE_WORKING != _threads[i]->GetState()) return false;
    }

    return true;
}

ThreadBase* ThreadMgr::GetThread(uint16_t threadId)
{
    uint16_t index = GetThreadIndex(threadId) - 1;
    if (index >= THREAD_MAX) return nullptr;
    return _threads[index];
}

void ThreadMgr::AddThread(ThreadBase& thread)
{
    if (_threadCnt >= THREAD_MAX)
    {
        EAP_LOG_ERROR("Thread overflow(%d)!", _threadCnt);
        return;
    }
    uint16_t threadId = thread.GetId();
    if (threadId >= THREAD_ID_MAX)
    {
        EAP_LOG_ERROR("threadId(%d) is error!", threadId);
        return;
    }
    if (0 != _threadId2Index[threadId])
    {
        _threads[_threadId2Index[threadId] - 1] = &thread;
    }
    else
    {
        _threads[_threadCnt] = &thread;

        ++_threadCnt;
        _threadId2Index[threadId] = _threadCnt;
    }
}

void ThreadMgr::DeleteThread(uint16_t threadId)
{
    if (threadId >= THREAD_ID_MAX)
    {
        EAP_LOG_ERROR("thread_id(%d) is error!", threadId);
        return;
    }
    if (0 != _threadId2Index[threadId])
    {
        _threadId2Index[threadId] = 0;
        _threads[_threadId2Index[threadId] - 1] = nullptr;
    }
}

int ThreadMgr::_StoreMsg(const Thread_Msg& msg)
{
    int16_t recvThreadId = EAP_GET_THREADID(msg.head.to);
    ThreadBase* thread = GetThread(recvThreadId);
    if (nullptr == thread)
    {
        EAP_LOG_ERROR("recvThreadId(%d) is error!", recvThreadId);
        return EAP_FAIL;
    }
    return thread->StoreMsg(msg);
}

void ThreadMgr::_StatSyncReqMsg(uint16_t threadId)
{
    ThreadBase* thread = GetThread(threadId);
    if (nullptr == thread) return;
    thread->StatSyncReqMsg();
}

void ThreadMgr::_StatSyncReqRemoteMsg(uint16_t threadId)
{
    ThreadBase* thread = GetThread(threadId);
    if (nullptr == thread) return;
    thread->StatSyncReqRemoteMsg();
}

void ThreadMgr::_StatSyncRespMsg(uint16_t threadId)
{
    ThreadBase* thread = GetThread(threadId);
    if (nullptr == thread) return;
    thread->StatSyncRespMsg();
}

void ThreadMgr::_StatSyncRespRemoteMsg(uint16_t threadId)
{
    ThreadBase* thread = GetThread(threadId);
    if (nullptr == thread) return;
    thread->StatSyncRespRemoteMsg();
}

bool ThreadMgr::_IsSyncMsg(const Thread_Msg& msg)
{
    return EAP_GET_THREADID(msg.head.from) != UINT16_INVALID;
}

void ThreadMgr::_StatSendReqMsg(const Thread_Msg& msg)
{
    _IncreCmdSendReqStat(msg.head.cmdCode);

    if (_IsSameProcess(msg.head))
    {
        if (_IsSyncMsg(msg)) _StatSyncReqMsg(EAP_GET_THREADID(msg.head.from));
    }
    else
    {
        uint32_t remoteHandle = _GetRemoteHandle(EAP_GET_THREADID(msg.head.to));
        if (0 != remoteHandle)
        {
            if (_IsSyncMsg(msg)) _StatSyncReqRemoteMsg(EAP_GET_THREADID(msg.head.from));
            else ThreadMsgStats::IncreAsyncSendRemoteMsg();
        }
    }
}

int ThreadMgr::SendReqMsg(const Thread_Msg& msg)
{
    _StatSendReqMsg(msg);
    if (_IsSameProcess(msg.head))
    {
		int32_t ret;
        ret =  _StoreMsg(msg);
		if(ret < 0){
			int16_t threadId = EAP_GET_THREADID(msg.head.to);
			ThreadBase* threadBase = GetThread(threadId);
			ThreadQueue * ThreadQueue = threadBase->GetQueue();
			if(ThreadQueue){
				EAP_LOG_ERROR("queue=%s,queueLen:%hu\n",threadBase->GetName(),ThreadQueue->GetRemainMsgNum());
			}else{
				EAP_LOG_ERROR("no found threadqueue, threadId:%hu\n",threadId);
			}
		} 
		return ret;
    }
    else
    {
        uint32_t remoteHandle = _GetRemoteHandle(EAP_GET_THREADID(msg.head.to));
        if (0 != remoteHandle)
        {
			int32_t ret;
		    ret = eap_queue_send(remoteHandle, &msg, msg.head.length + sizeof(Thread_Msg_Head), 10,0);
			if(ret < 0){
				int16_t threadId = EAP_GET_THREADID(msg.head.to);
				ThreadBase* threadBase = GetThread(threadId);
				ThreadQueue * ThreadQueue = threadBase->GetQueue();
				if(ThreadQueue){
					EAP_LOG_ERROR("queue=%s,queueLen:%hu\n",threadBase->GetName(),ThreadQueue->GetRemainMsgNum());
				}else{
					EAP_LOG_ERROR("no found threadqueue, threadId:%hu\n",threadId);
				}
			} 
            return ret;
        }
    }
    return -1;
}

uint32_t ThreadMgr::_GetRemoteHandle(int16_t threadId)
{
    if (threadId < THREAD_ID_MAX)
    {
        uint32_t handle = _threadId2Remote[threadId];
        if (0 == handle)
        {
            handle = eap_get_remote_queue_handle(threadId);
            if (-1 != handle)
            {
                _threadId2Remote[threadId] = handle;
            }
        }
        return _threadId2Remote[threadId];
    }
    return 0;
}

uint32_t ThreadMgr::_GetRemoteRespHandle(int16_t processId)
{
    if (processId < PROCESS_ID_MAX)
    {
        uint32_t handle = _processId2Remote[processId];
        if (0 == handle)
        {
            handle = eap_get_remote_resp_queue(processId);
            if (-1 != handle)
            {
                _processId2Remote[processId] = handle;
            }
        }
        return _processId2Remote[processId];
    }
    return 0;
}

void ThreadMgr::_StatSendRespMsg(const Thread_Msg& msg)
{
    _IncreCmdSendRespStat(msg.head.cmdCode);
    if (_IsSameProcess(msg.head))
    {
        _StatSyncRespMsg(EAP_GET_THREADID(msg.head.from));
    }
    else
    {
        uint32_t remoteHandle = _GetRemoteRespHandle(EAP_GET_PROCESSID(msg.head.to));
        if (0 != remoteHandle)
        {
            _StatSyncRespRemoteMsg(EAP_GET_THREADID(msg.head.from));
        }
    }
}

void ThreadMgr::SendRespMsg(const Thread_Msg& msg)
{
    if (_IsSameProcess(msg.head))
    {
        SendLocalRespMsg(msg);
    }
    else
    {
        uint32_t remoteHandle = _GetRemoteRespHandle(EAP_GET_PROCESSID(msg.head.to));
        if (0 != remoteHandle)
        {
			int32_t ret;
		    ret = eap_queue_send(remoteHandle, &msg, msg.head.length + sizeof(Thread_Msg_Head), 10,0);
			if(ret < 0){
				int16_t threadId = EAP_GET_THREADID(msg.head.to);
				ThreadBase* threadBase = GetThread(threadId);
				ThreadQueue * ThreadQueue = threadBase->GetQueue();
				if(ThreadQueue){
					EAP_LOG_ERROR("queue=%s,queueLen:%hu\n",threadBase->GetName(),ThreadQueue->GetRemainMsgNum());
				}else{
					EAP_LOG_ERROR("no found threadqueue, threadId:%hu\n",threadId);
				}
			}      
        }
    }
}

void ThreadMgr::SendLocalRespMsg(const Thread_Msg& msg)
{
    ThreadBase* thread = GetThread(EAP_GET_THREADID(msg.head.to));
    if (nullptr != thread)
    {
        thread->RecvRespMsg(msg);
    }
}

void ThreadMgr::SendAndWaitResp(const Thread_Msg& reqMsg, uint8_t* respData, uint32_t &respLen)
{
    ThreadBase* thread = GetThread(EAP_GET_THREADID(reqMsg.head.from));
   if (nullptr != thread)
    {
        thread->SendAndWaitResp(reqMsg, respData, respLen);
    }
}

void ThreadMgr::ShowAllThreadsMsgStats()
{
    cout << hex;
    ThreadBase::ShowMsgStatsTitle();
    for (int i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
        thread->ShowMsgStats();
    }
    cout << "\n" << "AsyncSendRemoteMsgs: " << ThreadMsgStats::GetAsyncSendRemoteMsg();
}

void ThreadMgr::ShowAllThreadsInfo()
{
    cout << hex;
    ThreadBase::ShowInfosTitle();
    for (int i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
        thread->ShowInfos();
    }
}

void ThreadMgr::ShowOneThreadMsgStats(uint16_t threadId)
{
    cout << hex;
    ThreadBase* thread = GetThread(threadId);
    if (nullptr == thread)
    {
        cout << "\nParam is invalid!";
        return;
    }
    ThreadBase::ShowMsgStatsTitle();
    thread->ShowMsgStats();
}

void ThreadMgr::StopModules()
{
    Thread_Msg msg = {0};
    msg.head.cmdType = EAP_CMDTYPE_STOPMODULE;
    for (int i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
        SendAsynMsg2Thread(thread->GetId(), (uint8_t*)&msg, 0);
    }
}

void ThreadMgr::BeginDetectRunHealth()
{
    Thread_Msg msg = {0};
    msg.head.cmdType = EAP_CMDTYPE_RUN_HEALTH;
    for (int i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
        if (EAP_THREAD_ID_HEALTH_DETECT == thread->GetId()) continue;
        thread->BeginDetectRunHealth();
        SendAsynMsg2Thread(thread->GetId(), (uint8_t*)&msg, 0);
    }
}
void ThreadMgr::EndDetectRunHealth()
{
    for (int i = 0; i < _threadCnt; ++i)
    {
        ThreadBase* thread = _threads[i];
        if (nullptr == thread) continue;
        thread->EndDetectRunHealth();
    }
    cout << "\nEnd Detect Thread Run Health.";
}

void ThreadMgr::Pause(uint16_t threadId)
{
    if (UINT16_ALL == threadId)
    {
        for (int i = 0; i < _threadCnt; ++i)
        {
            ThreadBase* thread = _threads[i];
            if (nullptr == thread) continue;
            if (EAP_THREAD_ID_HEALTH_DETECT == thread->GetId()) continue;
            thread->Pause();
        }
        cout << "\n All threads pause success.";
    }
    else
    {
        ThreadBase* thread = GetThread(threadId);
        if (nullptr == thread)
        {
            cout << "\n Thread pause fail.";
            return;
        }
        thread->Pause();
        cout << "\n " << thread->GetName() << " success.";
    }
}

void ThreadMgr::Idling(uint16_t threadId)
{
    if (UINT16_ALL == threadId)
    {
        for (int i = 0; i < _threadCnt; ++i)
        {
            ThreadBase* thread = _threads[i];
            if (nullptr == thread) continue;
            if (EAP_THREAD_ID_HEALTH_DETECT == thread->GetId()) continue;
            thread->Idling();
        }
        cout << "\n All threads idling success.";
    }
    else
    {
        ThreadBase* thread = GetThread(threadId);
        if (nullptr == thread)
        {
            cout << "\n Thread idling fail.";
            return;
        }
        thread->Idling();
        cout << "\n " << thread->GetName() << " idling success.";
    }
}

void ThreadMgr::Resume(uint16_t threadId)
{
    if (UINT16_ALL == threadId)
    {
        for (int i = 0; i < _threadCnt; ++i)
        {
            ThreadBase* thread = _threads[i];
            if (nullptr == thread) continue;
            if (EAP_THREAD_ID_HEALTH_DETECT == thread->GetId()) continue;
            thread->Resume();
        }
        cout << "\n All threads resume success.";
    }
    else
    {
        ThreadBase* thread = GetThread(threadId);
        if (nullptr == thread)
        {
            cout << "\n Thread resume fail.";
            return;
        }
        thread->Resume();
        cout << "\n " << thread->GetName() << " resume success.";
    }
}

void ThreadMgr::StartDelayTimer(uint16_t threadId, uint8_t timerCode)
{
    ThreadBase* thread = GetThread(threadId);
    if (nullptr != thread)
    {
        thread->StartDelayTimer(timerCode);
    }
}


void ThreadMgr::_IncreCmdSendReqStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX) return;
    _cmdSendRecvStats[cmdCode].sendRequest++;
}

void ThreadMgr::IncreCmdRecvReqStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX) return;
    _cmdSendRecvStats[cmdCode].recvRequest++;
}

void ThreadMgr::_IncreCmdSendRespStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX) return;
    _cmdSendRecvStats[cmdCode].sendResponse++;
}

void ThreadMgr::IncreCmdRecvRespStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX) return;
    _cmdSendRecvStats[cmdCode].recvResponse++;
}

void ThreadMgr::ClearCmdStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX)
    {
        cout << "\nParam is invalid!";
        return;
    }
    memset(&_cmdSendRecvStats[cmdCode], 0, sizeof(Cmd_Stats));
    cout << "\nSuccess!";
}

void ThreadMgr::ClearAllCmdStat()
{
    memset(_cmdSendRecvStats, 0, sizeof(_cmdSendRecvStats));
}

void ThreadMgr::_DoShowCmdStat(uint16_t cmdCode)
{
    if (cmdCode >= EAP_CMD_MAX)
    {
        cout << "\nParam is invalid!";
        return;
    }
    cout << hex;
    cout << "\n  " << right << setw(4) << cmdCode;
    cout << " | " << left << setw(40) << CmdprocBase::ConvertStr(cmdCode);
    cout << " | " << right << setw(8) << _cmdSendRecvStats[cmdCode].sendRequest;
    cout << " | " << right << setw(8) << _cmdSendRecvStats[cmdCode].recvRequest;
    cout << " | " << right << setw(8) << _cmdSendRecvStats[cmdCode].sendResponse;
    cout << " | " << right << setw(8) << _cmdSendRecvStats[cmdCode].recvResponse;
}

void ThreadMgr::_DoShowCmdStatTitle()
{
    cout << "\n" << "  No.  | CmdCode                                  | sendReq  | recvReq  | sendResp | recvResp";
}

void ThreadMgr::ShowCmdStat(uint16_t cmdCode)
{
    _DoShowCmdStatTitle();
    _DoShowCmdStat(cmdCode);
}

void ThreadMgr::ShowAllCmdStat()
{
    _DoShowCmdStatTitle();
    cout << hex;
    for (int i = 0; i < EAP_CMD_MAX; ++i)
    {
        _DoShowCmdStat(i);
    }
}

extern "C"
{
    void ShowOneThreadInfo(uint16_t threadId)
    {
        ThreadBase *thread = EapGetSys().GetThreadMgr().GetThread(threadId);
		if (nullptr != thread) thread->ShowInfo(0);
    }
	
    void ShowAllThreadsMsgStats()
    {
        EapGetSys().GetThreadMgr().ShowAllThreadsMsgStats();
    }

    void ShowOneThreadMsgStats(uint16_t threadId)
    {
        EapGetSys().GetThreadMgr().ShowOneThreadMsgStats(threadId);
    }

    void OpenThreadRunDetect()
    {
        SendAsynCmdCode(EAP_CMD_RUN_HEALTH_DETECT);
    }

    void ThreadPause(uint16_t threadId)
    {
        SendAsynData(EAP_CMD_RUN_PAUSE, (uint8_t *)&threadId, sizeof(uint16_t));
    }

    void ThreadIdling(uint16_t threadId)
    {
        SendAsynData(EAP_CMD_RUN_IDLING, (uint8_t *)&threadId, sizeof(uint16_t));
    }

    void ThreadResume(uint16_t threadId)
    {
        SendAsynData(EAP_CMD_RUN_RESUME, (uint8_t *)&threadId, sizeof(uint16_t));
    }

    void ShowAllThreadsInfo()
    {
        EapGetSys().GetThreadMgr().ShowAllThreadsInfo();
    }

    void ClearAllCmdStat()
    {
        EapGetSys().GetThreadMgr().ClearAllCmdStat();
        cout << "\nSuccess!";
    }

    void ClearOneCmdStat(uint16_t cmdCode)
    {
        EapGetSys().GetThreadMgr().ClearCmdStat(cmdCode);
    }

    void ShowAllCmdStat()
    {
        EapGetSys().GetThreadMgr().ShowAllCmdStat();
    }

    void ShowOneCmdStat(uint16_t cmdCode)
    {
        EapGetSys().GetThreadMgr().ShowCmdStat(cmdCode);
    }

    void ReCreateThread(uint16_t threadId)
    {
        EapGetSys().GetThreadMgr().ReCreateThread(threadId);
    }
}
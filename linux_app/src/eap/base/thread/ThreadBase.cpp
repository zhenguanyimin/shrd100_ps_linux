#include <string.h>

#include "ThreadBase.h"
#include "CmdprocBase.h"
#include "SysBase.h"
#include <iostream>
#include <chrono>
#include <iomanip>

uint32_t ThreadMsgStats::s_asyncSendRemoteMsgs = 0;

ThreadMsgStats::ThreadMsgStats()
{
    _syncReqMsgs = 0;
    _syncRespMsgs = 0;
    _syncReqRemoteMsgs = 0;
    _syncRespRemoteMsgs = 0;
    _recvMsgs = 0;
    _consumedRecvMsgs = 0;
    _timerMsgs = 0;
    _consumedTimerMsgs = 0;
}

ThreadBase::ThreadBase()
    : _stackSize(0), _prior(0), _state(0), _id(THREAD_STATE_INIT), _sleepMs(0),
    _queue(nullptr), _handle(0), _entry(nullptr), _cmdproc(nullptr), _coreId(-1),
    _runHealthState(THREAD_RUN_WELL)
{
    memset(&_respMsg, 0, sizeof(Thread_Msg));
    _isResponsed = false;
}

ThreadBase::~ThreadBase()
{
    EAP_DELETE(_queue);
}

void ThreadBase::PreInit()
{
    uint16_t num = 0;
    _InitQueue(num);
    uint32_t maxItemLen = (nullptr != _cmdproc) ? _cmdproc->GetRecvBufLen() : sizeof(Thread_Msg);
    if (nullptr == _queue)
    {
        _queue = new ThreadQueue(num + 1, maxItemLen, _name.c_str(),_id);
    }
    _StartTimer();
}

void ThreadBase::_InitQueue(uint16_t& num)
{
    num = 0;
}

void ThreadBase::_DetectRunHealthSuccess()
{
    _runHealthState = THREAD_RUN_WELL;
}

void ThreadBase::EndDetectRunHealth()
{
    if (THREAD_RUN_TESTING == _runHealthState) _runHealthState = THREAD_RUN_BAD;
}

void ThreadBase::BeginDetectRunHealth()
{
    _runHealthState = THREAD_RUN_TESTING;
}

bool ThreadBase::Run()
{
    if (THREAD_STATE_PAUSE == _state) return true;
    if (nullptr != _queue)
    {
        if (nullptr != _cmdproc)
        {
            if ( _queue->ConsumerMsg(_cmdproc->GetRecvBuf(), _cmdproc->GetRecvBufLen()) == 0)
            {
                if (THREAD_STATE_IDLING == _state) return true;
                _cmdproc->Dispatch();
                _msgStats.IncreConsumedMsg(_cmdproc->IsTimerMsg());
                if (_cmdproc->IsRunHealthDetectMsg()) _DetectRunHealthSuccess();
            }
            else
            {
                Thread_Msg* msg = (Thread_Msg*)_cmdproc->GetRecvBuf();
                if(msg != nullptr)
                {
                    EAP_LOG_ERROR("recv data is too long msg from %d to %d\n,reqType is %d,cmdType is %d,cmdCode is %d,\
                    seqNo is %d,length is %d\n",msg->head.from,msg->head.to,msg->head.reqType,msg->head.cmdType\
                    ,msg->head.cmdCode,msg->head.seqNo,msg->head.length);
                }
            }
        }
        else
        {
            Thread_Msg msg;
            if (_queue->ConsumerMsg((uint8_t*)&msg, sizeof(Thread_Msg)) == 0)
            {
                if (THREAD_STATE_IDLING == _state) return true;
                _OnCommand(msg);
                bool isTimerMsg = (EAP_CMDTYPE_TIMER_1 <= msg.head.cmdType && msg.head.cmdType <= EAP_CMDTYPE_TIMER_8);
                _msgStats.IncreConsumedMsg(isTimerMsg);
                bool isHealthDetectMsg = (EAP_CMDTYPE_RUN_HEALTH == msg.head.cmdType);
                if (isHealthDetectMsg) _DetectRunHealthSuccess();
            }
            else
            {
                EAP_LOG_ERROR("recv data is too long msg from %d to %d\n,reqType is %d,cmdType is %d,cmdCode is %d,\
                seqNo is %d,length is %d\n",msg.head.from,msg.head.to,msg.head.reqType,msg.head.cmdType\
                ,msg.head.cmdCode,msg.head.seqNo,msg.head.length);
            }
        }
        return true;
    }
    return false;
}

void ThreadBase::SendTimerMsg2Self(uint8_t timerCode)
{
    if (nullptr == _queue) return;

    Thread_Msg msg = {0};
    msg.head.cmdType = timerCode;
    msg.head.length = 0;
    _msgStats.IncreTimerMsg();
    _queue->ProduceMsgWithPrio((uint8_t*)&msg, sizeof(msg.head),10);
}

int ThreadBase::StoreMsg(const Thread_Msg& msg)
{
    if (nullptr == _queue) return EAP_FAIL;

    _msgStats.IncreRecvMsg();
    return _queue->ProduceMsg((const uint8_t* )&msg, sizeof(msg.head) + msg.head.length);
}

void ThreadBase::_SaveInfoForRemote()
{
    eap_set_processid_for_remote(_id, EapGetSys().GetProcessId());
    if (nullptr == _queue) return;

    eap_set_queue_handle_for_remote(_id, (int32_t)_queue->GetHandle());
}

void ThreadBase::RecvRespMsg(const Thread_Msg& respMsg)
{
    unique_lock<mutex> lock(_respMtx);
    if (sizeof(_respMsg.buf) < respMsg.head.length)
    {
        EAP_LOG_ERROR("respMsg length(%d) is greater than bufLen(%d)!", respMsg.head.length, sizeof(_respMsg.buf));
        return;
    }
    memcpy(&_respMsg, &respMsg, (sizeof(Thread_Msg_Head) + respMsg.head.length));
    _isResponsed = true;
    _respCv.notify_all();
}

bool ThreadBase::_IsMatched(const Thread_Msg& reqMsg)
{
    return CmdprocBase::IsMatched(reqMsg.head, _respMsg.head);
}

void ThreadBase::_StatRecvRespMsg()
{
    bool isRemote = (EAP_GET_PROCESSID(_respMsg.head.from) != EAP_GET_PROCESSID(_respMsg.head.to));
    if (isRemote) _msgStats.IncreSyncRespRemoteMsg();
    else _msgStats.IncreSyncRespMsg();
    
    EapGetSys().GetThreadMgr().IncreCmdRecvRespStat(_respMsg.head.cmdCode);
}

#ifdef __UNITTEST__
Thread_Msg stubReqMsg;
#endif

void ThreadBase::SendAndWaitResp(const Thread_Msg& reqMsg, uint8_t* respData, uint32_t &respLen)
{
    unique_lock<mutex> lock(_respMtx);
#ifdef __UNITTEST__
    memcpy(&stubReqMsg, &reqMsg, sizeof(Thread_Msg));
#endif

    if (EAP_SUCCESS != EapGetSys().GetThreadMgr().SendReqMsg(reqMsg))
    {
        EAP_LOG_ERROR("SendReqMsg error!");
        return;
    }
	while (!_isResponsed)
	{
#ifndef __UNITTEST__
		_respCv.wait(lock);
#else
        break;
#endif
	}
    if (_IsMatched(reqMsg))
    {
        if (_respMsg.head.length > respLen)
        {
            EAP_LOG_ERROR("Response's length(%d) is bigger!", _respMsg.head.length);
            respLen = 0;
        }
        else
        {
            respLen = _respMsg.head.length;
            memcpy(respData, _respMsg.buf, respLen);
            _StatRecvRespMsg();
        }
    }
    else
    {
        EAP_LOG_ERROR("Response is not matched with request!");
        respLen = 0;
    }
    _isResponsed = false;
}

void ThreadBase::StatSyncReqMsg()
{
    _msgStats.IncreSyncReqMsg();
}

void ThreadBase::StatSyncReqRemoteMsg()
{
    _msgStats.IncreSyncReqRemoteMsg();
}

void ThreadBase::StatSyncRespMsg()
{
    _msgStats.IncreSyncRespMsg();
}

void ThreadBase::StatSyncRespRemoteMsg()
{
    _msgStats.IncreSyncRespRemoteMsg();
}
void ThreadBase::ShowMsgStats()
{
    cout << "\n\t" << left << setw(20) << _name << " | ";
    cout << left << setw(4) << _id << " | ";
    uint32_t remainMsg = (nullptr == _queue) ? 0 : _queue->GetRemainMsgNum();
    _msgStats.Show(remainMsg);
}

void ThreadBase::ShowMsgStatsTitle()
{
    _ShowThreadName();
    ThreadMsgStats::ShowTitle();
}

void ThreadBase::ShowInfosTitle()
{
    _ShowThreadName();
    _ShowInfosTitle();
}

void ThreadBase::_ShowInfosTitle()
{
    cout << "Core | Pri | Stat | Run  | Stack    | Sleep    | Queue(num/wait)";
}

void ThreadBase::ShowInfos()
{
    cout << "\n\t";
    cout << left << setw(20) << _name << " | ";
    cout << left << setw(4) << _id << " | ";
    cout << left << setw(4) << (int16_t)_coreId << " | ";
    cout << left << setw(3) << (int16_t)_prior << " | ";
    cout << left << setw(4) << _ShowState() << " | ";
    cout << left << setw(4) << _ShowRunHealth() << " | ";
    cout << left << setw(8) << _stackSize << " | ";
    cout << left << setw(8) << _sleepMs << " | ";
    cout << right << setw(4) << ((nullptr == _queue) ? UINT16_INVALID : _queue->GetNum()) << "/";
    cout << left << ((nullptr == _queue) ? 0 : _queue->GetWaitMs());
}

string ThreadBase::_ShowState()
{
    switch (_state)
    {
    case THREAD_STATE_INIT:
        return "init";
    case THREAD_STATE_WORKING:
        return "work";
    default:
        break;
    }
    return "err";
}

string ThreadBase::_ShowRunHealth()
{
    switch (_runHealthState)
    {
    case THREAD_RUN_WELL:
        return "well";
    case THREAD_RUN_TESTING:
        return "====";
    case THREAD_RUN_BAD:
        return "bad";
    default:
        break;
    }
    return "err";
}

void ThreadBase::_ShowThreadName()
{
    cout << "\n\tName                 | Id   | ";
}

void ThreadMsgStats::Show(uint16_t remainMsg)
{
    cout << right << setw(8) << _syncReqMsgs << "/" << left << setw(8) << _syncRespMsgs << " | ";
    cout << right << setw(9) << _syncReqRemoteMsgs << "/" << left << setw(8) << _syncRespRemoteMsgs << " | ";
    cout << right << setw(8) << _recvMsgs << "/" << setfill('0') << setw(8) << _consumedRecvMsgs << "/" << setfill(' ') << left << setw(4) << remainMsg << " | ";
    cout << right << setw(8) << _timerMsgs << "/" << left << setw(8) << _consumedTimerMsgs;
	// 获取当前时间
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);
	// 将时间格式化为字符串
	cout << right << setw(8) << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << std::endl;
}

void ThreadMsgStats::ShowTitle()
{
    cout << "Send(Req/Resp)    | SendR(Req/Resp)    |   Recv(A/C/R)          |  Timer(A/C)   |  TimeStamp";
}

void ThreadBase::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    cout << EAP_BLANK_PREFIX(level) << "_id: " << (uint16_t)_id;
    cout << EAP_BLANK_PREFIX(level) << "_stackSize: " << _stackSize;
    cout << EAP_BLANK_PREFIX(level) << "_prior: " << (int16_t)_prior;
    cout << EAP_BLANK_PREFIX(level) << "_state: " << (int16_t)_state;
    cout << EAP_BLANK_PREFIX(level) << "_name: " << _name;
    cout << EAP_BLANK_PREFIX(level) << "_sleepMs: " << _sleepMs;
    cout << EAP_BLANK_PREFIX(level) << "_coreId: " << (int16_t)_coreId;
    cout << EAP_BLANK_PREFIX(level) << "_handle: " << _handle;
    cout << EAP_BLANK_PREFIX(level) << "_runHealthState: " << (uint16_t)_runHealthState;
    cout << EAP_BLANK_PREFIX(level) << "_isResponsed: " << _isResponsed;
    if (nullptr != _cmdproc)
    {
        cout << EAP_BLANK_PREFIX(level) << "_cmdproc: ";
        _cmdproc->ShowKeyInfo(level + 1);
    }
    if (nullptr != _queue)
    {
        cout << EAP_BLANK_PREFIX(level) << "_queue: ";
        _queue->ShowInfo(level + 1);
    }
    cout << EAP_BLANK_PREFIX(level) << "_respMsg: ";
    cout << EAP_BLANK_PREFIX(level + 1) << "head.from: " << _respMsg.head.from;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.to: " << _respMsg.head.to;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.reqType: " << (uint16_t)_respMsg.head.reqType;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.cmdType: " << (uint16_t)_respMsg.head.cmdType;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.cmdCode: " << _respMsg.head.cmdCode;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.seqNo: " << _respMsg.head.seqNo;
    cout << EAP_BLANK_PREFIX(level + 1) << "head.length: " << _respMsg.head.length;
}

void ThreadBase::Pause()
{
    if (THREAD_STATE_INIT != _state) _state = THREAD_STATE_PAUSE;
}

void ThreadBase::Idling()
{
    if (THREAD_STATE_INIT != _state) _state = THREAD_STATE_IDLING;
}

void ThreadBase::Resume()
{
    if (THREAD_STATE_INIT != _state) _state = THREAD_STATE_WORKING;
}

void ThreadBase::Create()
{
    _handle = eap_os_create_thread(_name.c_str(), (Thread_Ob)_entry, NULL, _stackSize, _prior);
    _SaveInfoForRemote();
    if(_coreId != -1)
    {
        eap_os_set_thread_affinity(_handle, _coreId);
    }
}

extern "C"
{

    static int32_t FillReqHead(Thread_Msg_Head* head, uint16_t from, uint16_t cmdCode, uint32_t dataLen)
    {
        CmdprocBase::FillReqHead(*head, from, cmdCode, dataLen);
        if (0 == head->to)
        {
            EAP_LOG_ERROR("FillMsgHead(cmdCode:%d) error!", cmdCode);
            return EAP_FAIL;
        }
        return EAP_SUCCESS;
    }


    void SendAsynMsg2Thread(uint16_t to, uint8_t *buf, uint32_t dataLen)
    {
        if (NULL == buf) return;
        Thread_Msg* msg = (Thread_Msg*)buf;
        CmdprocBase::FillMsgHead(msg->head, UINT16_INVALID, to, dataLen);
        if (EAP_SUCCESS != EapGetSys().GetThreadMgr().SendReqMsg(*msg))
        {
            EAP_LOG_ERROR("SendReqMsg(to:%d) error!", to);
        }
    }

    void SendAsynCmdCode(uint16_t cmdCode)
    {
        Thread_Msg msg = {0};
        SendAsynMsg(cmdCode, (uint8_t *)&msg, 0);
    }

    void SendAsynData(uint16_t cmdCode, uint8_t *data, uint32_t dataLen)
    {
        Thread_Msg msg = {0};
        if (dataLen > CMD_MSG_MAX_LEN)
        {
            EAP_LOG_ERROR("dataLen(%d) is invalid!", dataLen);
            return;
        }
        if (nullptr != data && dataLen != 0) memcpy(msg.buf, data, dataLen);
        SendAsynMsg(cmdCode, (uint8_t *)&msg, dataLen);
    }

    void SendAsynMsg(uint16_t cmdCode, uint8_t *buf, uint32_t dataLen)
    {
        if (NULL == buf) return;
        Thread_Msg* msg = (Thread_Msg*)buf;
        if (EAP_SUCCESS != FillReqHead(&msg->head, UINT16_INVALID, cmdCode, dataLen))
        {
            EAP_LOG_ERROR("FillMsg(cmdCode:%d) error!", cmdCode);
        }
        else if (EAP_SUCCESS != EapGetSys().GetThreadMgr().SendReqMsg(*msg))
        {
            EAP_LOG_ERROR("SendReqMsg(cmdCode:%d) error!", cmdCode);
        }
    }

    void SendSynData(uint16_t from, uint16_t cmdCode, uint8_t* reqData, uint32_t reqDataLen, uint8_t* respData, uint32_t *respLen)
    {
        Thread_Msg reqMsg = {0};
        if (reqDataLen > CMD_MSG_MAX_LEN)
        {
            EAP_LOG_ERROR("reqDataLen(%d) is invalid!", reqDataLen);
            return;
        }
        if (nullptr != reqData && reqDataLen != 0) memcpy(reqMsg.buf, reqData, reqDataLen);
        SendSynMsg(from, cmdCode, (uint8_t *)&reqMsg, reqDataLen, respData, respLen);
    }

    void SendSynMsg(uint16_t from, uint16_t cmdCode, uint8_t* reqBuf, uint32_t reqDataLen, uint8_t* respData, uint32_t *respLen)
    {
        if (NULL == reqBuf) return;
        Thread_Msg* reqMsg = (Thread_Msg*)reqBuf;
        if (EAP_SUCCESS != FillReqHead(&reqMsg->head, from, cmdCode, reqDataLen))
        {
            EAP_LOG_ERROR("FillMsg(cmdCode:%d) error!", cmdCode);
            return;
        }
        EapGetSys().GetThreadMgr().SendAndWaitResp(*reqMsg, respData, *respLen);
    }

    bool IsBig_Endian()
    {
        unsigned short test = 0x1234;
        return (*( (unsigned char*) &test ) == 0x12) ? true : false;
    }//IsBig_Endian()

}

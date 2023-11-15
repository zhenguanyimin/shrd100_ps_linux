#include "json_pb_converter.h"
#include "HeartBeatCmdproc.h"
#include "HeartBeatModule.h"
#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "SysBase.h"

#include "../../srv/wit/WitService.h"
#include "../alink_pb/TracerUavMsg.pb.h"

#include <sys/time.h>
#include <cstdlib>

#if 0
#define DEBUG_HEARTBEAT_PRINTF(arg...) printf(arg)
#else
#define DEBUG_HEARTBEAT_PRINTF(arg...) 
#endif

#ifndef __UNITTEST__
#else
WIT_STATUS_t wit_status(void) {return WIT_STATUS_ENABLE_NO_CONNECT;};
#endif

extern void ShowAllThreadsMsgStats();
HeartBeatCmdproc::HeartBeatCmdproc()
{
    _isWitLinked  = false;
    _isWifiLinked = false;
    _usbLinkedCnt = 0;

    m_c2_heartbeat_tick[TCP_LINK_C2] = 0;
    m_c2_heartbeat_tick[TYPEC_LINK_C2] = 0;
	
    m_c2_tcp_tick[TCP_LINK_C2] = 0;
    m_c2_tcp_tick[TYPEC_LINK_C2] = 0;
	
    m_c2_heartbeat_flag[TCP_LINK_C2] = false;
    m_c2_heartbeat_flag[TYPEC_LINK_C2] = true;

    hearbeat_sync_typec_flag = false;
    _c2HeartBeatStatus = FLAG_BOTH_DISCONNECT;
}

HeartBeatCmdproc::~HeartBeatCmdproc()
{

}

void HeartBeatCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_TARGET_WARNING, (Cmdproc_Func)&HeartBeatCmdproc::_OnTargetWarning);
    Register(EAP_CMD_REFRESH_WORK_STATUS, (Cmdproc_Func)&HeartBeatCmdproc::_OnRefreshWorkStatus);
    Register(EAP_CMD_REFRESH_FAULT_STATUS, (Cmdproc_Func)&HeartBeatCmdproc::_OnRefreshFaultStatus);
    Register(EAP_CMD_SET_COVERT_MODE, (Cmdproc_Func)&HeartBeatCmdproc::_OnSetCovertMode);
    Register(EAP_CMD_SHOW_HEARTBEAT_MODULE, (Cmdproc_Func)&HeartBeatCmdproc::_OnShowHeartBeatModuleCmd);
    Register(EAP_CMD_UPDATE_C2_HEARTBEAT_TIME, (Cmdproc_Func)&HeartBeatCmdproc::_OnUpdateC2HeartBeatTime);
    Register(EAP_CMD_UPDATE_ALINK_TIME, (Cmdproc_Func)&HeartBeatCmdproc::_OnUpdateALinkTime);
}

void HeartBeatCmdproc::_OnTargetWarning(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    if (sizeof(TargetWarning) != inMsg.dataLen) return;
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    TargetWarning* warning = (TargetWarning*)inMsg.data;
    if (nullptr != heartBeatModule)
    {
        heartBeatModule->TargetAppear(warning->scene, warning->targetState);
    }
}

void HeartBeatCmdproc::_OnRefreshWorkStatus(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    if (nullptr != heartBeatModule)
    {
        // TODO: 需要添加打击和定向的逻辑判断处理(暂无定向功能)
        heartBeatModule->SetWorkStatus(*inMsg.data);
    }
}

void HeartBeatCmdproc::_OnRefreshFaultStatus(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    if (nullptr != heartBeatModule)
    {
        heartBeatModule->SetFaultStatus(*inMsg.data);
    }
}

void HeartBeatCmdproc::_OnSetCovertMode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    if (nullptr != heartBeatModule)
    {
        heartBeatModule->SetCovertMode(*inMsg.data);
    }
}

void HeartBeatCmdproc::_OnTimer(uint8_t timerType)
{
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        _ReportHeartBeat();
        /* 连接状态灯由C2心跳报文决定, 物理连接状态可用于将来诊断功能 */
        // _CheckLinkStatus();
    }
    else if (EAP_CMDTYPE_TIMER_2 == timerType)
    {
    	int timeout = _TCPC2TimeOutCheck();
		EAP_LOG_DEBUG("timeout = %d\n", timeout);
        if(timeout)
        {
	        HalMgr* halMgr = EapGetSys().GetHalMgr();
            if (nullptr != halMgr)
            {
				Hal_Data inData = {0, 0};
				uint8_t data = 0;
				
            	if(timeout & (1 << TCP_LINK_C2)){
					//tcp timeout
					//printf("[%s, %d]timeout = %d\n", __func__, __LINE__, timeout);
					data = 1 << TCP_LINK_C2;
				 
            	}

				if(timeout & (1 << TYPEC_LINK_C2)){
					//typec timeout
					//printf("[%s, %d]timeout = %d\n", __func__, __LINE__, timeout);
					data |= 1 << TYPEC_LINK_C2;

				    /* timeout and clear the sync heartbeat flag */
					hearbeat_sync_typec_flag = false;
				    _TypecSetRevHeartBeatFlag(false);
            	}

				inData.length = 1;
				inData.buf = &data;

				EAP_LOG_DEBUG("data = %d\n", data);
				
                halMgr->OnSet(EAP_DRVCODE_C2_HEARTBEAT_TIMEOUT, inData);
            }
        }
        if (_c2HeartBeatStatus)
            _LedLinkStatus(true);
        else
            _LedLinkStatus(false);
    }
    else if(EAP_CMDTYPE_TIMER_3 == timerType)
    {
        ShowAllThreadsMsgStats();
    }
}

void HeartBeatCmdproc::_RequestC2TimeInfo()
{
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    if (nullptr != heartBeatModule)
    {
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = heartBeatModule->FillInfo(msg.buffer, sizeof(msg.buffer) - 2);
        if (-1 == dataLen) return;
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_TIME_INFO);
    }
}

void HeartBeatCmdproc::_ReportHeartBeat()
{
    HeartBeatModule* heartBeatModule = dynamic_cast<HeartBeatModule*>(_module);
    if (nullptr != heartBeatModule)
    {
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = heartBeatModule->FillInfo(msg.buffer, sizeof(msg.buffer) - 2);
        if (-1 == dataLen) return;
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_HEARTBEAT);

        if (ProductSwCfgGet().issupportairborne())
        {
            std::uint8_t buffer[256] = {0};
            const std::uint32_t maxLength = 256;

            protobuf::traceruav::TracerUavMessagePB uavMsgData;

            uavMsgData.set_msgchannel("");
            uavMsgData.set_msgtype(protobuf::traceruav::TRACERUAVMSG_HEART_BEAT);
            uavMsgData.set_msgname("heartbeat");
            uavMsgData.set_msghandle(0);
            uavMsgData.set_msgseq(1);

            std::size_t dataLength = uavMsgData.ByteSizeLong();

            if (dataLength <= maxLength)
            {
                uavMsgData.SerializeToArray(buffer, dataLength);
            }

            alink_msg_t msg = {0};
            msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
            msg.channelId   = UINT16_ALL;
            memcpy(msg.buffer, buffer, dataLength);

            AlinkCmdproc::SendAlinkCmd(msg, dataLength, EAP_ALINK_RPT_TRACER_UAV_MESSAGE);
        }
    }
}

bool HeartBeatCmdproc::_CheckLinkStatus()
{
    //struct timeval t1;
    //gettimeofday(&t1, nullptr);

    std::uint8_t isLinked = 0;

    _isWitLinked = (WIT_STATUS_ENABLE_CONNECTED == wit_status()) ? true : false;

    if (_usbLinkedCnt > 0)
    {
        _usbLinkedCnt--;
    }

    Hal_Data inData = {0, nullptr};
    std::uint8_t buff[2] = {0, 0};
    Hal_Data outData = {2, buff};

    if (EAP_SUCCESS != EapGetSys().GetHalMgr()->OnGet(EAP_DRVCODE_GET_WIFI_STATUS, inData, outData))
    {
        // not log because wifi interface maynot exist
        // EAP_LOG_ERROR("get hardware wifi status failed!\n");
    }

    if (0 != outData.buf[0])
    {
        _isWifiLinked = true;
    }

    /*
    std::cout << "WifiLinked: " << std::boolalpha << _isWifiLinked
              << ", WitLinked: " << _isWitLinked
              << ", usbLinkedCnt: " << _usbLinkedCnt << std::endl;
    */

    if ((_isWitLinked) || (_isWifiLinked) || (_usbLinkedCnt))
    {
        isLinked = 1;
    }

    _LedLinkStatus(isLinked);

    //struct timeval t2;
    //gettimeofday(&t2, nullptr);
    //std::cout << "t1: " << t1.tv_sec << ", " << t1.tv_usec << ", t2: " << t2.tv_sec << ", " << t2.tv_usec << std::endl;
    //t1: 1637344096, 861190, t2: 1637344096, 861441
    //t1: 1637344097, 861187, t2: 1637344097, 861386
    //t1: 1637344098, 861204, t2: 1637344098, 861399
    //t1: 1637344099, 861202, t2: 1637344099, 861412
    //t1: 1637344100, 861145, t2: 1637344100, 861352
    //t1: 1637344101, 861205, t2: 1637344101, 861460
    //t1: 1637344102, 861187, t2: 1637344102, 861398
    //t1: 1637344103, 861191, t2: 1637344103, 861389
    //t1: 1637344104, 861228, t2: 1637344104, 861424
    //t1: 1637344105, 861173, t2: 1637344105, 861367
    //t1: 1637344106, 861187, t2: 1637344106, 861432
    //about 0.2ms

    return isLinked;
}

void HeartBeatCmdproc::_LedLinkStatus(bool isLinked)
{
    std::uint8_t ledBuff[2];
    ledBuff[0] = LED_STATUS_LINK;
    ledBuff[1] = isLinked;

    Hal_Data inLedData = {2, ledBuff};
    if (EAP_SUCCESS != EapGetSys().GetHalMgr()->OnSet(EAP_DRVCODE_SET_LED_STATUS, inLedData))
    {
        EAP_LOG_ERROR("set hardware led status failed!\n");
    }
}

void HeartBeatCmdproc::_OnShowHeartBeatModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    _DoShowModuleInfo();
}

void HeartBeatCmdproc::_OnUpdateC2HeartBeatTime(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	uint16_t channeltype = 0;
	alink_msg_t *pmsg = (alink_msg_t *)inMsg.data;
	channeltype = pmsg->channelType;

	DEBUG_HEARTBEAT_PRINTF("[%s, %d] channeltype = %d\n", __func__, __LINE__, channeltype);
	
	if(EAP_CHANNEL_TYPE_TYPEC & channeltype){
	    /*c2 to tracer by typec*/
	    m_c2_heartbeat_tick[TYPEC_LINK_C2] = eap_get_cur_time_ms();
	    m_c2_heartbeat_flag[TYPEC_LINK_C2] = true;
	    _TypecSetRevHeartBeatFlag(true);
	    _c2HeartBeatStatus |= FLAG_TYPEC_CONNECT;
	}

	if(EAP_CHANNEL_TYPE_TCP & channeltype){
	    /*c2 to trace by tcp*/
	    m_c2_heartbeat_tick[TCP_LINK_C2] = eap_get_cur_time_ms();
	    m_c2_heartbeat_flag[TCP_LINK_C2] = true;
	    _c2HeartBeatStatus |= FLAG_TCP_CONNECT;
	}
    static int32_t time = 0;
    if(time == 0)
    {
        _RequestC2TimeInfo();
        EAP_LOG_DEBUG("HeartBeatCmdproc::_RequestC2TimeInfo()"); 
    }
    time++;
}

void HeartBeatCmdproc::_OnUpdateALinkTime(const Cmdproc_Data&, Cmdproc_Data&)
{
    _usbLinkedCnt = 5;
}

int HeartBeatCmdproc::_TCPC2TimeOutCheck()
{
	int timeout = 0;

	DEBUG_HEARTBEAT_PRINTF("[%s, %d]m_c2_heartbeat_flag[TCP_LINK_C2] = %d\n", __func__, __LINE__, m_c2_heartbeat_flag[TCP_LINK_C2]);
    if(m_c2_heartbeat_flag[TCP_LINK_C2])
    {
        m_c2_tcp_tick[TCP_LINK_C2] = eap_get_cur_time_ms();
        DEBUG_HEARTBEAT_PRINTF("m_c2_tcp_tick is %d m_c2_heartbeat_tick is %d \n",m_c2_tcp_tick[TCP_LINK_C2],m_c2_heartbeat_tick[TCP_LINK_C2]);
        if((m_c2_tcp_tick[TCP_LINK_C2] - m_c2_heartbeat_tick[TCP_LINK_C2]) > EAP_C2_HEARTBEAT_TIMEOUT)
        {
            m_c2_heartbeat_flag[TCP_LINK_C2] = false;
            printf("_Tcp_C2_is Timeout %s:%d.......\n",__FILE__,__LINE__);
            timeout |= 1 << TCP_LINK_C2;
            _c2HeartBeatStatus &= (~FLAG_TCP_CONNECT);
        }
    }
	
	DEBUG_HEARTBEAT_PRINTF("[%s, %d]m_c2_heartbeat_flag[TYPEC_LINK_C2] = %d\n", __func__, __LINE__, m_c2_heartbeat_flag[TYPEC_LINK_C2]);
    if(m_c2_heartbeat_flag[TYPEC_LINK_C2])
    {

		/* the first time to check time out set the time the same and set the flag */
		if(!hearbeat_sync_typec_flag){
			m_c2_tcp_tick[TYPEC_LINK_C2] = eap_get_cur_time_ms();
			m_c2_heartbeat_tick[TYPEC_LINK_C2] = m_c2_tcp_tick[TYPEC_LINK_C2];
			hearbeat_sync_typec_flag = true;
		}
		
        m_c2_tcp_tick[TYPEC_LINK_C2] = eap_get_cur_time_ms();
        DEBUG_HEARTBEAT_PRINTF("m_c2_tcp_tick is %d m_c2_heartbeat_tick is %d \n",m_c2_tcp_tick[TYPEC_LINK_C2],m_c2_heartbeat_tick[TYPEC_LINK_C2]);
        if((m_c2_tcp_tick[TYPEC_LINK_C2] - m_c2_heartbeat_tick[TYPEC_LINK_C2]) > EAP_C2_HEARTBEAT_TIMEOUT)
        {
            //m_c2_heartbeat_flag[TYPEC_LINK_C2] = false;
            //printf("_Typec_C2_is Timeout %s:%d.......\n",__FILE__,__LINE__);
            timeout |= 1 << TYPEC_LINK_C2;
            _c2HeartBeatStatus &= (~FLAG_TYPEC_CONNECT);
        }
    }
    return timeout;
}


void HeartBeatCmdproc::_TypecSetRevHeartBeatFlag(bool flag)
{
       HalMgr* halMgr = EapGetSys().GetHalMgr();
       if (nullptr != halMgr)
       {
               Hal_Data inData = {0, 0};
               uint8_t ustate = (uint8_t)flag;

               inData.length = 1;
               inData.buf = &ustate;

               DEBUG_HEARTBEAT_PRINTF("[%s, %d]ustate = %d\n", __func__, __LINE__, ustate);

               halMgr->OnSet(EAP_DRVCODE_TYPEC_SET_HEARTBEAT_FLAG, inData);
       }
}

void HeartBeatCmdproc::ShowInfo(uint8_t level)
{
    CmdprocBase::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "m_c2_heartbeat_tick[0]: " << (uint32_t)m_c2_heartbeat_tick[TCP_LINK_C2];
    cout << EAP_BLANK_PREFIX(level) << "m_c2_heartbeat_tick[1]: " << (uint32_t)m_c2_heartbeat_tick[TYPEC_LINK_C2];
	
    cout << EAP_BLANK_PREFIX(level) << "m_c2_tcp_tick[0]: " << (uint32_t)m_c2_tcp_tick[TCP_LINK_C2];
    cout << EAP_BLANK_PREFIX(level) << "m_c2_tcp_tick[1]: " << (uint32_t)m_c2_tcp_tick[TYPEC_LINK_C2];
	
    cout << EAP_BLANK_PREFIX(level) << "m_c2_heartbeat_flag[0]: " << (bool)m_c2_heartbeat_flag[TCP_LINK_C2];
    cout << EAP_BLANK_PREFIX(level) << "m_c2_heartbeat_flag[1]: " << (bool)m_c2_heartbeat_flag[TYPEC_LINK_C2];
}

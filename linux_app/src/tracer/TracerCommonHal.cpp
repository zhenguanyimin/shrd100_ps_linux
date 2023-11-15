
#include "TracerCommonHal.h"
#include "TracerMainHalMgr.h"
#include "hal/eth_link/socket/udp_socket.h"
#include "hal/eth_link/eth_link.h"
#include "hal/eth_link/usb_typec/usb_typec.h"
#include "hal/c2_network/c2_network.h"
#include "hal/c2_alink/c2_alink.h"
#include "AlinkCmdproc.h"
#include "hal/remoteid_wifi/RemoteIdWifiThread.h"
#include "json_pb_converter.h"
#include "hal/c2_network/c2_network.h"
#include <math.h>
#include "TracerUavMsg.pb.h"
#include "../srv/bluetooth/bt_gatt_server_info.h"
#include "../srv/bluetooth/bt_gatt_server.h"

#ifndef __UNITTEST__
#include <arpa/inet.h>
#endif
#include "hal/nvram/nvram_cust.h"
#include "hal/nvram/ifaddr.h"

#if 0 
#define DEBUG_ALINK_MSG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_ALINK_MSG_PRINTF(arg...) 
#endif

#define DEBUG_REPORT_ALINK_INFO 1 
#define FLAG_DEBUG_NO_REPORT_ALINK_INFO "/run/media/mmcblk0p1/flag_debug_no_report_alink_info"

int32_t loginFlag = -1;
uint32_t heart_beat_package_count = 0;
extern udp_socket_t sUdpSocket;
uint32_t netProtoType = 0;
extern "C"
{
	char userID[16] = {'\0'};
	static int _isDisableReportAlinkInfo;
	
}

int TracerCommonHal::_GetPlVersion(const Hal_Data& inData, Hal_Data& outData)
{
    Drv_GetPLVersion *version = (Drv_GetPLVersion *)(outData.buf);

	version->PL_Version_date = axi_read_data(PL_VOERSION_VP0); /*set threshold*/
	version->PL_Version_time = axi_read_data(PL_VOERSION_VP1); /*set threshold*/

    return EAP_SUCCESS;
}

int TracerCommonHal::_GetGpioValue(const Hal_Data& inData, Hal_Data& outData)
{
	gpio_ctrl_t *gpio_in = (gpio_ctrl_t *)(inData.buf);
	gpio_ctrl_t *gpio_out = (gpio_ctrl_t *)(outData.buf);

	if (gpio_in->direction == GPIO_INPUT)
		gpio_out->value = Input_GetValue(INPUT_NAME(gpio_in->name));
	else if (gpio_in->direction == GPIO_OUTPUT)
		gpio_out->value = Output_GetValue(OUTPUT_NAME(gpio_in->name));

    return EAP_SUCCESS;
}

int TracerCommonHal::_SetGpioValue(const Hal_Data& inData, Hal_Data& outData)
{
	int num = 0;
	int i;
	gpio_ctrl_t *gpio_in = (gpio_ctrl_t *)(inData.buf);
	num = inData.length/sizeof(gpio_ctrl_t);
	
	EAP_LOG_DEBUG("set gpio num = %d\n", num);
	
	for(i=0; i<num; i++){
		if (gpio_in[i].direction == GPIO_INPUT)
			printf("the input pin %u can't write.\n", gpio_in[i].name);
		else if (gpio_in[i].direction == GPIO_OUTPUT)
			GPIO_OutputCtrl(OUTPUT_NAME(gpio_in[i].name), gpio_in[i].value);
	}

    return EAP_SUCCESS;
}

int TracerCommonHal::_GetRegisterValue(const Hal_Data& inData, Hal_Data& outData)
{
	register_ctrl_t *register_in = (register_ctrl_t *)(inData.buf);
	register_ctrl_t *register_out = (register_ctrl_t *)(outData.buf);

	if (register_in->addr < 0xFFFF)
		register_out->value = axi_read_data(register_in->addr);
	else
		register_out->value = axi_read(register_in->addr);

    return EAP_SUCCESS;
}

int TracerCommonHal::_SetRegisterValue(const Hal_Data& inData, Hal_Data& outData)
{
	register_ctrl_t *register_in = (register_ctrl_t *)(inData.buf);

	if (register_in->addr < 0xFFFF)
		axi_write_data(register_in->addr, register_in->value);
	else
		axi_write(register_in->addr, register_in->value);

    return EAP_SUCCESS;
}


int TracerCommonHal::_SetRemoteIdBufUseFinish(const Hal_Data& inData, Hal_Data& outData)
{
	return EAP_SUCCESS;
}


int TracerCommonHal::_SetRemotIdChn(const Hal_Data& inData, Hal_Data& outData)
{
	Drv_RemoteIdChnSet *req = (Drv_RemoteIdChnSet *)(inData.buf);
	int ret;
#ifdef __UNITTEST__
	ret = 0;
#else
	ret = RemoteIdWifiThread::remoteIdSwitchChannel(req->chn);
#endif
	if(!ret)
    	return EAP_SUCCESS;
	else
		return EAP_FAIL;
}

int TracerCommonHal::_getBoardType(void)
{
	const char *type = ProductHwCfgGet().producthwtype().c_str();
	const std::map<std::string, BOARD_TYPE_E> boardTypeMap = {
		{"SHRD100-T2", BOARD_TYPE_T2},
		{"SHRD100-T3", BOARD_TYPE_T3},
		{"SHRD100-T3_1", BOARD_TYPE_T3_1}
	};

	EAP_LOG_DEBUG("board type = %s\n", type);
	auto it = boardTypeMap.find(type);
	if (it != boardTypeMap.end()) {
		return it->second;
	} else {
		// 默认返回BOARD_TYPE_T3
		return BOARD_TYPE_T3;
	}
}

void TracerCommonHal::Init()
{
	ProductSwCfg &swcfg = ProductSwCfgGet();
	ProductHwCfg &hwcfg = ProductHwCfgGet();
	const PbNetProto &netProto = swcfg.netproto();

	BOARD_TYPE_E type = (BOARD_TYPE_E)_getBoardType();
	EAP_LOG_DEBUG("board type = 0x%x\n", type);
	set_device_type(type);

	EAP_LOG_DEBUG("BUILDTIME:%s%s\n",__DATE__,__TIME__);
    gpio_init();
    hal_init();

	// 频谱侦测版本需要切换天线(默认#2天线接到了RN440给RemoteID用)
	if (ProductSwCfgGet().issupportspectrum() || ProductSwCfgGet().issupportairborne())
	{
		// 与_pe42442Ctrl定义的值相同
		GPIO_OutputCtrl(EMIO_PE42442_V1, 0);
		GPIO_OutputCtrl(EMIO_PE42442_V2, 0);
		GPIO_OutputCtrl(EMIO_PE42442_V3, 1);
	}

	const PbMuteAttr& muteAttr = swcfg.muteattr();
	uint32_t isSupportSwitchVersion = muteAttr.issupportswitchversion();
	if(isSupportSwitchVersion)
	{
		if (Input_GetValue(MIO_MUTE_SWITCH) == 0x00)
		{
			_netProtoType = NET_PROTO_TYPE_ALINK;
		}
		else if(Input_GetValue(MIO_MUTE_SWITCH) == 0x01)
		{
			_netProtoType = NET_PROTO_TYPE_TRACER_UAVCONTROLLER;
		}
	}
	else
	{
		_netProtoType = netProto.netprototype();
	}
	netProtoType = _netProtoType;
	EAP_LOG_DEBUG("eth_link_init\n");
    eth_link_init();
	EAP_LOG_DEBUG("alink_init\n");
    alink_init();
	EAP_LOG_DEBUG("alink_init \n");
	c2_network_init_param init_param = {0};
	init_param.net_proto_type = _netProtoType;
	init_param.typec_tty_dev = hwcfg.devttytypec().c_str();
    c2_network_init(&init_param);
	EAP_LOG_DEBUG("c2_network_init end\n");
}

void TracerCommonHal::TcpConnect(tcp_client_t &tcp)
{
    tcp_client_cfg_t *psCfg;
#ifndef __UNITTEST__
    struct in_addr addr;
#endif

    unique_lock<mutex> lock(_tcpMtx);
    for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
    {
        if (_tcps[i].iSocket == 0)
        {
            _tcps[i] = tcp;
			_tcps[i].bFlagConnect = true;
            psCfg = tcp.psCfg;
#ifndef __UNITTEST__
            addr.s_addr = psCfg->uRemoteIp;
            printf("TCP connect to [%s:%d] iSocket=%d\n", inet_ntoa(addr), psCfg->wRemotePort, tcp.iSocket);
#endif
            break;
        }
    }
}


void TracerCommonHal::TypeCConnect(tcp_client_t &tcp)
{
	_typeC = tcp;
	_typeC.bFlagConnect = true;
}

void TracerCommonHal::TypeCSetRevHeartBeatFlag(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t *pdata = NULL;
	uint8_t data = 0;
	if(inData.length && inData.buf){
		pdata = inData.buf;
		data = *pdata;
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] data = %d, inData.length = %d\n", __func__, __LINE__, data, inData.length);

		if(data){
			_typeC.bFlagRevHeartBeat = true;
		}

		if(!data){
			_typeC.bFlagRevHeartBeat = false;
		}
	}
	return ;
}


void TracerCommonHal::TcpDisconnect(tcp_client_t &tcp)
{
    tcp_client_cfg_t *psCfg;
#ifndef __UNITTEST__
    struct in_addr addr;
#endif
    unique_lock<mutex> lock(_tcpMtx);
    if(RTH_LINK_NET_TCP_CLIENT == tcp.link_net){
	    for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	    {
	        if (_tcps[i].iSocket == tcp.iSocket)
	        {
	            psCfg = tcp.psCfg;
#ifndef __UNITTEST__
	            addr.s_addr = psCfg->uRemoteIp;
	            EAP_LOG_DEBUG("TCP disconnect with [%s:%d] iSocket=%d\n", inet_ntoa(addr), psCfg->wRemotePort, tcp.iSocket);
#endif
	            _tcps[i].iSocket = 0;
				_tcps[i].bFlagConnect = false;
	            break;
	        }
	    }
    }
	
	if(RTH_LINK_USB_TYPEC == tcp.link_net){
		_typeC.iSocket = 0;
		_typeC.bFlagConnect = 0;
	    _typeC.bFlagInit = 0;
	}
	
}

bool TracerCommonHal::TcpOk(tcp_client_t &tcp)
{
	unique_lock<mutex> lock(_tcpMtx);
	if(RTH_LINK_NET_TCP_CLIENT == tcp.link_net){
	    for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	    {
	        if (_tcps[i].iSocket == tcp.iSocket)
	        {
	            return _tcps[i].bFlagConnect;
	        }
	    }
	}
	
	if(RTH_LINK_USB_TYPEC == tcp.link_net){
		return _typeC.bFlagConnect;
	}
	
	return false;
}
int TracerCommonHal::_SendMsgToLinkChannel(const Hal_Data& inData, Hal_Data& outData)
{
	alink_msg_t *msg = (alink_msg_t*)(inData.buf);
	alink_msg_head_t *h = &msg->msg_head;
    (void)h;

	int32_t eRet = 0;
	uint32_t channelType = (uint32_t)msg->channelType;
	_DumpAlinkMsgHead(msg);
    if (EAP_CHANNEL_TYPE_UDP & channelType){
		unique_lock<mutex> lock(_tcpMtx);
		eRet = udp_socket_send( &sUdpSocket, (uint8_t *)&msg->msg_head, inData.length - 4 );
    }
    else if (EAP_CHANNEL_TYPE_TCP & channelType){
		
		unique_lock<mutex> lock(_tcpMtx);
        if (UINT16_ALL == msg->channelId)
        {
            for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
            {
				DEBUG_ALINK_MSG_PRINTF("[%s, %d] _tcps[%d].bFlagConnect = %d, _tcps[%d].iSocket = %d, _tcps[%d].link_net = %d\n", __func__, __LINE__, i, _tcps[i].bFlagConnect, i, _tcps[i].iSocket, i, _tcps[i].link_net);
                if (_tcps[i].bFlagConnect && _tcps[i].iSocket && (_tcps[i].link_net == RTH_LINK_NET_TCP_CLIENT))
                    tcp_client_send( &_tcps[i], (uint8_t *)&msg->msg_head, inData.length - 4 );
            }
        }
        else
        {
            for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
            {
				DEBUG_ALINK_MSG_PRINTF("***(%s:%d)%d:bFlagConnect:%d,iSocket:%d, link_net:%d\n",__FUNCTION__,__LINE__,i,(int)_tcps[i].bFlagConnect,_tcps[i].iSocket, _tcps[i].link_net);
                if (_tcps[i].bFlagConnect && _tcps[i].iSocket && _tcps[i].iSocket == msg->channelId  && (_tcps[i].link_net == RTH_LINK_NET_TCP_CLIENT)) {
					DEBUG_ALINK_MSG_PRINTF("***(%s:%d) %d\n",__FUNCTION__,__LINE__,i);
                    eRet = tcp_client_send( &_tcps[i], (uint8_t *)&msg->msg_head, inData.length - 4 );
                	break;
                }
            }
        }
    }

	if (EAP_CHANNEL_TYPE_TYPEC & channelType){
    
		unique_lock<mutex> lock(_tcpMtx);
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] _typeC.bFlagConnect = %d, _typeC.iSocket = %d, _typeC.link_net = %d, _typeC.bFlagRevHeartBeat = %d\n", __func__, __LINE__, _typeC.bFlagConnect, _typeC.iSocket, _typeC.link_net, _typeC.bFlagRevHeartBeat);
		if(_typeC.bFlagRevHeartBeat && _typeC.bFlagConnect && _typeC.iSocket && (_typeC.iSocket == msg->channelId || UINT16_ALL == msg->channelId)	&& (_typeC.link_net == RTH_LINK_USB_TYPEC)){
			eRet = usb_typec_client_send( &_typeC, (uint8_t *)&msg->msg_head, inData.length - 4 );
		}
		
		if((false == _typeC.bFlagRevHeartBeat) && ((0xEF == msg->msg_head.msgid) || ((0xE5 == msg->msg_head.msgid) && (NET_PROTO_TYPE_TRACER_UAV ==_netProtoType)))){
			DEBUG_ALINK_MSG_PRINTF("[%s, %d\n] send heart beat msg!\n", __func__, __LINE__);
			eRet = usb_typec_client_send( &_typeC, (uint8_t *)&msg->msg_head, inData.length - 4 );
		}
	}

	if(EAP_CHANNEL_TYPE_BLE_BT & channelType){
		printf("[%s, %d]channelType = %d\n", __func__, __LINE__, channelType);
		uint8_t server_if = 0;
		uint16_t servre_handle = 0;
		bt_status_t return_bt;
		bt_uuid_t uuid;
		bt_uuid_t server_uuid;
		int rsp_length = 0;
	
		server_if = gatt_get_server_if();
		uuid = gatt_get_uuid();
		server_uuid = gatt_get_server_uuid();
		
		servre_handle = server_info_get_chara_handle_by_uuid(server_uuid, uuid);
		
		printf("[%s, %d] server_if = 0x%x, servre_handle = 0x%x\n", __func__, __LINE__, server_if, servre_handle);
		
		if(server_if && servre_handle){
			/* inData.length include the channeltype and channelid, not need*/
			rsp_length = inData.length - 2 -2; 
			return_bt = goc_gatt_server_send_response(server_if, 0, 0, 0, servre_handle, 0, 0, rsp_length, (uint8_t *)&(msg->msg_head));
			printf("[%s, %d] return_bt = 0x%x\n", __func__, __LINE__, return_bt);
		}
	}
	DEBUG_ALINK_MSG_PRINTF("link channel type:%hu,channel id:%hu\n",msg->channelType,msg->channelId);

	return eRet;
}

int TracerCommonHal::_LoginToRemoteControlUnit(const Hal_Data& inData, Hal_Data& outData)
{
	int32_t eRet = 0;
	uint32_t dataLen = 0;
	heart_beat_package_count = 0;
	char loginBuf[1024] = {'\0'};
	uint64_t curTick = eap_get_cur_time_ms();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
	sprintf(loginBuf,"\r\n{\"head\":{\"des\":1,\"msg_id\":1000,\"msg_no\":1,\"res\":3,\"timestamp\":%lld,\"version\":1},\
	\"data\":{\"PassWord\":\"autel123\",\"UserName\":\"autel\"}}\r\n" , curTick);
#pragma GCC diagnostic pop

  	dataLen = strlen(loginBuf);
	for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	{
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] _tcps[%d].bFlagConnect = %d, _tcps[%d].iSocket = %d, _tcps[%d].link_net = %d\n", __func__, __LINE__, i, _tcps[i].bFlagConnect, i, _tcps[i].iSocket, i, _tcps[i].link_net);
		if (_tcps[i].bFlagConnect && _tcps[i].iSocket)
		{
			eRet = tcp_client_send( &_tcps[i], loginBuf, dataLen);
			//printf("tcp_client_send msg is %s ret is %d %s:%d\n",loginBuf,eRet,__FILE__,__LINE__);
		}
	}
	return eRet;
}

int TracerCommonHal::_SendHeartBeatToRemoteControlUnit(const Hal_Data& inData, Hal_Data& outData)
{
	int32_t eRet = 0;
	uint32_t dataLen = 0;
	char heartBuf[1024] = {'\0'};
	uint64_t curTick = eap_get_cur_time_ms();
	heart_beat_package_count++;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
	sprintf(heartBuf,"{\"head\":{\"msg_id\":20003,\"msg_no\":1,\"version\":1,\"res\":1,\"des\":3,\"timestamp\":%lld},\
	\"devID\":%s,\"count\":%d}\r\n", curTick, userID,heart_beat_package_count);
#pragma GCC diagnostic pop  

  	dataLen = strlen(heartBuf);

	for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	{
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] _tcps[%d].bFlagConnect = %d, _tcps[%d].iSocket = %d, _tcps[%d].link_net = %d\n", __func__, __LINE__, i, _tcps[i].bFlagConnect, i, _tcps[i].iSocket, i, _tcps[i].link_net);
		if (_tcps[i].bFlagConnect && _tcps[i].iSocket)
		{
			eRet = tcp_client_send( &_tcps[i], heartBuf, dataLen);
		}
	}
	//printf("tcp_client_send heart msg is %s ret is %d %s:%d\n",heartBuf,eRet,__FILE__,__LINE__);
	return eRet;
}

int TracerCommonHal::_SendDroneInfoToRemoteControlUnit(const char* buf, uint32_t len)
{
	int32_t eRet  = -1;
	if(buf == NULL && len <= 0)
	{
		return -1;
	}
	for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	{
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] _tcps[%d].bFlagConnect = %d, _tcps[%d].iSocket = %d, _tcps[%d].link_net = %d\n", __func__, __LINE__, i, _tcps[i].bFlagConnect, i, _tcps[i].iSocket, i, _tcps[i].link_net);
		if (_tcps[i].bFlagConnect && _tcps[i].iSocket)
		{
			eRet = tcp_client_send( &_tcps[i], const_cast<char*>(buf), len);
		}
	}
	return eRet;
}


int TracerCommonHal::_ReportAlinkInfo(const Hal_Data& inData, Hal_Data& outData)
{
	alink_msg_t *msg = (alink_msg_t*)(inData.buf);
	static int count0 = 0;	// 统计0个无人机上报次数
	static int count1 = 0;	// 统计>=1个无人机上报次数
	int count;
	int32_t eRet = 0;
#if DEBUG_REPORT_ALINK_INFO
	static int debug_flag_no_report_alink_info = 0;
	static int debug_flag_no_report_alink_info_inited = 0;
	
	//only check FLAG_DEBUG_NO_REPORT_ALINK_INFO once at startup.
	if(!debug_flag_no_report_alink_info_inited){
		debug_flag_no_report_alink_info_inited = 1;
		if(access(FLAG_DEBUG_NO_REPORT_ALINK_INFO,0) == 0){
			printf("***found  %s, disable report Alink Info\n",FLAG_DEBUG_NO_REPORT_ALINK_INFO);
			debug_flag_no_report_alink_info = 1;
		}
	}

	if(debug_flag_no_report_alink_info){
		return eRet;
	}
#endif	

	if(_isDisableReportAlinkInfo){
		if(
			0xE0 == msg->msg_head.msgid ||
			0xE1 == msg->msg_head.msgid ||
			0xE2 == msg->msg_head.msgid){
			return 0;
		}else if(0xEF == msg->msg_head.msgid){
			printf(">>heartbeat to C2, passed\n");
		}
		else if(0xE5 == msg->msg_head.msgid)
		{
			std::uint8_t *data = msg->buffer;
			std::uint32_t dataLength = (static_cast<std::uint16_t>(msg->msg_head.len_hi) << 8) | msg->msg_head.len_lo;

			protobuf::traceruav::TracerUavMessagePB uavMsgPb;
			bool isParseOK = uavMsgPb.ParseFromArray(data, dataLength);
            (void)isParseOK;
			if(uavMsgPb.msgtype() != protobuf::traceruav::TRACERUAVMSG_HEART_BEAT)
			{
				return 0;
			}
		}
	}

	// printf("TracerCommonHal::_ReportAlinkInfo MSGID:%x\r\n", msg->msg_head.msgid);
	if (0xBC == msg->msg_head.msgid || 0xBD == msg->msg_head.msgid)
	{
		// printf("TracerCommonHal::_ReportAlinkInfo MSGID:%x\r\n", msg->msg_head.msgid);
	}
	else if (0xE0 == msg->msg_head.msgid)	// 打印DroneID上报数据的调试信息
	{
		if (inData.length < sizeof(Rpt_DroneInfo))
			count0++;
		else
			count1++;
		count = count0 + count1;
		if (count++ % 100 == 1)
		{
			printf("\nsend DroneID(0xE0) message to C2: No.%d / count0=%d, count1=%d", count, count0, count1);
			for (std::uint32_t i = 0; i < inData.length; ++i)
			{
				if (i % 10 == 0) printf("\n");
				printf("%02x ", inData.buf[i]);
			}
			printf("\n");
		}
	}
	
	//printf("[%s, %d] _netProtoType:%d\n", __func__, __LINE__, _netProtoType);
	if(_netProtoType == NET_PROTO_TYPE_ALINK){
		eRet = _SendMsgToLinkChannel(inData,outData);
	}else if(_netProtoType == NET_PROTO_TYPE_TRACER_UAV){
		eRet = _SendMsgToLinkChannel(inData,outData);
	}else if(_netProtoType == NET_PROTO_TYPE_TRACER_UAVCONTROLLER){
		alink_msg_t *msg = (alink_msg_t*)(inData.buf);
		uint32_t channelType = (uint32_t)msg->channelType;
		if (EAP_CHANNEL_TYPE_TYPEC & channelType){
			unique_lock<mutex> lock(_tcpMtx);
			DEBUG_ALINK_MSG_PRINTF("[%s, %d] _typeC.bFlagConnect = %d, _typeC.iSocket = %d, _typeC.link_net = %d, _typeC.bFlagRevHeartBeat = %d\n", __func__, __LINE__, _typeC.bFlagConnect, _typeC.iSocket, _typeC.link_net, _typeC.bFlagRevHeartBeat);
			if(_typeC.bFlagRevHeartBeat && _typeC.bFlagConnect && _typeC.iSocket && (_typeC.iSocket == msg->channelId || UINT16_ALL == msg->channelId)  && (_typeC.link_net == RTH_LINK_USB_TYPEC)){
				eRet = usb_typec_client_send( &_typeC, (uint8_t *)&msg->msg_head, inData.length - 4 );
			}

			if((false == _typeC.bFlagRevHeartBeat) && ((0xEF == msg->msg_head.msgid) || (0xE5 == msg->msg_head.msgid))){
				DEBUG_ALINK_MSG_PRINTF("[%s, %d\n] send heart beat msg!\n", __func__, __LINE__);
				eRet = usb_typec_client_send( &_typeC, (uint8_t *)&msg->msg_head, inData.length - 4 );
			}
		}
		if(EAP_CHANNEL_TYPE_TCP & channelType)
		{
			unique_lock<mutex> lock(_tcpMtx);
			if(loginFlag == -1){
				//if not login yet, try to login.
				eRet = _LoginToRemoteControlUnit(inData,outData);
			}else {
				if(msg->msg_head.msgid == 0xEF){
					//send heartbeat
					//printf("send heartbeat to c2\n");
					eRet = _SendHeartBeatToRemoteControlUnit(inData,outData);
				}else if(0xE0 == msg->msg_head.msgid){
					//send drone id info
					//===============report drone info to remote control unit=====================================================================================================
					uint8_t *data = msg->buffer;
					data += EAP_SN_LENGTH;
					uint8_t droneNum = *data;
					//printf("droneNum is %hhd\n",droneNum);
					data++;
					for (int i = 0; i < droneNum; ++i)
					{
						Rpt_DroneInfo* info = (Rpt_DroneInfo *)data;
						char poiltBuf[1024] = {'\0'};
						memset(poiltBuf,0,sizeof(poiltBuf));
						uint64_t timeStamp = eap_get_cur_time_ms();
						int32_t pilotLatitude = (int32_t)((float)(info->pilotLatitude/M_PI)*180);
						int32_t pilotLongitude = (int32_t)((float)(info->pilotLongitude/M_PI)*180);
						int32_t droneLatitude = (int32_t)((float)(info->droneLatitude/M_PI)*180);
						int32_t droneLongitude = (int32_t)((float)(info->droneLongitude/M_PI)*180);

                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wformat="
						sprintf(poiltBuf,"{\"head\":{\"msg_id\":12105,\"version\":1,\"res\":3,\"des\":1,\"timestamp\":%lld},\"devID\":%s,\
						\"data\":{\"protocolType\": \"drone id\",\"pilotLatitude\":%d,\"pilotLongitude\":%d,\"uavLatitude\":%d,\"uavLongitude\":%d,\"uavSN\":\"%s\"\
						,\"uavSpeed\":%d,\"uavHeight\":%d,\"uavName\":\"%s\"}}\r\n", \
						timeStamp,userID, pilotLatitude, pilotLongitude, droneLatitude,droneLongitude,info->serialNum,info->droneSpeed,info->droneHeight,info->droneName);
                    #pragma GCC diagnostic pop

						data += sizeof(Rpt_DroneInfo);
						eRet = _SendDroneInfoToRemoteControlUnit(poiltBuf,strlen(poiltBuf));
						// printf("pilotLatitude %d pilotLongitude %d uavLatitude %d uavLongitude %d [%s:%d]\n"
						// ,pilotLatitude,pilotLongitude,droneLatitude,droneLongitude,__func__,__LINE__);
						// printf("pilotLatitude %d pilotLongitude %d uavLatitude %d uavLongitude %d info->serialNum %s [%s:%d]\n",info->pilotLatitude,
						// info->pilotLongitude,info->droneLatitude,info->droneLongitude,info->serialNum,__func__,__LINE__);
						//printf("poiltBuf is %s [%s:%d]\n",poiltBuf,__func__,__LINE__);
					}					
					//====================================================================================================================
				}else if(0xE1 == msg->msg_head.msgid){
					//===============report remote id info to remote control unit=====================================================================================================
					uint8_t *data = msg->buffer;
					data += EAP_SN_LENGTH;
					uint8_t droneNum = *data;
					data++;
					for (int i = 0; i < droneNum; ++i)
					{
						Rpt_RemoteInfo* info = (Rpt_RemoteInfo *)data;
						char poiltBuf[1024] = {'\0'};
						memset(poiltBuf,0,sizeof(poiltBuf));
						uint64_t timeStamp = eap_get_cur_time_ms();
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wformat="
						sprintf(poiltBuf,"{\"head\":{\"msg_id\":12105,\"version\":1,\"res\":3,\"des\":1,\"timestamp\":%lld},\"devID\":%s,\
						\"data\":{\"protocolType\": \"remote id\",\"pilotLatitude\":%d,\"pilotLongitude\":%d,\"uavLatitude\":%d,\"uavLongitude\":%d,\"uavSN\":\"%s\"\
						,\"uavSpeed\":%d,\"uavHeight\":%d,\"uavName\":\"%s\"}}\r\n", \
						timeStamp,userID, info->droneSailLatitude, info->droneSailLongitude, info->droneLatitude,info->droneLongitude,info->serialNum,info->droneSpeed,info->droneHeight,info->droneName);
                    #pragma GCC diagnostic pop

						data += sizeof(Rpt_RemoteInfo);
						eRet = _SendDroneInfoToRemoteControlUnit(poiltBuf,strlen(poiltBuf));
						// printf("pilotLatitude %d pilotLongitude %d uavLatitude %d uavLongitude %d [%s:%d]\n"
						// ,pilotLatitude,pilotLongitude,droneLatitude,droneLongitude,__func__,__LINE__);
						// printf("pilotLatitude %d pilotLongitude %d uavLatitude %d uavLongitude %d info->serialNum %s [%s:%d]\n",info->pilotLatitude,
						// info->pilotLongitude,info->droneLatitude,info->droneLongitude,info->serialNum,__func__,__LINE__);
						//printf("poiltBuf is %s [%s:%d]\n",poiltBuf,__func__,__LINE__);
					}					
				}
			}
		}

		if(EAP_CHANNEL_TYPE_BLE_BT & channelType){
			unique_lock<mutex> lock(_tcpMtx);
			DEBUG_ALINK_MSG_PRINTF("[%s, %d]channelType = %d\n", __func__, __LINE__, channelType);
			uint8_t server_if = 0;
			uint16_t servre_handle = 0;
			bt_status_t return_bt;
			bt_uuid_t uuid;
			bt_uuid_t server_uuid;
			
			server_if = gatt_get_server_if();
			uuid = gatt_get_uuid();
			server_uuid = gatt_get_server_uuid();
			
			servre_handle = server_info_get_chara_handle_by_uuid(server_uuid, uuid);
			
			DEBUG_ALINK_MSG_PRINTF("[%s, %d] server_if = 0x%x, servre_handle = 0x%x\n", __func__, __LINE__, server_if, servre_handle);
			
			if(server_if && servre_handle){
				return_bt = goc_gatt_server_send_response(server_if, 0, 0, 0, servre_handle, 0, 0, 0, NULL);
				DEBUG_ALINK_MSG_PRINTF("[%s, %d] return_bt = 0x%x\n", __func__, __LINE__, return_bt);
			}
		}
		
	}else{
		printf("!!!!!!!!!!err,unknown net proto type, drop msg.\n");
		//drop msg.
		eRet = -1;
	}
	
	return eRet;
}

int TracerCommonHal::_DumpAlinkMsgHead(alink_msg_t *msg)
{
	alink_msg_head_t *h = &msg->msg_head;
    (void)h;

	EAP_LOG_DEBUG("channelType:%hu,channelId:%hu,msgid:%hhx,(sourceid:%hhx,destid:%hhx),len:%d,magic:%hhx,seq:%hhx\n",msg->channelType,msg->channelId,h->msgid,h->sourceid,h->destid,(((uint32_t)h->len_hi)<<8)+(uint32_t)h->len_lo,h->magic,h->seq);
	return 0;
}



int TracerCommonHal::_C2HeartBeatTimeOutProc(const Hal_Data& inData, Hal_Data& outData)
{
	uint8_t *pdata = NULL;
	uint8_t data = 0;
	DEBUG_ALINK_MSG_PRINTF("%s, %d, inData.length = %d, data = 0x%x\n", __func__, __LINE__, inData.length, *(inData.buf));
	if(inData.length && inData.buf){
		pdata = inData.buf;
		data = *pdata;
	
	    DEBUG_ALINK_MSG_PRINTF("[%s, %d] data = %d, inData.length = %d\n", __func__, __LINE__, data, inData.length);
		if(data & (1 << TCP_LINK_C2)){
			if(_netProtoType == NET_PROTO_TYPE_TRACER_UAVCONTROLLER){
				//FIXME: now PB_NET_PROTO_TYPE_TRACER_UAVCONTROLLER not support receiving heartbeat from uav controller.
				//only run NET_PROTO_TYPE_TRACER_UAVCONTROLLER on tcp link. 
				return 1;
			}
			_SetTcpState(TCP_LINK_C2, false);
		}

		if(data & (1 << TYPEC_LINK_C2)){
			_SetTcpState(TYPEC_LINK_C2, false);
		}
	}
	return 1;
}

TracerCommonHal::TracerCommonHal(HalMgr* halMgr) : HalBase(halMgr)
{
	ProductSwCfg &swcfg = ProductSwCfgGet();
	//ProductHwCfg &hwcfg = ProductHwCfgGet();
	const PbNetProto &netProto = swcfg.netproto();
	_netProtoType = netProto.netprototype();

	_Register(EAP_DRVCODE_GET_PL_VERSION, (Hal_Func)&TracerCommonHal::_GetPlVersion);
	_Register(EAP_DRVCODE_GET_GPIO_VALUE, (Hal_Func)&TracerCommonHal::_GetGpioValue);
	_Register(EAP_DRVCODE_SET_GPIO_VALUE, (Hal_Func)&TracerCommonHal::_SetGpioValue);
	_Register(EAP_DRVCODE_GET_REGISTER_VALUE, (Hal_Func)&TracerCommonHal::_GetRegisterValue);
	_Register(EAP_DRVCODE_SET_REGISTER_VALUE, (Hal_Func)&TracerCommonHal::_SetRegisterValue);
	_Register(EAP_DRVCODE_RPT_INFO_BY_ALINK, (Hal_Func)&TracerCommonHal::_ReportAlinkInfo);
	_Register(EAP_DRVCODE_C2_HEARTBEAT_TIMEOUT, (Hal_Func)&TracerCommonHal::_C2HeartBeatTimeOutProc);
	
	_Register(EAP_DRVCODE_REMOTEID_BUF_USE_FINISH, (Hal_Func)&TracerCommonHal::_SetRemoteIdBufUseFinish);
	_Register(EAP_DRVCODE_REMOTEID_SET_CHANNEL, (Hal_Func)&TracerCommonHal::_SetRemotIdChn);

	
	_Register(EAP_DRVCODE_SEND_ALINK, (Hal_Func)&TracerCommonHal::_SendMsgToLinkChannel);

	_Register(EAP_DRVCODE_TYPEC_SET_HEARTBEAT_FLAG, (Hal_Func)&TracerCommonHal::TypeCSetRevHeartBeatFlag);
	_Register(EAP_DRVCODE_BLUTTOOTH_SET_NETWORK, (Hal_Func)&TracerCommonHal::_BlueToothSetNetwork);
	
	_Register(EAP_DRVCODE_GET_SN, (Hal_Func)&TracerCommonHal::_GetSerialNumber);
	_Register(EAP_DRVCODE_SET_SN, (Hal_Func)&TracerCommonHal::_SetSerialNumber);

	_Register(EAP_DRVCODE_GET_REMOTE_ADDR, (Hal_Func)&TracerCommonHal::_GetRemoteAddress);

	memset(&_tcps, 0, sizeof(_tcps));
	memset(&_typeC, 0, sizeof(_typeC));

	memset(_name, 0, sizeof(_name));
	memset(_secret, 0, sizeof(_name));
	memset(_path, 0, sizeof(_path));
	_auth = 0;
}


void TracerCommonHal::_BlueToothGetPathofCfg()
{
	int length = 0;
	const char *path = "/run/media/mmcblk0p1/wpa_supplicant.conf";
	
   /* config files and flags in mmcblk0p1  */
	length = strlen(path);

	memcpy(_path, path, length);
	_path[length] = '\n';

	
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]_path = %s\n", __func__, __LINE__, _path);
	
	return ;
}

int TracerCommonHal::_BlueToothSetWpaConfig(void)
{
	int status = 0;
	int i = 0;
	FILE *pfile_cfg_file = NULL;
	FILE *pfile_wpa_file = NULL;
	
	char cmd[256] = {0};
	char rm_cmd_cfg[128] = {0};
	char rm_cmd_tmp[128]={0};
	char rm_cmd_tmp_cfg[128]={0};
	
	char cp_cmd_emmc[128] = {0};
	char cp_cmd_etc[128] = {0};
	char cfg_file_content[5][256] ={0};
	
	const char *tmp_path ="/tmp/wpa_supplicant.conf";
	const char *tmp_file ="/tmp/wpa_config";
	const char *etc_path = "/etc/";

	const char *content_0 = "ctrl_interface=/var/run/wpa_supplicant\n";
	const char *content_1 = "ctrl_interface_group=0\n";
	const char *content_2 = "update_config=1\n";

	sprintf(rm_cmd_cfg, "rm -rf %s", _path);
	sprintf(rm_cmd_tmp, "rm -rf %s", tmp_path);
	sprintf(rm_cmd_tmp_cfg, "rm -rf %s", tmp_file);
	
	sprintf(cp_cmd_emmc, "sudo cp %s %s", tmp_path, _path);
	sprintf(cp_cmd_etc, "sudo cp %s %s", tmp_path, etc_path);
	
	sprintf(cmd, "wpa_passphrase %s %s > %s", _name, _secret, tmp_file);

	DEBUG_ALINK_MSG_PRINTF("[%s, %d] cmd = %s\n", __func__, __LINE__, cmd);
	status = system(cmd);
    if (status == -1) {
        printf("Failed to execute command.\n");
        return 1;
    }

	pfile_cfg_file = fopen(tmp_file, "r");
	while (fgets(cfg_file_content[i], sizeof(cfg_file_content[i]), pfile_cfg_file) != NULL) {
		printf("%s", cfg_file_content[i]);
		i++;
	}
	
    fclose(pfile_cfg_file);

    // Open the generated wpa_supplicant.conf file
    pfile_wpa_file = fopen(tmp_path, "a");
    if (pfile_wpa_file == NULL) {
        printf("[%s, %d]Failed to open %s file.\n", __func__, __LINE__, tmp_path);
        return 1;
    }

    // Write additional configuration to the file
    fprintf(pfile_wpa_file, "ctrl_interface=/var/run/wpa_supplicant\n");
    fprintf(pfile_wpa_file, "ctrl_interface_group=0\n");
    fprintf(pfile_wpa_file, "update_config=1\n");
	
	fwrite(cfg_file_content[0], strlen(cfg_file_content[0]), 1, pfile_wpa_file);
	
    fprintf(pfile_wpa_file, "	scan_ssid=1\n");
	
	fwrite(cfg_file_content[1], strlen(cfg_file_content[1]), 1, pfile_wpa_file);
	fwrite(cfg_file_content[2], strlen(cfg_file_content[2]), 1, pfile_wpa_file);
	fwrite(cfg_file_content[3], strlen(cfg_file_content[3]), 1, pfile_wpa_file);
	fwrite(cfg_file_content[4], strlen(cfg_file_content[4]), 1, pfile_wpa_file);

	fclose(pfile_wpa_file);
	
	if(access(_path, F_OK) == 0){
		/* if the wpa already be created */
		status = system(rm_cmd_cfg);
		if (status == -1) {
			printf("[%s, %d]Failed to rm %s\n", __func__, __LINE__, _path);
			return 1;
		} else if (status != 0) {
			printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
			return 1;
		}
	}

	status = system(rm_cmd_tmp_cfg);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cp_cmd_emmc);
		return 1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return 1;
	}
	
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]cp_cmd_emmc = %s\n", __func__, __LINE__, cp_cmd_emmc);
	status = system(cp_cmd_emmc);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cp_cmd_emmc);
		return 1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return 1;
	}

	DEBUG_ALINK_MSG_PRINTF("[%s, %d]cp_cmd_etc = %s\n", __func__, __LINE__, cp_cmd_etc);
	status = system(cp_cmd_etc);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cp_cmd_etc);
		return 1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return 1;
	}
	
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]rm_cmd_tmp = %s\n", __func__, __LINE__, rm_cmd_tmp);
	status = system(rm_cmd_tmp);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, rm_cmd_tmp);
		return 1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return 1;
	}

	return 0;
}

void TracerCommonHal::_BlueTooth_BD_Pro_Analyz(uint8_t *pbuff, int length)
{
	uint8_t *data = NULL;

	if(!pbuff || (length != 68)){
		printf("[%s, %d] parameters err!\n", __func__, __LINE__);
		return ;
	}
	data = pbuff;
	memcpy(_name, data, 32);
	_name[32]='\n';
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]_name = %s\n", __func__, __LINE__, _name);
	
	data += 32;
	memcpy(_secret, data, 32);
	_secret[32] = '\n';
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]_secret = %s\n", __func__, __LINE__, _secret);

	data +=34;
	_auth = *data;
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]_auth = %d\n", __func__, __LINE__, _auth);
	
	return;
}

int TracerCommonHal::_BlueToothCreateWifiConnectService()
{
	int status = 0;
	FILE *pfile = NULL;
	char rm_cmd_tmp[128]={0};
	char cp_cmd_etc[128] = {0};
	char cp_cmd_emm[128] = {0};
	
	const char *interface_wifi_path ="/tmp/ifname_wifi";
	const char *cfg_path_emm = "/run/media/mmcblk0p1/";
	
	char interface_name[20] = {0};
	const char *etc_path = "/etc/systemd/system";
	const char *service_0 = "ExecStart=/usr/sbin/wpa_supplicant -D nl80211 -i ";
	const char *service_1 = " -c /etc/wpa_supplicant.conf";
	const char *service_2 = "ExecStartPost=/sbin/udhcpc -i ";
	const char *wifi_connect_cfg_tmp_path = "/tmp/wifi-connection.service";

	const char *content_0 = "[Unit]";
	const char *content_1 = "Description=WPA Supplicant and udhcpc Service";
	const char *content_2 = "After=network.target";
	const char *content_3 = "[Service]";
	char content_4[128] = {0};
	char content_5[128] = {0};
	const char *content_6 = "ExecStop=/usr/bin/killall wpa_supplicant udhcpc";
	const char *content_7 = "[Install]";
	const char *content_8 = "WantedBy=multi-user.target";

	if(access(interface_wifi_path, F_OK) == 0){
	    pfile = fopen(interface_wifi_path, "r");
		if(!pfile){
			printf("[%s,%d] open file failed!\n", __func__, __LINE__);
			return -1;
		}
	}

	if(!fgets(interface_name, sizeof(interface_name), pfile)){
		printf("[%s,%d] read file failed!\n", __func__, __LINE__);
		fclose(pfile);
		return -1;
	}

	DEBUG_ALINK_MSG_PRINTF("[%s, %d]interface_name = %s\n", __func__, __LINE__, interface_name);

	sprintf(content_4, "%s%s%s", service_0, interface_name, service_1);
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]content_4 = %s\n", __func__, __LINE__, content_4);

	sprintf(content_5, "%s%s", service_2, interface_name);
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]content_5 = %s\n", __func__, __LINE__, content_5);


	sprintf(rm_cmd_tmp, "rm -rf %s", wifi_connect_cfg_tmp_path);
	sprintf(cp_cmd_etc, "sudo cp %s %s", wifi_connect_cfg_tmp_path, etc_path);

	/* config files put int /run/media/mmcblk0p1 */
	sprintf(cp_cmd_emm, "sudo cp %s %s", wifi_connect_cfg_tmp_path, cfg_path_emm);

	pfile = fopen(wifi_connect_cfg_tmp_path, "w");
	if (!pfile) {
		printf("Failed to open file: %s\n", wifi_connect_cfg_tmp_path);
		return 1;
	}
	
	fwrite(content_0, strlen(content_0), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	
	fwrite(content_1, strlen(content_1), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_2, strlen(content_2), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_3, strlen(content_3), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_4, strlen(content_4), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_5, strlen(content_5), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_6, strlen(content_6), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_7, strlen(content_7), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	fwrite(content_8, strlen(content_8), 1, pfile);
	fwrite("\n", strlen("\n"), 1, pfile);
	
	fclose(pfile);


	DEBUG_ALINK_MSG_PRINTF("[%s, %d]cp_cmd_etc = %s\n", __func__, __LINE__, cp_cmd_etc);
	status = system(cp_cmd_etc);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cp_cmd_etc);
		return -1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return -1;
	}

	DEBUG_ALINK_MSG_PRINTF("[%s, %d]cp_cmd_emm = %s\n", __func__, __LINE__, cp_cmd_emm);
	status = system(cp_cmd_emm);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cp_cmd_emm);
		return -1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return -1;
	}
	
	DEBUG_ALINK_MSG_PRINTF("[%s, %d]rm_cmd_tmp = %s\n", __func__, __LINE__, rm_cmd_tmp);
	status = system(rm_cmd_tmp);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, rm_cmd_tmp);
		return -1;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return -1;
	}

	return 0;
}


void TracerCommonHal::_BlueToothSetWifiConnectServer()
{
	int status = 0;
	const char *blk1_path = "/run/media/mmcblk0p1/wifi-connection.service";
	const char *dir_cfg = "/etc/systemd/system/";
	char cmd_wifi_cfg[128] = {0};
	const char *path = "/etc/systemd/system/wifi-connection.service";

	if(access(path, F_OK) == 0){
		/* if the file already be created */
		printf("[%s, %d] %s is already exist!\n", __func__, __LINE__, path);
		return ;
	}

	/* config files put into blk1 */
	if (access(blk1_path, F_OK) == 0) {
		/* wifi connect server file in mmcblkp1 */
		sprintf(cmd_wifi_cfg, "sudo cp %s %s", blk1_path, dir_cfg);
		printf("[%s, %d]cmd_wifi_cfg = %s\n", __func__, __LINE__, cmd_wifi_cfg);
	
	}else{
	    /* this file should be create */
		status = _BlueToothCreateWifiConnectService();
		if (status != 0) {
			printf("[%s, %d]Failed to create wifi connect file!\n", __func__, __LINE__);
			return ;
		}
		return ;
	}

	status = system(cmd_wifi_cfg);
	if (status == -1) {
		printf("[%s, %d]Failed to %s\n", __func__, __LINE__, cmd_wifi_cfg);
		return ;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return ;
	}
}

void TracerCommonHal::_BlueToothDisableWifiNetwork()
{
	int status = 0;
	const char *disable_wifi_cmd = "sudo systemctl stop wifi-connection";
	const char *wifi_service_path = "/etc/systemd/system/wifi-connection.service";

	if(access(wifi_service_path, F_OK) == 0){
		status = system(disable_wifi_cmd);
		if (status == -1) {
			printf("[%s, %d]Failed to sudo systemctl stop wifi-connection\n", __func__, __LINE__);
			return ;
		} else if (status != 0) {
			printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
			return ;
		}
	}
	
}

void TracerCommonHal::_BlueToothEnableWifiNetwork()
{
	int status = 0;
	const char *deamon_cmd = "sudo systemctl daemon-reload";
	const char *enable_wifi_cmd = "sudo systemctl start wifi-connection";

	status = system(deamon_cmd);
	if (status == -1) {
		printf("[%s, %d]Failed to sudo systemctl daemon-reload\n", __func__, __LINE__);
		return ;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return ;
	}

	status = system(enable_wifi_cmd);
	if (status == -1) {
		printf("[%s, %d]Failed to sudo systemctl start wifi-connection\n", __func__, __LINE__);
		return ;
	} else if (status != 0) {
		printf("[%s, %d]Command failed with exit status %d\n", __func__, __LINE__, status);
		return ;
	}
}

int TracerCommonHal::_BlueToothSetNetwork(const Hal_Data& inData, Hal_Data& outData)
{
	int ret = 0;
	uint8_t *pdata = NULL;
	uint16_t data_length = 0;
	
	if(inData.length && inData.buf){
		pdata = inData.buf;
		data_length = inData.length;

		unique_lock<mutex> lock(_buletoothMtx);
		
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] analyze the BD pro data!\n", __func__, __LINE__);
		
		_BlueTooth_BD_Pro_Analyz(pdata, data_length);

		DEBUG_ALINK_MSG_PRINTF("[%s, %d] disable wifi service!\n", __func__, __LINE__);
		/* disable wifi network */
		_BlueToothDisableWifiNetwork();

		DEBUG_ALINK_MSG_PRINTF("[%s, %d] get the path of wpa cfg!\n", __func__, __LINE__);
		_BlueToothGetPathofCfg();

		ret = _BlueToothSetWpaConfig();
		if(ret){
			printf("[%s, %d] set wpa config failed!\n", __func__, __LINE__);
			return ret;
		}

		_BlueToothSetWifiConnectServer();
		
		DEBUG_ALINK_MSG_PRINTF("[%s, %d] enable the wifi service!\n", __func__, __LINE__);
		/* enable wifi network */
		_BlueToothEnableWifiNetwork();

	}
	return ret;
}


void TracerCommonHal::_SetTcpState(link_c2_t link_type, bool state)
{	
	EAP_LOG_DEBUG("link_type = %d, state = %d\n", link_type, state);
	if(link_type == TCP_LINK_C2){
	    for (int i = 0; i < TCP_MAX_CONNECT_NUM; ++i)
	    {
	        if (_tcps[i].iSocket && (_tcps[i].link_net == RTH_LINK_NET_TCP_CLIENT))
	        	_tcps[i].bFlagConnect = state;
	    }
	}
	
	if(link_type == TYPEC_LINK_C2)
			_typeC.bFlagConnect = state;
}

int TracerCommonHal::_GetSerialNumber(const Hal_Data& inData, Hal_Data& outData)
{
    if (outData.buf == nullptr || outData.length <= 0)
    {
        return EAP_FAIL;
    }
    if (0 >= nvram_read_sn(outData.buf, &(outData.length)))
    {
        return EAP_FAIL;
    }
    return EAP_SUCCESS;
}

int TracerCommonHal::_SetSerialNumber(const Hal_Data& inData, Hal_Data& outData)
{
    if (inData.buf == nullptr || inData.length <= 0)
    {
        return EAP_FAIL;
    }
    if (0 >= nvram_write_sn(inData.buf, inData.length))
    {
        return EAP_FAIL;
    }
    return EAP_SUCCESS;
}

int TracerCommonHal::_GetRemoteAddress(const Hal_Data& inData, Hal_Data& outData)
{
    if (outData.buf == nullptr || outData.length <= 0)
    {
        return EAP_FAIL;
    }

    int result = get_ifaddr_wifi(outData.buf, outData.length);
    if (0 >= result)
    {
        return EAP_FAIL;
    }
    outData.length = result;
    return EAP_SUCCESS;
}

extern "C"
{
	void AlinkTcpConnect(tcp_client_t *tcp)
	{    
		TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
		//printf("[%s:%d]halMgr:%p\n",__FUNCTION__,__LINE__,halMgr);
		if (nullptr != halMgr)
		{
			TracerCommonHal *commHal = halMgr->GetCommHal();
			//printf("[%s:%d]commHal:%p\n",__FUNCTION__,__LINE__,commHal);
			if (nullptr != commHal)
			{
				if (RTH_LINK_NET_TCP_CLIENT == tcp->link_net) commHal->TcpConnect(*tcp);
				else if (RTH_LINK_USB_TYPEC == tcp->link_net) commHal->TypeCConnect(*tcp);
			}
		}
	}

	void AlinkTcpDisconnect(tcp_client_t *tcp)
	{
		TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
		if (nullptr != halMgr)
		{
			TracerCommonHal *commHal = halMgr->GetCommHal();
			if (nullptr != commHal) commHal->TcpDisconnect(*tcp);
		}
	}

	bool AlinkTcpStatusIsOk(tcp_client_t *tcp)
	{
		TracerMainHalMgr *halMgr = dynamic_cast<TracerMainHalMgr *>(EapGetSys().GetHalMgr());
		if (nullptr != halMgr)
		{
			TracerCommonHal *commHal = halMgr->GetCommHal();
			if (nullptr != commHal) return commHal->TcpOk(*tcp);
		}
		return false;
	}


	/*
	*/
	int AlinkDisableReportAlinkInfoSet(int isDisable)
	{
		if(_isDisableReportAlinkInfo != isDisable){
			printf("*****_isDisableReportAlinkInfo set to %d\n",isDisable?1:0);
			_isDisableReportAlinkInfo = isDisable;
		}
		return _isDisableReportAlinkInfo;
	}

	
}

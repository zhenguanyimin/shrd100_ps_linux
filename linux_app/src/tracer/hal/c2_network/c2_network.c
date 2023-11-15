#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "eap_pub.h"
#include "c2_network.h"
#include "../eth_link/eth_link.h"
#include "../eth_link/socket/tcp_client.h"
#include "../c2_alink/c2_alink.h"
#include "../c2_alink/check/alink_check.h"

#define TYPEC_DEV_DEFAULT "/dev/ttyGS0"


extern void AlinkTcpConnect(tcp_client_t *tcp);
extern void AlinkTcpDisconnect(tcp_client_t *tcp);
extern int32_t loginFlag;
#if 0
#define DEBUG_ALINK_MSG_RCV_PRINTF(arg...) printf(arg)
#else
#define DEBUG_ALINK_MSG_RCV_PRINTF(arg...) 
#endif

//can store at least two msgs in buf.
#define ALINK_MSG_BUF_MAX (64*1024 + 64*1024 + 1024)
#define ALINK_MSG_RCV_TIMEOUT_MS (5000)
#define ALINK_MSG_HEAD_SIZE sizeof(alink_msg_head_t)

extern char userID[16];
typedef struct c2_network
{
	alink_send_list_t sSendAlinkHeartBeat;
	alink_send_list_t sSendAlinkTracked;
	alink_send_list_t sSendAlinkBeamScheduling;
	alink_send_list_t sSendAlinkAttitude;
	alink_send_list_t sSendAlinkCalibriteData;
	alink_socket_t sAlinkSocket;
	
	eth_link_cbk_t	sCbk;
	eth_link_user_t *psUser;

	uint32_t buf_len;//how many bytes cached in buf.
	uint64_t msg_rcv_start_tick;
	uint64_t msg_rcv_stat_count;
	char buf[ALINK_MSG_BUF_MAX];//cached data for alink msg.
	uint32_t msg_len;//the msg_len of the cached msg which is waiting for payload data
	
}c2_network_t;

static c2_network_init_param _sg_init_param;

static inline void _c2_network_reset_buf(c2_network_t *psNetwork)
{	psNetwork->buf_len = 0; 
	psNetwork->msg_rcv_start_tick = 0;
	psNetwork->msg_len = 0;
}

int32_t c2_network_send( void* handle, uint8_t *pbyData, uint32_t uLen )
{
	eth_link_user_t *psUser = (eth_link_user_t*)handle;
	return	psUser->pi_send( psUser->psSocket, pbyData, uLen );
}

static void c2_network_cbk_connect(void *psHandle)
{
	c2_network_t *psNetwork = (c2_network_t *)psHandle;
	if (psNetwork && psNetwork->psUser)
	{
		tcp_client_t *sClient = (tcp_client_t *)psNetwork->psUser->psSocket;
		if (sClient)
		{
			AlinkTcpConnect(sClient);
			
			EAP_LOG_DEBUG("[%s %d] connect: set bFlagConnect=%d, iSocket=%d\n", __func__, __LINE__, sClient->bFlagConnect, sClient->iSocket);
		}
	}
}

static bool c2_network_cbk_tcp_Isok(void *psHandle)
{
	c2_network_t *psNetwork = (c2_network_t *)psHandle;
	if (psNetwork && psNetwork->psUser)
	{
		tcp_client_t *sClient = (tcp_client_t *)psNetwork->psUser->psSocket;
		if (sClient)
		{
			return AlinkTcpStatusIsOk(sClient);
		}
	}
	return false;
}

static void remote_control_network_cbk_data(void *psHandle,void *psClient,int32_t (*pi_recv)(void *psClient,void *pvBuffer,uint32_t uSize))
{
	c2_network_t *psNetwork = (c2_network_t*)psHandle;
	char *buf  = psNetwork->buf;
	uint32_t buf_len = psNetwork->buf_len;
	uint32_t left = sizeof(psNetwork->buf) - buf_len; //left space for receiving the comming msg.
	uint64_t cur_tick = eap_get_cur_time_ms();
	int is_msg_timeout = (buf_len && psNetwork->msg_rcv_start_tick && (cur_tick - psNetwork->msg_rcv_start_tick) > ALINK_MSG_RCV_TIMEOUT_MS)?1:0;
	int i = 0;
	ssize_t bytesRead = 0;
	//check if prev msg receiving timeout. should clear buffer if timeout.
	if(is_msg_timeout) {
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,err,msg_timeout (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		_c2_network_reset_buf(psNetwork);
	}
	//rcv data,we don't care if socket closed or not, it's lower link's responsibility to handle the abnormal status.
	bytesRead = pi_recv( psClient, buf + buf_len, left);
	if (bytesRead == 0) {
        // 连接已经关闭
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,err,bytesRead==0 socket closed (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		_c2_network_reset_buf(psNetwork);
		return;
    }else if(bytesRead < 0){
        // 接收出现错误，可以根据errno进行错误处理
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 接收缓冲区为空，稍后再试
        } else if (errno == EINTR) {
            // 接收被中断，可以重新尝试接收
        } else {
            // 其他类型的错误
			EAP_LOG_ERROR("*****[%s:%d]rcv buf:%p,err,bytesRead< 0 (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
			_c2_network_reset_buf(psNetwork);
        }
		return ;
    }
	if(!buf_len){
		//a new msg comming.
		psNetwork->msg_rcv_start_tick = cur_tick;
	}
	buf_len += bytesRead;
	psNetwork->buf_len = buf_len;
	psNetwork->msg_rcv_stat_count++;

	DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]*****rcv data:***rcv buf:%p,(bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
	if(psNetwork->msg_len){
		//waiting msg payload comming. 
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,waiting msg data: (buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		if(buf_len < psNetwork->msg_len){
			EAP_LOG_DEBUG("*****[%s:%d]rcv buf:%p, lack of msg data:(buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
			return;
		}
	}else if(buf_len < ALINK_MSG_HEAD_SIZE){
		//waiting msg header.
		EAP_LOG_DEBUG("*****[%s:%d]rcv buf:%p,waiting msg header: buf_len:%d/(msg_header_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,ALINK_MSG_HEAD_SIZE, psNetwork->msg_rcv_stat_count);		
		return;
	}
	if(psNetwork->buf)
	{
		// char one_msg[512] = {0};
		// int package_end_flag = 0;
		// while(1)
		// {
		// 	if(psNetwork->buf[i] == '\r' && psNetwork->buf[i+1] == '\n')
		// 	{
		// 		package_end_flag++;
		// 		break;
		// 	}
		// 	package_end_flag++;
		// }
		// if(package_end_flag > 0)
		// {
		// 	memcpy(one_msg,psNetwork->buf,package_end_flag);
		// 	printf("recv psNetwork->buf is %s %s:%s:%d\n",psNetwork->buf,__FILE__,__func__,__LINE__);
		// 	//memset(psNetwork->buf,0,ALINK_MSG_BUF_MAX);
		// 	package_end_flag = 0;
		// }
		if(psNetwork->buf)
		{
			//printf("recv msg %s\n",psNetwork->buf);
		}
		
		char *result = strstr(psNetwork->buf, "\"Code\":2001");
		int userid = 0;
		if(result != NULL)
		{
			char* start = strstr(psNetwork->buf, "\"UserID\":\"");

			if (start != NULL) 
			{
				start += strlen("\"UserID\":\"");
				char* end = strchr(start, '\"');
				if (end != NULL) {
				int length = end - start;
				strncpy(userID, start, length);
			    }	
			}
			if(loginFlag == -1)
			{
				//printf("login success recv userid is %s psNetwork->buf is %s %s:%s:%d\n",userID,psNetwork->buf,__FILE__,__func__,__LINE__);
			}
			loginFlag = 1;
		}
		char *resultError = strstr(psNetwork->buf, "\"Code\":-1001");
		if(resultError != NULL)
		{
			loginFlag = -1;
			//printf("connect disconnect need reconnect......\n");
		}
	}	
}

static void c2_network_cbk_data(void *psHandle,void *psClient,int32_t (*pi_recv)(void *psClient,void *pvBuffer,uint32_t uSize))
{
	c2_network_t *psNetwork = (c2_network_t*)psHandle;

	//huaguoqing modify,2023.08.19
	alink_msg_head_t *msg_head = NULL;
	char *buf  = psNetwork->buf;
	uint32_t buf_len = psNetwork->buf_len;
	uint32_t left = sizeof(psNetwork->buf) - buf_len; //left space for receiving the comming msg.
	uint64_t cur_tick = eap_get_cur_time_ms();
	int is_msg_timeout = (buf_len && psNetwork->msg_rcv_start_tick && (cur_tick - psNetwork->msg_rcv_start_tick) > ALINK_MSG_RCV_TIMEOUT_MS)?1:0;
	int i = 0;
	ssize_t bytesRead = 0;
	//check if prev msg receiving timeout. should clear buffer if timeout.
	if(is_msg_timeout) {
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,err,msg_timeout (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		_c2_network_reset_buf(psNetwork);
	}
	//rcv data,we don't care if socket closed or not, it's lower link's responsibility to handle the abnormal status.
	bytesRead = pi_recv( psClient, buf + buf_len, left);
	if (bytesRead == 0) {
        // 连接已经关闭
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,err,bytesRead==0 socket closed (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		_c2_network_reset_buf(psNetwork);
		return;
    }else if(bytesRead < 0){
        // 接收出现错误，可以根据errno进行错误处理
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 接收缓冲区为空，稍后再试
        } else if (errno == EINTR) {
            // 接收被中断，可以重新尝试接收
        } else {
            // 其他类型的错误
			EAP_LOG_DEBUG("*****[%s:%d]rcv buf:%p,err,bytesRead< 0 (bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
			_c2_network_reset_buf(psNetwork);
        }
		return ;
    }

	if(!buf_len){
		//a new msg comming.
		psNetwork->msg_rcv_start_tick = cur_tick;
	}
	buf_len += bytesRead;
	psNetwork->buf_len = buf_len;
	psNetwork->msg_rcv_stat_count++;

	DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]*****rcv data:***rcv buf:%p,(bytesRead:%d->buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf,bytesRead,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
	if(psNetwork->msg_len){
		//waiting msg payload comming. 
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,waiting msg data: (buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
		if(buf_len < psNetwork->msg_len){
			EAP_LOG_DEBUG("*****[%s:%d]rcv buf:%p, lack of msg data:(buf_len:%d/(msg_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,psNetwork->msg_len, psNetwork->msg_rcv_stat_count);		
			return;
		}
	}else if(buf_len < ALINK_MSG_HEAD_SIZE){
		//waiting msg header.
		EAP_LOG_DEBUG("*****[%s:%d]rcv buf:%p,waiting msg header: buf_len:%d/(msg_header_len:%d), msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__, buf,buf_len,ALINK_MSG_HEAD_SIZE, psNetwork->msg_rcv_stat_count);		
		return;
	}

	
	//parse msgs. maybe multi msgs in buf; maybe half msg;
	msg_head = (alink_msg_head_t *)buf;
	while(msg_head){
		int payload_len;
		int cached_data_len = buf_len - (int)(((char*)msg_head) - buf);
		int msg_len;
		bool is_head_crc_ok = true,is_data_crc_ok = true;

		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p,  %d :msg_head:%p,cached_data_len:%d, msg_rcv_stat_count:%llu\n",__FUNCTION__,__LINE__,buf, i, msg_head,cached_data_len,psNetwork->msg_rcv_stat_count);		
		if(cached_data_len < ALINK_MSG_HEAD_SIZE){
			//need more data for head.
			DEBUG_ALINK_MSG_RCV_PRINTF("*****************[%s:%d]rcv buf:%p,  header lack data,msg_head:%p,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,cached_data_len);		
			if((char*)msg_head != buf){
				EAP_LOG_DEBUG("*[%s:%d]rcv buf:%p, msg not at begin,move to begin ,msg_head:%p,msg_len:%d,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,msg_len,cached_data_len);	
				memmove(buf,(char*)msg_head,cached_data_len);
				psNetwork->msg_rcv_start_tick = cur_tick;
				
			}
			psNetwork->buf_len = cached_data_len;
			psNetwork->msg_len = 0;
			return;
		}

		//check header crc.
		is_head_crc_ok = alink_check_header_checksum(msg_head);
		if(is_head_crc_ok != true){
			//crc err,drop cached data.
			EAP_LOG_DEBUG("<<<<*****[%s:%d]rcv buf:%p, header crc err, msg_head:%p,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,cached_data_len);		
			_c2_network_reset_buf(psNetwork);
			return;
		}
		
		//check if ready of payload data. should add the length of  data crc.
		payload_len = msg_head->len_hi << 8 | msg_head->len_lo;
		msg_len = ALINK_MSG_HEAD_SIZE + payload_len + sizeof(alink_data_crc_t);
		
		DEBUG_ALINK_MSG_RCV_PRINTF("*****[%s:%d]rcv buf:%p, msg_head:%p, msgid:%hhx,payload_len:%d,msg_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,msg_head->msgid, payload_len,msg_len);		
		if(cached_data_len < msg_len){
			//msg not ready. if msg not the first msg, move it to start of buf.
			DEBUG_ALINK_MSG_RCV_PRINTF("*****************[%s:%d]rcv buf:%p, msg lack data, msg_head:%p,msg_len:%d,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,msg_len,cached_data_len); 	
			if((char*)msg_head != buf){
				EAP_LOG_DEBUG("*[%s:%d]rcv buf:%p, msg not at begin,move to begin msg_head:%p,msg_len:%d,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,msg_len,cached_data_len);	
				memmove(buf,(char*)msg_head,cached_data_len);
				psNetwork->msg_rcv_start_tick = cur_tick;
				
			}
			psNetwork->buf_len = cached_data_len;
			psNetwork->msg_len = msg_len;
			return;
		}

		//msg ready
		//check msg data crc
		is_data_crc_ok = alink_check_crc((char*)msg_head, msg_len);
		if(is_data_crc_ok != true){
			//crc err,drop cached data.
			EAP_LOG_ERROR("<<<<<<<<<*****[%s:%d]rcv buf:%p, data crc err: msg_head:%p,cached_data_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,cached_data_len);		
			_c2_network_reset_buf(psNetwork);
			return;
		}

		//send msg output.
		//using socketfd as channel id.
		tcp_client_t *sClient = (tcp_client_t *)psNetwork->psUser->psSocket;
		if(sClient && RTH_LINK_USB_TYPEC == sClient->link_net){
			send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_TYPEC, (uint16_t)sClient->iSocket, (uint8_t*)msg_head, ALINK_MSG_HEAD_SIZE + payload_len);
		}

		if(sClient && RTH_LINK_NET_TCP_CLIENT == sClient->link_net){
			send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_TCP, (uint16_t)sClient->iSocket, (uint8_t*)msg_head, ALINK_MSG_HEAD_SIZE + payload_len);
		}

		EAP_LOG_DEBUG("msg_head->msgid = 0x%x, sClient->link_net = %d\n", msg_head->msgid, sClient->link_net);
		
		DEBUG_ALINK_MSG_RCV_PRINTF("*****>>>>>>[%s:%d]rcv buf:%p, sendmsg to eap, msg_head:%p, msgid:%hhx,payload_len:%d,msg_len:%d\n",__FUNCTION__,__LINE__,buf, msg_head,msg_head->msgid, payload_len,msg_len);		
		//next msg.
		cached_data_len -= msg_len;
		if(cached_data_len){
			//more msg in buf
			msg_head = (alink_msg_head_t *)(((char*)msg_head)  + msg_len);
			psNetwork->msg_rcv_start_tick = cur_tick;
			psNetwork->msg_len = 0;
		}else{
			//already consumed all msg. buf empty now.
			_c2_network_reset_buf(psNetwork);
			msg_head = NULL;
			break;
		}
		i++;
	}

	//printf("recv c2 data value is %d len is %d %s:%d=============\n",abyBuffer[9],iLength,__FILE__,__LINE__);
}

static void c2_network_cbk_data_by_net_proto_type(void *psHandle,void *psClient,int32_t (*pi_recv)(void *psClient,void *pvBuffer,uint32_t uSize))
{
	c2_network_t *psNetwork = (c2_network_t*)psHandle;
	tcp_client_t *sClient = (tcp_client_t *)psNetwork->psUser->psSocket;

	if(_sg_init_param.net_proto_type == NET_PROTO_TYPE_ALINK ){
		c2_network_cbk_data(psHandle,psClient,pi_recv);
	}else if (_sg_init_param.net_proto_type == NET_PROTO_TYPE_TRACER_UAV) {
		c2_network_cbk_data(psHandle,psClient,pi_recv);
	}else if(_sg_init_param.net_proto_type == NET_PROTO_TYPE_TRACER_UAVCONTROLLER){
		//for NET_PROTO_TYPE_TRACER_UAVCONTROLLER,only run this proto on tcp link;  typec still running NET_PROTO_TYPE_ALINK
		if(sClient && RTH_LINK_USB_TYPEC == sClient->link_net){
			c2_network_cbk_data(psHandle,psClient,pi_recv);
		}else if(sClient && RTH_LINK_NET_TCP_CLIENT == sClient->link_net){
			remote_control_network_cbk_data(psHandle,psClient,pi_recv);
		}	
	}else{
		EAP_LOG_DEBUG("err, unknown net_proto_type:%d\n",_sg_init_param.net_proto_type);
		c2_network_cbk_data(psHandle,psClient,pi_recv);
	}
}

static void c2_network_cbk_disconnect(void *psHandle)
{
	c2_network_t *psNetwork = (c2_network_t *)psHandle;
	if (psNetwork && psNetwork->psUser)
	{
		tcp_client_t *sClient = (tcp_client_t *)psNetwork->psUser->psSocket;
		if (sClient)
		{
			AlinkTcpDisconnect(sClient);
			EAP_LOG_DEBUG("[%s %d] disconnect: set bFlagConnect=%d, iSocket=%d\n", __func__, __LINE__, sClient->bFlagConnect, sClient->iSocket);
		}
	}
}
void c2_network_cbk_delete(void *psHandle)
{
	c2_network_t *psNetwork = (c2_network_t*)psHandle;
	
	EAP_LOG_DEBUG(" delete tcp!\r\n");
	alink_disconnect_send( &psNetwork->sSendAlinkHeartBeat,			0xD0 );
	alink_disconnect_send( &psNetwork->sSendAlinkTracked, 			0x12 );
	alink_disconnect_send( &psNetwork->sSendAlinkBeamScheduling,	0x14 );
	alink_disconnect_send( &psNetwork->sSendAlinkAttitude,			0x13 );
	alink_disconnect_send( &psNetwork->sSendAlinkCalibriteData,		0xCC );

	free( psNetwork );
}

eth_link_cbk_t* c2_network_create( eth_link_user_t *psUser )
{
	c2_network_t *psNetwork = (c2_network_t*)malloc(sizeof(c2_network_t));
	//printf( "[%s %d]\r\n", __func__, __LINE__ );
	if( psNetwork )
	{
		memset( psNetwork, 0, sizeof(c2_network_t) );

        if (NET_PROTO_TYPE_TRACER_UAV == _sg_init_param.net_proto_type)
        {
            alink_connect_send( &psNetwork->sSendAlinkHeartBeat, EAP_ALINK_RPT_HEARTBEAT, ALINK_DEV_ID_UAV, psUser, c2_network_send);
        }
        else
        {
    		alink_connect_send( &psNetwork->sSendAlinkHeartBeat,		0xD0, ALINK_DEV_ID_C2, psUser, c2_network_send );
    		alink_connect_send( &psNetwork->sSendAlinkTracked, 			0x12, ALINK_DEV_ID_C2, psUser, c2_network_send );
    		alink_connect_send( &psNetwork->sSendAlinkBeamScheduling,	0x14, ALINK_DEV_ID_C2, psUser, c2_network_send );
    		alink_connect_send( &psNetwork->sSendAlinkAttitude,			0x13, ALINK_DEV_ID_C2, psUser, c2_network_send );
    		alink_connect_send( &psNetwork->sSendAlinkCalibriteData,	0xCC, ALINK_DEV_ID_C2, psUser, c2_network_send );
        }

		alink_connect_port( &psNetwork->sAlinkSocket, NULL );
		
		psNetwork->psUser = psUser;
		psNetwork->sCbk.psHandle = psNetwork;
		psNetwork->sCbk.pv_cbk_connect		= c2_network_cbk_connect;
		psNetwork->sCbk.pv_cbk_tcp_Isok     = c2_network_cbk_tcp_Isok;
		psNetwork->sCbk.pv_cbk_data 		= c2_network_cbk_data_by_net_proto_type;	
		psNetwork->sCbk.pv_cbk_disconnect	= c2_network_cbk_disconnect;
		psNetwork->sCbk.pv_cbk_delete		= c2_network_cbk_delete;

		return &(psNetwork->sCbk);
	}

	return NULL;
}

int usb_typec_device_ready(void)
{
	int ret = 0;
	const char* deviceNode = _sg_init_param.typec_tty_dev;  // 设备节点路径
	if(!deviceNode || strlen(deviceNode) == 0)
		deviceNode = TYPEC_DEV_DEFAULT;
	
	// 检查设备节点是否存在
	if (access(deviceNode, F_OK) == 0) {
		ret = 1;
	} else {
		ret = 0;
	}

	return ret;
}


int32_t c2_network_init( c2_network_init_param *init_param )
{
	int32_t ret = 0;
	
	_sg_init_param = *init_param;
	EAP_LOG_DEBUG( "[%s %d] netprototype:%d, typecdev:%s\n", __func__, __LINE__ ,_sg_init_param.net_proto_type,_sg_init_param.typec_tty_dev);

	eth_link_add_type( 1, RTH_LINK_NET_TCP_CLIENT, TCP_MAX_CONNECT_NUM, c2_network_create );
#if 1
	ret = usb_typec_device_ready();
	if(ret){
		eth_link_add_type( 2, RTH_LINK_USB_TYPEC, 1, c2_network_create );
	}
#endif
	return 0;
}


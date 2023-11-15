#include "eth_link.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket/udp_socket.h"
#include "socket/tcp_client.h"
#include "../c2_alink/c2_alink.h"
#include "eap_pub.h"
#include "usb_typec/usb_typec.h"
#include "wificfg.h"
#include "polit_tcp_cfg.h"
#include "nvram_cust.h"
#include "spectrum_dump.h"
//#include "../../../eap/include/pub/eap_sys_cmds.h"
#include "eap_sys_cmds.h"
#include "../c2_network/c2_network.h"

#define CFG_DEBUG_CREATE_TCP_CLIENT_IP  

static int    sg_flag_debug_tcp_client_ip = 0;
extern uint32_t netProtoType;
#define FLAG_DEBUG_TCP_CLIENT_IP "/run/media/mmcblk0p1/flag_debug_tcp_client_ip"
static in_addr_t sg_debug_tcp_ipaddr;
static short  sg_debug_tcp_client_port = 0;

static const char* sg_usb_typec_i2c_bus_name = "/dev/i2c-0";
static const uint16_t sg_usb_typec_i2c_dev_addr = 0x3d;
int32_t max_4t_tcp_connect_flag = -1;
static eth_link_list_t *psEthList = NULL;
/*******************************************************************************
自动组网
*******************************************************************************/
// static udp_socket_t sUdpSocket = { 0 };
udp_socket_t sUdpSocket = { 0 };

static volatile udp_socket_cfg_t sUdpCfg = 
{
	.bFlagRecv	= true,
	.bBroadcast	= true,
	.wRecvPort	= 1800,
	.uRecvIp	= INADDR_ANY,
	.wSendPort	= 1810,
	.uSendIp	= 0xFFFFFFFF,//0xC0A801FF, //0xFFFFFFFF,
};

static void eth_link_udp_cbk_init( void* psHandle, void* result )
{
	printf( "[%s %d] %p\r\n", __func__, __LINE__, result );
}

static void eth_link_add_tcp_client_list( eth_link_list_t* psEthList, uint32_t uIp, uint16_t wPort, uint16_t wConnectTime );
static void eth_protocol_handler( void* msg, uint32_t len, uint32_t uIp );
static void eth_link_udp_cbk_data( void	*psHandle, int32_t (*pi_recv)(void *psSocket,void *pvBuffer,uint32_t uSize,uint32_t *puIp) )
{
	uint8_t abyBuffer[64];
	int32_t iLength;
	uint32_t uIp = 0xFFFFFFFF;
	
	iLength = pi_recv( &sUdpSocket, abyBuffer, sizeof(abyBuffer), &uIp );

	sUdpCfg.uSendIp = uIp | 0xFF;
#if 0
	printf( "[%s %d]", __func__, __LINE__ );
	for( int i = 0; iLength > i; ++i )
		printf( " %02X", abyBuffer[i] );
	printf( "\r\n" );
#endif
	eth_protocol_handler( abyBuffer, iLength, uIp );
}

static void eth_link_udp_cbk_delete( void *psHandle )
{
	printf( "[%s %d]\r\n", __func__, __LINE__ );
}

//static uint8_t abyBuffer[64] = { 0 };
static udp_socket_cbk_t sUdpCbk = 
{
	.psHandle = NULL,
	.pv_cbk_init = eth_link_udp_cbk_init,
	.pv_cbk_data = eth_link_udp_cbk_data,
	.pv_cbk_delete = eth_link_udp_cbk_delete,
};

static volatile bool bFlagTick = true;
/*
static int eth_link_broadcast(uint8_t *pbyData, uint32_t uLength)
{
	int eRet = 0;

	if( bFlagTick )
	{
		printf("\n sUdpSocket:");
		for (int i = 0; i < uLength; ++i)
		{
			if (i % 16 == 0) printf("\r\n");
			printf("%x ", pbyData[i]);
		}
		eRet = udp_socket_send( &sUdpSocket, pbyData, uLength );
		eRet = udp_socket_send( &sUdpSocket, pbyData, uLength );
		eRet = udp_socket_send( &sUdpSocket, pbyData, uLength );
		bFlagTick = false;
	}
	return eRet;
}
*/

static void *eth_link_task(void *p_arg)
{
	int eRet = 0;
	
	for( ; ; )
	{
		sleep(1);
		// alink_upload_heartbeat();
		bFlagTick = true;

		if(false == sUdpSocket.bFlagInit){
			eRet = udp_socket_create( &sUdpSocket, &sUdpCfg, &sUdpCbk );
			if(eRet){
				EAP_LOG_ERROR("udp_socket_create failed! eRet = %d\n", eRet);
			}
		}

		if(max_4t_tcp_connect_flag == 0 && netProtoType == NET_PROTO_TYPE_TRACER_UAVCONTROLLER)
		{
			eth_link_list_t *psList = psEthList;
			if( psList->next->type == 1 && sg_flag_debug_tcp_client_ip)
			{
				EAP_LOG_DEBUG( "debug tcp client ip\r\n");
				eth_link_add_tcp_client_list( psList->next, htonl(sg_debug_tcp_ipaddr), sg_debug_tcp_client_port, 0 );
			}
		}

	}
}

int eth_link_init( void )
{
	//int32_t eRet = 0;

    usb_typec_InitDev(sg_usb_typec_i2c_bus_name, sg_usb_typec_i2c_dev_addr);
    usb_typec_SetMode(sg_usb_typec_i2c_bus_name, sg_usb_typec_i2c_dev_addr, TYPEC_MODE_DEVICE);

	wificfg_init();
	polit_tcp_cfg_init();
	spectrum_dump_init();

	if(access(FLAG_DEBUG_TCP_CLIENT_IP,0) == 0){
		int fd;
		size_t ret;
	  	EAP_LOG_DEBUG("***found %s,start a debug tcp client\n",FLAG_DEBUG_TCP_CLIENT_IP);
	    fd = open(FLAG_DEBUG_TCP_CLIENT_IP,O_RDONLY);
		if(fd >= 0){
			char buf[64] = {'\0'};
			ret = read(fd,buf,sizeof(buf)-1);
			if(ret > 0 && strlen(buf) > 0){

				if(buf[ret-1] == '\n'){
					//delete \n if exist.
					buf[ret-1] = 0;
				}
				char ip[16] = {'\0'};
				char port[6] = {'\0'};
				char* colon = strchr(buf,':');
				if(colon == NULL)
				{
					EAP_LOG_ERROR("can't find ip:port\n");
					close(fd);
					return -1;
				}
				strncpy(ip, buf, colon - buf);
				strncpy(port, colon + 1, sizeof(port) - 1);
				if(strlen(ip) > 0 && strlen(port) > 0)
				{
					sg_debug_tcp_client_port = atoi(port);
					EAP_LOG_DEBUG("get ip is %s port is %d\n",ip,sg_debug_tcp_client_port);
					sg_debug_tcp_ipaddr = inet_addr(ip);
					if(sg_debug_tcp_ipaddr != INADDR_NONE)
					{
						sg_flag_debug_tcp_client_ip = 1;
					}
				}
			}
			close(fd);
		}
		
	}
	
	pthread_t pid;
	if( 0 == pthread_create(&pid, NULL, eth_link_task, NULL) )
	{
		pthread_setname_np(pid, "eth_link");
		//move to eth_link_task
		//eRet = udp_socket_create( &sUdpSocket, &sUdpCfg, &sUdpCbk );
	}
	
	return 0;
}

/*******************************************************************************
自动组网处理列表
*******************************************************************************/
int eth_link_add_type( uint16_t type, uint8_t net, uint8_t max, eth_link_cbk_t* (*ps_init)(eth_link_user_t *psUser) )
{
	int eRet = 0;

	if( psEthList )
	{
		eth_link_list_t *psList = psEthList;
		for( ; psList->next; psList = psList->next )
		{
			if( psList->type == type )
			{
				eRet = 1;
				break;
			}
		}

		EAP_LOG_DEBUG( "psList->type = %d, type = %d\r\n", psList->type, type);
		
		if( psList->type != type )
		{
			eth_link_list_t*list = (eth_link_list_t*)malloc( sizeof(eth_link_list_t) );
			memset( list, 0, sizeof(eth_link_list_t) );
			if( list )
			{
				psList->next = (eth_link_list_t*)(list);
				psList->next->type		= type;
				psList->next->net		= net;
				psList->next->max		= max;
				psList->next->ps_init	= ps_init;				
				psList->next->list 		= NULL;
				psList->next->next 		= NULL;

				if( (type != 1) && (RTH_LINK_USB_TYPEC == net) ){
					/*typec*/
					eth_link_add_tcp_client_list( psList->next, 0, 0, 0 );
				}


#ifdef CFG_DEBUG_CREATE_TCP_CLIENT_IP

				if( type == 1 && sg_flag_debug_tcp_client_ip)
				{
					EAP_LOG_DEBUG( "debug tcp client ip\r\n");
					eth_link_add_tcp_client_list( psList->next, htonl(sg_debug_tcp_ipaddr), sg_debug_tcp_client_port, 0 );
				}
#endif

			}
			else
			{
				free( list );
				eRet = -1;
			}
		}
	}
	else
	{
		eth_link_list_t*list = (eth_link_list_t*)malloc( sizeof(eth_link_list_t) );
		memset( list, 0, sizeof(eth_link_list_t) );
		EAP_LOG_DEBUG( "malloc eth link list\r\n");
		if( list )
		{
			psEthList = (eth_link_list_t*)(list);
			psEthList->type		= type;
			psEthList->net		= net;
			psEthList->max		= max;
			psEthList->ps_init	= ps_init;
			psEthList->list		= NULL;
			psEthList->next		= NULL;
#ifdef CFG_DEBUG_CREATE_TCP_CLIENT_IP
			if( type == 1 && sg_flag_debug_tcp_client_ip)
			{
				EAP_LOG_DEBUG( "debug tcp client ip\r\n", __func__, __LINE__ );
				eth_link_add_tcp_client_list( psEthList, htonl(sg_debug_tcp_ipaddr), sg_debug_tcp_client_port, 0 );
			}
#endif

		}
		else
		{
			free( list );
			eRet = -1;
		}
	}

	return eRet;
}

int eth_link_del_type( uint16_t type )
{
	int eRet = 0;
	
	eth_link_list_t *psList = psEthList;
	if( psList )
	{
		eth_link_list_t *psLast = psList;
		for( ; psList; psList = psList->next )
		{
			if( psList->type == type )
			{
				psLast->next = psList->next;
				free( psList );
				break;
			}
			psLast = psList;
		}
	}

	return eRet;
}

/*******************************************************************************
TCP服务端连接列表
*******************************************************************************/
typedef struct eth_link_tcp_client_list
{
	tcp_client_cfg_t sCfg;
	tcp_client_cbk_t sCbk;
	tcp_client_t 	 sClient;
	eth_link_user_t	 sUser;
	eth_link_cbk_t	 *psUserCbk;
	eth_link_list_t	 *psRoot;
	struct eth_link_tcp_client_list *psNext;
}eth_link_tcp_client_list_t;

static void eth_link_del_tcp_client_list( eth_link_tcp_client_list_t *psList )
{
	eth_link_tcp_client_list_t *psIndex = (eth_link_tcp_client_list_t*)psList->psRoot->list;
	if( psList == psIndex )
	{
		psList->psRoot->list = psIndex->psNext;
	}
	else
	{
		eth_link_tcp_client_list_t *psLast = psIndex;
		for( ; psIndex; psIndex = psIndex->psNext )
		{
			if( psList == psIndex )
			{
				psLast->psNext = psIndex->psNext;
				break;
			}
		}
	}
	free( psList );
}

static void eth_link_socket_cbk_connect( void *psHandle )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psHandle;
	if( psList && psList->psUserCbk && psList->psUserCbk->pv_cbk_connect )
		psList->psUserCbk->pv_cbk_connect( psList->psUserCbk->psHandle );
}

static bool eth_link_socket_cbk_tcp_Isok( void *psHandle )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psHandle;
	if( psList && psList->psUserCbk && psList->psUserCbk->pv_cbk_tcp_Isok)
		return psList->psUserCbk->pv_cbk_tcp_Isok( psList->psUserCbk->psHandle );
	return false;
}

static void eth_link_socket_cbk_data( void *psHandle, void *psClient, int32_t (*pi_recv)(void *psClient,void *pvBuffer,uint32_t uSize) )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psHandle;
	if( psList && psList->psUserCbk && psList->psUserCbk->pv_cbk_data )
	{
		psList->psUserCbk->pv_cbk_data( psList->psUserCbk->psHandle, psClient, pi_recv );
	}
	else
	{
		uint8_t abyBuffer[128];
		int32_t iLength = pi_recv( psClient, abyBuffer, sizeof(abyBuffer) );
	}
}

static void eth_link_socket_cbk_disconnect( void *psHandle )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psHandle;
	if( psList && psList->psUserCbk && psList->psUserCbk->pv_cbk_disconnect )
		psList->psUserCbk->pv_cbk_disconnect( psList->psUserCbk->psHandle );
}

static void eth_link_socket_cbk_delete( void *psHandle )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psHandle;
	if( psList && psList->psUserCbk && psList->psUserCbk->pv_cbk_delete )
		psList->psUserCbk->pv_cbk_delete( psList->psUserCbk->psHandle );

	EAP_LOG_DEBUG( "Ip = %d, Port = %d\r\n", psList->sCfg.uRemoteIp, psList->sCfg.wRemotePort);
	eth_link_del_tcp_client_list( psList );
}

static void *eth_link_create_tcp_client_list( uint16_t wConnectTime, uint32_t uIp, uint16_t wPort, eth_link_list_t* psEthList )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)malloc(sizeof(eth_link_tcp_client_list_t));

	memset( psList, 0, sizeof(eth_link_tcp_client_list_t) );

	EAP_LOG_DEBUG( " %x %d\r\n", uIp, wPort);
	if( psList )
	{

		psList->sCfg.uRemoteIp		= uIp;
		psList->sCfg.wRemotePort	= wPort;
		psList->sCfg.wConnectTime	= wConnectTime;

		psList->sCbk.psHandle			= psList;
		psList->sCbk.pv_cbk_connect		= eth_link_socket_cbk_connect;
		psList->sCbk.pv_cbk_data		= eth_link_socket_cbk_data;
		psList->sCbk.pv_cbk_disconnect	= eth_link_socket_cbk_disconnect;
		psList->sCbk.pv_cbk_delete		= eth_link_socket_cbk_delete;
		psList->sCbk.pv_cbk_tcp_Isok    = eth_link_socket_cbk_tcp_Isok;

		psList->sClient.bFlagInit = false;
		psList->psRoot = psEthList;
		
		EAP_LOG_DEBUG("psList->psRoot->net = %d, wConnectTime = %d\n", psList->psRoot->net, wConnectTime);

		if(RTH_LINK_NET_TCP_CLIENT == psList->psRoot->net){
			int eRet = tcp_client_create( &(psList->sClient), &(psList->sCfg), &(psList->sCbk) );
			if( 0 == eRet )
			{
				psList->sUser.psSocket	= (void*)(&(psList->sClient));
				psList->sUser.pi_send	= (int32_t (*)(void*,void*,uint32_t))tcp_client_send;
				psList->sUser.pi_close	= (int32_t (*)(void*))tcp_client_close;
				psList->psUserCbk		= (eth_link_cbk_t *)psEthList->ps_init( &(psList->sUser)) ;
			}
			else
			{
				free( psList );
				psList = NULL;
			}
		}

		if(RTH_LINK_USB_TYPEC == psList->psRoot->net){
			int eRet = 0;
			
			psList->sUser.psSocket	= (void*)(&(psList->sClient));
			psList->sUser.pi_send	= (int32_t (*)(void*,void*,uint32_t))usb_typec_client_send;
			psList->sUser.pi_close	= (int32_t (*)(void*))usb_typec_client_close;
			psList->psUserCbk		= (eth_link_cbk_t *)psEthList->ps_init( &(psList->sUser)) ;

			eRet = usb_typec_client_create( &(psList->sClient), &(psList->sCfg), &(psList->sCbk) );
			if(eRet )
			{
				free( psList );
				psList = NULL;
			}
		}
	}

	return psList;
}

static void eth_link_add_tcp_client_list( eth_link_list_t* psEthList, uint32_t uIp, uint16_t wPort, uint16_t wConnectTime )
{
	eth_link_tcp_client_list_t *psList = (eth_link_tcp_client_list_t*)psEthList->list;
	if( psList )
	{
		EAP_LOG_DEBUG("uIp = 0x%x, wPort = 0x%x, wConnectTime = %d\n", uIp, wPort, wConnectTime);
		uint32_t uCount = 1;
		for( ; psList->psNext; psList = psList->psNext )
		{
			if( psList->sCfg.uRemoteIp == uIp && psList->sCfg.wRemotePort == wPort ){
				//break;
				return;
			}
			uCount++;
		}
		
		if( psList->sCfg.uRemoteIp != uIp && psList->sCfg.wRemotePort != wPort && psEthList->max > uCount )
		{
			psList->psNext = eth_link_create_tcp_client_list( wConnectTime, uIp, wPort, psEthList );
			if( psList->psNext )
			{
				psList = psList->psNext;
				psList->psNext = NULL;
				psList->psRoot = psEthList;
			}
		}

	}
	else
	{
		EAP_LOG_DEBUG("wConnectTime = %d\n", wConnectTime);
		psEthList->list = eth_link_create_tcp_client_list( wConnectTime, uIp, wPort, psEthList );
		if( psEthList->list )
		{
			psList = (eth_link_tcp_client_list_t*)psEthList->list;
			psList->psNext = NULL;
			psList->psRoot = psEthList;
		}
	}
}

/*******************************************************************************
创建连接
*******************************************************************************/
/* typec will use this interface */
int eth_link_create_socket( uint16_t type, uint32_t ip, uint16_t port )
{
	int eRet = 0;

	for( eth_link_list_t *psList = psEthList; psList; psList = psList->next )
	{
		if( psList->type == type )
		{
			switch( psList->net )
			{
				case RTH_LINK_NET_TCP_CLIENT:
//					printf( "[%s %d]\r\n", __func__, __LINE__ );
					eth_link_add_tcp_client_list( psList, ip, port, 5 );
					break;
				case RTH_LINK_NET_UDP:
					break;
				case RTH_LINK_USB_TYPEC:
					//printf( "[%s %d]\r\n", __func__, __LINE__ );
					eth_link_add_tcp_client_list( psList, 0, 0, 0 );
					break;
				default:
					break;
			}
			break;
		}
	}

	return eRet;
}

/*******************************************************************************
交互协议
*******************************************************************************/
static char* eth_protocol_get_sn( void )
{
	static char sn[EAP_SN_LENGTH];
	uint32_t len = EAP_SN_LENGTH;
	nvram_read_sn(sn, &len);
	return sn;
}


static uint8_t* eth_protocol_analysis( void *msg, uint32_t len )
{
	uint8_t* pData = NULL;
	eth_protocol_head_t *psHead = (eth_protocol_head_t*)(msg);

	do{
		if( 0xFD != psHead->magic )
			break;

		uint32_t size;
		size = psHead->len_hi;
		size <<= 8;
		size |= psHead->len_lo;
		if( (size + 11) != (len) )
			break;

		uint8_t checksum = 0;
		for( int i = 0; 8 > i; ++i )
			checksum += ((uint8_t*)(psHead))[i];
		if( psHead->checksum != checksum )
			break;

		uint16_t crc = 0xFFFF;
		uint32_t count = len - 3;
		uint8_t *data = (uint8_t*)(psHead) + 1;
	    while( count-- )
	    {
			uint8_t tmp;
		    tmp = *data ^ (uint8_t)(crc & 0xFF);
		    tmp ^= (tmp << 4);
		    crc = (crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
			data++;
	    }
		uint16_t check = (data[1]<<8) | (data[0]<<0);
		if( crc != check )
			break;

		pData = ((uint8_t*)(msg)) + sizeof(eth_protocol_head_t);
	}while(0);

	return pData;
}

/*
static uint32_t eth_protocol_package( uint8_t msgid, void* msg, uint32_t len )
{
	eth_protocol_head_t *psHead = (eth_protocol_head_t*)(msg);
	psHead->magic = 0xFD;
	psHead->len_lo = (0xFF & (len>>0));
	psHead->len_hi = (0xFF & (len>>8));
	psHead->seq = 0;
	psHead->destid = 0xFF;
	psHead->sourceid = 0x03;	//radar id
	psHead->msgid = msgid;
	psHead->ans = 0;

	uint8_t checksum = 0;
	for( int i = 0; 8 > i; ++i )
		checksum += ((uint8_t*)(psHead))[i];
	psHead->checksum = checksum;

	uint16_t crc = 0xFFFF;
	uint32_t count = len + 11 - 3;
	uint8_t *data = (uint8_t*)(psHead);
    while( count-- )
    {
		uint8_t tmp;
		++data;
	    tmp = *data ^ (uint8_t)(crc & 0xFF);
	    tmp ^= (tmp << 4);
	    crc = (crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
    }
	data[0] = (0xFF & (crc>>0));
	data[1] = (0xFF & (crc>>8));

	return (len+11);
}
*/

#pragma pack(1)
	typedef struct eth_protocol_devinfo
	{
		eth_protocol_head_t head;
			
		uint32_t	protocol;
		uint8_t 	sn[32];
		uint16_t	type;
		uint16_t	status;
		char		version[64];

		uint16_t	crc;
	}eth_protocol_devinfo_t;
#pragma pack()
/*
static int eth_protocol_devinfo_pkg_send( eth_protocol_devinfo_t *psDevInfo )
{
//	printf( "[%s %d] %p\r\n", __func__, __LINE__, psDevInfo );
	psDevInfo->protocol = 0x03;
	strncpy( psDevInfo->sn, eth_protocol_get_sn(), sizeof(psDevInfo->sn) );
	psDevInfo->type = 3;
	psDevInfo->status = 1;
	strncpy( psDevInfo->version, "acur101_app_v00.01.02_dev", sizeof(psDevInfo->version) );
	eth_protocol_package( 0xBB, (uint8_t*)psDevInfo, sizeof(eth_protocol_devinfo_t) - 11 );
	return eth_link_broadcast( (uint8_t*)psDevInfo, sizeof(eth_protocol_devinfo_t) );
}
*/

#pragma pack(1)
	typedef struct eth_protocol_search
	{
		uint32_t	protocol;
		uint8_t		sn[32];
	}eth_protocol_search_t;
#pragma pack()
/*
static void eth_protocol_search_handler( eth_protocol_search_t *psSearch, uint32_t uIp )
{
	if( psSearch->sn[0] )
	{
		printf("eth_protocol_search_handler 111 \r\n");
		uint32_t len = strlen(psSearch->sn);
		if( strlen(eth_protocol_get_sn()) == len )
		{
			printf("eth_protocol_search_handler 222 : psSearch->sn\r\n", psSearch->sn);
			//if( 0 == memcmp(psSearch->sn, eth_protocol_get_sn(), len) )
			{				
				eth_protocol_devinfo_t sDevInfo;
				eth_protocol_devinfo_pkg_send( &sDevInfo );
			}
		}
	}
	else
	{
		eth_link_list_t *psList = psEthList;
		for( ; psList; psList = psList->next )
		{
			if( RTH_LINK_NET_TCP_CLIENT == psList->net )
			{
				eth_link_tcp_client_list_t *psClient = (eth_link_tcp_client_list_t *)psList->list;
				for( ; psClient; psClient = psClient->psNext )
				{
					if( htonl(psClient->sCfg.uRemoteIp) == uIp )
						break;
				}
				if( psClient )
					break;
			}
		}
		if( NULL == psList )
		{
			eth_protocol_devinfo_t sDevInfo;
			eth_protocol_devinfo_pkg_send( &sDevInfo );
		}
	}
}
*/

static int eth_protocol_check_ip( eth_protocol_search_t *psSearch, uint32_t uIp )
{
	int ret = 1;

	if( psSearch->sn[0] )
	{
		EAP_LOG_DEBUG("eth_protocol_search_handler 111 \r\n");
		uint32_t len = strlen(psSearch->sn);
		if( strlen(eth_protocol_get_sn()) == len )
		{
			EAP_LOG_DEBUG("eth_protocol_search_handler 222 : psSearch->sn\r\n", psSearch->sn);
			//if( 0 == memcmp(psSearch->sn, eth_protocol_get_sn(), len) )
			{
				ret = 0;				
			}
		}
	}
	else
	{
		eth_link_list_t *psList = psEthList;
		for( ; psList; psList = psList->next )
		{
			if( RTH_LINK_NET_TCP_CLIENT == psList->net )
			{
				eth_link_tcp_client_list_t *psClient = (eth_link_tcp_client_list_t *)psList->list;
				for( ; psClient; psClient = psClient->psNext )
				{
					if( htonl(psClient->sCfg.uRemoteIp) == uIp )
						break;
				}
				if( psClient )
					break;
			}
		}
		if( NULL == psList )
		{
			ret = 0;
		}
	}

	return ret;
}

#pragma pack(1)
	typedef struct eth_protocol_cntinfo
	{
		uint8_t		sn[32];
		uint32_t	ip;
		uint16_t	port;
		uint16_t	type;
	}eth_protocol_cntinfo_t;
#pragma pack()
static void eth_protocol_cntinfo_handler( eth_protocol_cntinfo_t *psCntInfo, uint32_t uIp )
{
	struct in_addr addr = {0};

	addr.s_addr = psCntInfo->ip;
	if( psCntInfo->sn[0] )
	{
		uint32_t len = strlen(psCntInfo->sn);
		if( strlen(eth_protocol_get_sn()) == len )
		{
			if( 0 == memcmp(psCntInfo->sn, eth_protocol_get_sn(), len) )
			{
				EAP_LOG_DEBUG( "connect to %s:%d\r\n", inet_ntoa(addr), psCntInfo->port);
				eth_link_create_socket( psCntInfo->type, htonl(psCntInfo->ip), psCntInfo->port );
			}
		}
	}
	else
	{
		EAP_LOG_DEBUG( "connect to %s:%d\r\n", inet_ntoa(addr), psCntInfo->port);
		eth_link_create_socket( psCntInfo->type, htonl(psCntInfo->ip), psCntInfo->port );
	}
}

void send_alinkmsg_to_eap(uint16_t channelType, uint16_t channel, uint8_t *data, uint32_t dataLen)
{
	Thread_Msg msg = {0};
	alink_msg_t* alink_msg = (alink_msg_t* )msg.buf;
	
	if (dataLen + 4 > sizeof(msg.buf)) {
		EAP_LOG_ERROR( "err,msg too long, datalen:%d, msg.buf len:%d\r\n", dataLen,sizeof(msg.buf));
		return;
	}
	alink_msg->channelType = channelType;
    alink_msg->channelId = channel;
	memcpy((void*)&alink_msg->msg_head,data,dataLen);

	SendAsynMsg(EAP_CMD_ALINK, (uint8_t *)&msg, dataLen + 4);
	SendAsynMsg(EAP_CMD_UPDATE_ALINK_TIME, (uint8_t *)&msg, dataLen + 4);  //maintain Alink linkstatus
}


extern void send_BB_msg(void);
extern void set_eth_proto(uint32_t protocol);

static void get_eth_link_proto(uint8_t *data, uint32_t dataLen)
{
	AlinkDupBroadcastReq *req = (AlinkDupBroadcastReq *)data;
	if(req){
		EAP_LOG_DEBUG("req->protocol = 0x%x\n", req->protocol);
		set_eth_proto(req->protocol);
	}
}

static void eth_protocol_handler( void* msg, uint32_t len, uint32_t uIp )
{
	uint8_t* pbyData = eth_protocol_analysis( msg, len );

	if( pbyData )
	{
		eth_protocol_head_t *psHead = (eth_protocol_head_t*)(msg);
		switch( psHead->msgid )
		{
			case 0xBA:
				// printf("eth_link_udp_cbk_data  0xBA \r\n");
				// eth_protocol_search_handler( (eth_protocol_search_t*)pbyData, uIp );
				//send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_UDP, 0xFFFF, msg, len);
                                ///if(! eth_protocol_check_ip( (eth_protocol_search_t*)pbyData, uIp )){
                                       EAP_LOG_DEBUG("eth_link_udp_cbk_data  0xBA \r\n");
                                       //send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_UDP, 0xFFFF, msg, len);
                                       //EapGetSys().DupResponse();
                                       get_eth_link_proto(msg, len);
                                       send_BB_msg();
                                //}

				break;
			case 0xBC:
				EAP_LOG_DEBUG("eth_link_udp_cbk_data  0xBC \r\n");
				eth_protocol_cntinfo_handler( (eth_protocol_cntinfo_t*)pbyData, uIp );
				break;
			default:
				EAP_LOG_DEBUG("eth_link_udp_cbk_data  default \r\n");
				break;
		}
	}
}



#define DEBUG_SEND_WIFICFG_SIMU_CLI_CMD

#ifdef DEBUG_SEND_WIFICFG_SIMU_CLI_CMD

	void wificfg_send_simu_msg(char *cmd)
	{
		char buf[256];
		
		alink_msg_head_t  *h = (alink_msg_head_t  *)buf;
		alink_cli_cmd_t   *req = (alink_cli_cmd_t *)h->payload;
		uint32_t req_len;
		uint32_t msg_len;
		int ret;
		req->handle = 1;
		ret = sprintf(req->cmd,"%s",cmd);
		req_len = sizeof(alink_cli_cmd_t) + ret + 1;

		msg_len = sizeof(alink_msg_head_t) + req_len;
		
		alink_msg_fill_header(h, (uint16_t)req_len, 0x2f, DEV_TRACER,0,0);
		
		send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_TCP,UINT16_ALL , (uint8_t*)h, msg_len);
		EAP_LOG_DEBUG("sent out simu wificfg :%s\n", req->cmd);
	}
	void wificfg_send_simu_msg_set()
	{
		wificfg_send_simu_msg("wificfg set cuav23 12345678");
	}

	void wificfg_send_simu_msg_get()
	{
		wificfg_send_simu_msg("wificfg get 1 1");
	}	

	void wificfg_send_simu_msg_clear()
	{
		wificfg_send_simu_msg("wificfg clear 1 1");
	}	

	
#endif



#define DEBUG_SEND_SPECTRUM_DUMP_SIMU_CLI_CMD

#ifdef DEBUG_SEND_SPECTRUM_DUMP_SIMU_CLI_CMD

	void spectrum_dump_send_simu_msg(char *cmd)
	{
		char buf[256];

		alink_msg_head_t  *h = (alink_msg_head_t  *)buf;
		alink_cli_cmd_t   *req = (alink_cli_cmd_t *)h->payload;
		uint32_t req_len;
		uint32_t msg_len;
		int ret;
		req->handle = 1;
		ret = sprintf(req->cmd,"%s",cmd);
		req_len = sizeof(alink_cli_cmd_t) + ret + 1;

		msg_len = sizeof(alink_msg_head_t) + req_len;

		alink_msg_fill_header(h, (uint16_t)req_len, 0x2f, DEV_TRACER,0,0);

		send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_TCP,UINT16_ALL , (uint8_t*)h, msg_len);
		EAP_LOG_DEBUG("sent out simu spectrum_dump :%s\n", req->cmd);
	}
	void spectrum_dump_send_simu_msg_set()
	{
		// 连续抓20次频谱数据
		spectrum_dump_send_simu_msg("spectrum_dump set 20");
	}

	void spectrum_dump_send_simu_msg_get()
	{
		// 连续抓包剩余次数
		spectrum_dump_send_simu_msg("spectrum_dump get 1");
	}

	void spectrum_dump_send_simu_msg_clear()
	{
		// 清除存储数据
		spectrum_dump_send_simu_msg("spectrum_dump clear 1");
	}

#endif


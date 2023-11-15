
#include "eth_link.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>


/*******************************************************************************
自动组网

*******************************************************************************/
typedef struct udp_socket_cbk
{
	void		*psHandle;
	void		(*pv_cbk_init)(void	*psHandle,void *psSocket );
	void		(*pv_cbk_data)(void	*psHandle,int32_t (*pi_recv)(void *psSocket,void *pvBuffer,uint32_t uSize,uint32_t *puIp));
	void		(*pv_cbk_delete)(void *psHandle);
}udp_socket_cbk_t;
typedef struct udp_socket_cfg
{
	bool		bFlagRecv;
	bool		bBroadcast;
	uint16_t	wRecvPort;
	uint16_t	wSendPort;
	uint32_t	uRecvIp;
	uint32_t	uSendIp;
}udp_socket_cfg_t;
typedef struct udp_socket
{
	volatile bool		bFlagInit;
	volatile bool		bFlagClose;
	int32_t 			iSocket;
	pthread_t 			pid;
	pthread_mutex_t 	mutex;
	udp_socket_cfg_t	*psCfg;
	udp_socket_cbk_t	*psCbk;
}udp_socket_t;
udp_socket_t sUdpSocket = { 0 };


int eth_link_init( void )
{
	return 0;
}

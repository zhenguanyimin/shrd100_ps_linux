
#ifndef _ETH_LINK_H_
#define _ETH_LINK_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <pthread.h>
// #include <arpa/inet.h>
// #include "sys/socket.h"
#include "sys/types.h"
#ifdef __cplusplus 
extern "C" { 
#endif
typedef enum
{
	RTH_LINK_NET_TCP_CLIENT	= (1),
	RTH_LINK_NET_UDP		= (2),
}eth_link_net_t;

typedef struct eth_link_user
{
	void	*psSocket;
	int32_t	(*pi_send)(void *psSocket,void *pvData,uint32_t uLength);
	int32_t (*pi_close)(void *psSocket);
}eth_link_user_t;

typedef struct eth_link_cbk
{
	void		*psHandle;
	void		(*pv_cbk_connect)(void *psHandle);
	void		(*pv_cbk_data)(void	*psHandle,void *psClient,int32_t (*pi_recv)(void *psClient,void *pvBuffer,uint32_t uSize));
	void		(*pv_cbk_disconnect)(void *psHandle);
	void		(*pv_cbk_delete)(void *psHandle);
	bool		(*pv_cbk_tcp_Isok)(void *psHandle);
}eth_link_cbk_t;

int eth_link_init( void );

int eth_link_add_type( uint16_t type, uint8_t net, uint8_t max, eth_link_cbk_t* (*ps_init)(eth_link_user_t *psUser) );
#ifdef __cplusplus 
} 
#endif 
#endif


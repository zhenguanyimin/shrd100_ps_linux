
#include "udp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

static int32_t udp_socket_recv( void *p_arg, void* pvBuffer, uint32_t uSize )
{
	return 0;
}

static void *udp_socket_task(void *p_arg)
{	
	return 0;
}

int32_t udp_socket_crate( udp_socket_t *psSocket, udp_socket_cfg_t *psCfg, udp_socket_cbk_t *psCbk )
{
	return 0;

}

int32_t udp_socket_close( udp_socket_t *psSocket )
{
	return 0;
}

int32_t udp_socket_send( udp_socket_t *psSocket, uint8_t *pbyData, uint32_t uLength )
{	
	return 0;
}


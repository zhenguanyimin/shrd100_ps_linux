
#include "udp_socket.h"
#include "eap_pub.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#if 0
#define DEBUG_UDP_PRINTF(arg...) printf(arg)
#else
#define DEBUG_UDP_PRINTF(arg...) 
#endif

static int32_t udp_socket_recv( void *p_arg, void* pvBuffer, uint32_t uSize, uint32_t *uIp )
{
	udp_socket_t *psSocket = (udp_socket_t*)p_arg;
	int32_t iLength = 0;

	if( psSocket )
	{
		struct sockaddr_in client_addr;
		socklen_t client_len;
		memset(&client_addr, 0, sizeof(client_addr));
		client_len = sizeof(client_addr);
		iLength = recvfrom(psSocket->iSocket, pvBuffer, uSize, 0, (struct sockaddr*)&client_addr, &client_len);
		if( 0 > iLength )
		{
			psSocket->bFlagClose = true;
			iLength = 0;
		}

		if(uIp)
		{
			*uIp = htonl(client_addr.sin_addr.s_addr);
			DEBUG_UDP_PRINTF( "uIp %x\r\n", *uIp );
                }

	}
	return iLength;
}

static void *udp_socket_task(void *p_arg)
{
	udp_socket_t *psSocket	= (udp_socket_t*)p_arg;
	struct timeval tv;


	if( psSocket->psCbk && psSocket->psCbk->pv_cbk_init )
		psSocket->psCbk->pv_cbk_init( psSocket->psCbk->psHandle, psSocket );
	for( ; ; )
	{
		if( psSocket->bFlagClose )
			break;
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(psSocket->iSocket, &readSet);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
        int ready = select(psSocket->iSocket + 1, &readSet, NULL, NULL, &tv);
        if(0 > ready)
		{
            break;
        }
		else if(0 == ready)
		{
			continue;
        }
		if(FD_ISSET(psSocket->iSocket, &readSet))
		{
			if( psSocket->psCbk && psSocket->psCbk->pv_cbk_data )
			{
				psSocket->psCbk->pv_cbk_data( psSocket->psCbk->psHandle, udp_socket_recv );
			}
			else
			{
				uint8_t abyBuffer[128];
				udp_socket_recv( psSocket, abyBuffer, sizeof(abyBuffer), NULL );
			}
		}
	}
	if( 0 <= psSocket->iSocket )
		close( psSocket->iSocket );
	psSocket->iSocket = -1;
	psSocket->bFlagInit = false;
	pthread_mutex_destroy( &psSocket->mutex );
	if( psSocket->psCbk && psSocket->psCbk->pv_cbk_delete )
		psSocket->psCbk->pv_cbk_delete( psSocket->psCbk->psHandle );
	return 0;
}

int32_t udp_socket_create( udp_socket_t *psSocket, udp_socket_cfg_t *psCfg, udp_socket_cbk_t *psCbk )
{
	int32_t eRet = 0;

	if( false == psSocket->bFlagInit )
	{
		do
		{
			if( NULL == psCfg || NULL == psCbk )
			{
				eRet = -1;
				break;
			}
			
			psSocket->bFlagInit	= true;
			psSocket->bFlagClose = false;
			psSocket->psCfg	= psCfg;
			psSocket->psCbk = psCbk;

			psSocket->iSocket = socket(AF_INET, SOCK_DGRAM, 0);
			if( 0 > psSocket->iSocket )
			{
				eRet = -2;
				break;
			}

			if( psSocket->psCfg->bFlagRecv )
			{
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(addr));
			    addr.sin_family = AF_INET;
			    addr.sin_addr.s_addr = htonl(psSocket->psCfg->uRecvIp);
			    addr.sin_port = htons(psSocket->psCfg->wRecvPort);
				if ( 0 > bind(psSocket->iSocket, (struct sockaddr *)&addr, sizeof(addr)) )
				{
					eRet = -3;
					EAP_LOG_ERROR("bind failed !!! \r\n");
					break;
				}
				EAP_LOG_DEBUG("bind succeed !!! \r\n");
			}

			if( psSocket->psCfg->bBroadcast )
			{
				int broadcastEnable = 1;
				eRet = setsockopt(psSocket->iSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
				if( 0 != eRet )
				{
					eRet = -4;
					EAP_LOG_ERROR("setsockopt failed !!! \r\n");
					break;
				}
				EAP_LOG_DEBUG("setsockopt succeed !!! \r\n");
			}

			if( 0 != pthread_mutex_init( &(psSocket->mutex), NULL ) )
			{
				eRet = -5;
				EAP_LOG_ERROR("create mutex failed !!! \r\n");
				break;
			}

			if( psSocket->psCfg->bFlagRecv )
			{
				if( 0 != pthread_create(&(psSocket->pid), NULL, udp_socket_task, psSocket) )
				{
					eRet = -6;
					EAP_LOG_ERROR("pthread create failed !!! \r\n");
					break;
				}
				pthread_setname_np(psSocket->pid, "udp_socket");
			}
			
			eRet = 0;
		}while(0);

		if( -4 > eRet )
		{
			EAP_LOG_ERROR("close socket eRet = %d !!! \r\n", eRet);
			close( psSocket->iSocket );
			pthread_mutex_destroy( &psSocket->mutex );
		}
		else if( -1 > eRet )
		{
			EAP_LOG_ERROR("eRet = %d\n", eRet);
			close( psSocket->iSocket );
		}
	}

	return eRet;

}

int32_t udp_socket_close( udp_socket_t *psSocket )
{
	int32_t eRet = 0;

	if( psSocket && psSocket->bFlagInit )
	{
		psSocket->bFlagClose = true;
	}

	return eRet;
}

int32_t udp_socket_send( udp_socket_t *psSocket, uint8_t *pbyData, uint32_t uLength )
{
	int32_t eRet = 0;
	  
	if( psSocket && psSocket->bFlagInit )
	{
		struct sockaddr_in send_addr;
		memset(&send_addr, 0, sizeof(send_addr));
		send_addr.sin_family = AF_INET;
		send_addr.sin_port = htons(psSocket->psCfg->wSendPort);		
		send_addr.sin_addr.s_addr = htonl(psSocket->psCfg->uSendIp);
		pthread_mutex_lock( &psSocket->mutex );
		eRet = sendto(psSocket->iSocket, pbyData, uLength, 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
		pthread_mutex_unlock( &psSocket->mutex );
		EAP_LOG_DEBUG("Ip = 0x%x, port = %d, eRet = %d\n", psSocket->psCfg->uSendIp, psSocket->psCfg->wSendPort, eRet);
	}
	else
	{
		eRet = -2;
		EAP_LOG_ERROR("eRet = %d\n", eRet);
	}

	return eRet;
}


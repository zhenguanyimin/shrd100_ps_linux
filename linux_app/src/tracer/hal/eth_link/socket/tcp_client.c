
#include "tcp_client.h"
#include "eap_pub.h"
#include "eap_sys_cmds.h"
#include <unistd.h>
#define _GNU_SOURCE 			/* See feature_test_macros(7) */
#include <sched.h>

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
#define DEBUG_TCP_PRINTF(arg...) printf(arg)
#else
#define DEBUG_TCP_PRINTF(arg...) 
#endif

extern int32_t loginFlag;
extern int32_t max_4t_tcp_connect_flag;
static int32_t tcp_client_recv( void *p_arg, void* pvBuffer, uint32_t uSize )
{
	int32_t iLength = 0;
	
	tcp_client_t *psClient = (tcp_client_t*)p_arg;
	if( psClient && pvBuffer )
	{
		iLength = recv(psClient->iSocket, pvBuffer, uSize, 0);
		if( 0 >= iLength )
		{
			psClient->bFlagConnect = false;
			iLength = 0;
		}
		DEBUG_TCP_PRINTF("*** sock:%d, rcv len:%d\n",psClient->iSocket,iLength);
	}
	
	return iLength;
}

static void *tcp_client_task(void *p_arg)
{
	tcp_client_t *psClient	= (tcp_client_t*)p_arg;
	struct sockaddr_in	remoteAddr;
	struct timeval timeout;
	fd_set		readSet;
	int32_t		iRet;
	int32_t		iLength;
	uint32_t	uConnectCount;

	uConnectCount = 0;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(psClient->psCfg->wRemotePort);
	remoteAddr.sin_addr.s_addr = htonl(psClient->psCfg->uRemoteIp);

	for( ; ; )
	{
		if( psClient->bFlagClose ){
			max_4t_tcp_connect_flag = 0;
			break;
		}
		// connect to the server
		iRet = connect(psClient->iSocket, (struct sockaddr *)&(remoteAddr), sizeof(remoteAddr));
		if (iRet < 0)
		{
			DEBUG_TCP_PRINTF( "tcp connect %d %x %d\r\n", iRet, psClient->psCfg->uRemoteIp, psClient->psCfg->wRemotePort );
			if( psClient->psCfg->wConnectTime )
			{
				if( psClient->psCfg->wConnectTime <= ++uConnectCount ){
					max_4t_tcp_connect_flag = 0;
					break;
				}
			}
			close( psClient->iSocket );
			
			psClient->iSocket = socket(AF_INET, SOCK_STREAM, 0);
			sleep( 2 );
		}
		else
		{
			psClient->bFlagConnect	= true;
			uConnectCount			= 0;
			if( psClient->psCbk->pv_cbk_connect )
				psClient->psCbk->pv_cbk_connect( psClient->psCbk->psHandle );
			for( ; ; )
			{
				/* waiting for data */
				FD_ZERO(&readSet);
				FD_SET(psClient->iSocket, &readSet);
				timeout.tv_sec			= 1;
				timeout.tv_usec 		= 0;
				
				EAP_LOG_DEBUG("sock:%d,  port = 0x%x, ip = 0x%x\n",psClient->iSocket, psClient->psCfg->wRemotePort, psClient->psCfg->uRemoteIp); //to string
				iRet = select(psClient->iSocket + 1, &readSet, NULL, NULL, &timeout);
				if( 0 < iRet )
				{
					// read the data
					if(FD_ISSET(psClient->iSocket, &readSet))
					{
						max_4t_tcp_connect_flag = 1;
						if( psClient->psCbk->pv_cbk_data )
						{
							psClient->psCbk->pv_cbk_data( psClient->psCbk->psHandle, psClient, tcp_client_recv );
						}
						else
						{
							uint8_t abyBuffer[128];
							int32_t iLength = tcp_client_recv( psClient, abyBuffer, sizeof(abyBuffer) );
							if( 0 >= iLength )
								break;
						}
						if( false == psClient->bFlagConnect )
							break;
					}
				}
				else if( 0 > iRet )
				{
					// error, reconnect
					EAP_LOG_DEBUG("*** sock:%d, errno = %d, iRet = %d\n",psClient->iSocket, errno, iRet);
					if (errno == EBADF && FD_ISSET(psClient->iSocket, &readSet))
						break;
				}else{
					EAP_LOG_DEBUG("*** sock:%d, select return 0\n",psClient->iSocket);
				}
				if( psClient->psCbk->pv_cbk_tcp_Isok )
				{
					psClient->bFlagClose = !psClient->psCbk->pv_cbk_tcp_Isok( psClient->psCbk->psHandle );
					//printf("get psClient->bFlagClose is %d\n",psClient->bFlagClose);
				}	
				if( psClient->bFlagClose ) break;
			}
			// disconnect, reconnect
			psClient->bFlagConnect	= false;
			loginFlag = -1;
			if( psClient->psCbk->pv_cbk_disconnect )
				psClient->psCbk->pv_cbk_disconnect( psClient->psCbk->psHandle );
		}
	}

	if( 0 <= psClient->iSocket )
		close( psClient->iSocket );
	psClient->iSocket 	= -1;
	psClient->bFlagInit	= false;
	psClient->bFlagClose = false;
	pthread_mutex_destroy( &psClient->mutex );
	if( psClient->psCbk->pv_cbk_delete )
		psClient->psCbk->pv_cbk_delete( psClient->psCbk->psHandle );
	return 0;
}

int32_t tcp_client_create( tcp_client_t *psClient, tcp_client_cfg_t *psCfg, tcp_client_cbk_t *psCbk )
{
	int32_t eRet = 0;

	if( false == psClient->bFlagInit )
	{
		psClient->bFlagInit	= true;
		psClient->bFlagClose = false;
		psClient->bFlagConnect = false;
		psClient->link_net = RTH_LINK_NET_TCP_CLIENT;
		if( psCfg && psCbk )
		{
			psClient->psCfg = psCfg;
			psClient->psCbk = psCbk;
			psClient->iSocket = socket(AF_INET, SOCK_STREAM, 0);
			if( 0 <= psClient->iSocket )
			{
				int keepalive = 1;
				int keepidle = 1;
				int keepinterval = 1;
				int keepcount = 10;
				struct timeval timeout;

				timeout.tv_sec = 0;  // 设置超时时间为300ms
				timeout.tv_usec = 300000;

				setsockopt(psClient->iSocket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
				setsockopt(psClient->iSocket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
				setsockopt(psClient->iSocket, IPPROTO_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval));
				setsockopt(psClient->iSocket, IPPROTO_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
				setsockopt(psClient->iSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

				if( 0 == pthread_mutex_init( &(psClient->mutex), NULL ) )
				{
					if( 0 == pthread_create(&(psClient->pid), NULL, tcp_client_task, psClient) )
					{
						EAP_LOG_DEBUG("tcp client create ok\n");
						pthread_setname_np(psClient->pid, "tcp_client");
						eRet = 0;
					}
					else
					{
						pthread_mutex_destroy( &psClient->mutex );
						close( psClient->iSocket );
						psClient->bFlagInit	= false;
						eRet = -2;
					}
				}
				else
				{
					close( psClient->iSocket );
					psClient->bFlagInit	= false;
					eRet = -2;
				}
			}
			else
			{
				psClient->bFlagInit = false;
				eRet = -2;
			}
		}
		else
		{
			psClient->bFlagInit	= false;
			eRet = -1;
		}
	}

	if(eRet < 0){
		EAP_LOG_DEBUG("eRet = %d\n", __func__, __LINE__, eRet);
	}

	return eRet;
}

int32_t tcp_client_send( tcp_client_t *psClient, void *pvData, uint32_t uLength )
{
	int32_t eRet = 0;
	struct timeval timeout;
	void *p = pvData;
	int   len = uLength;
	ssize_t bytesSent = 0;
	alink_msg_head_t *phead = (alink_msg_head_t *)pvData;

	DEBUG_TCP_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len);
	if( psClient && pvData && psClient->bFlagConnect )
	{
		pthread_mutex_lock( &psClient->mutex );
		//huaguoqing modify,2023.08.19
tryagain:
		if(EAP_ALINK_CMD_PC_REQUEST_FIRMWARE_UPGRADE == phead->msgid){
			/* ota recover set no timeout */
			timeout.tv_sec = 0;  // recover set no timeout
			timeout.tv_usec = 0;
			setsockopt(psClient->iSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
		}

		DEBUG_TCP_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len);
		bytesSent = send( psClient->iSocket, p, len, 0);
		if (bytesSent > 0) {
			// 成功发送了bytesSent字节的数据
			if(bytesSent != len){
				len -= bytesSent;
				p += bytesSent;
			
				usleep(1000);
				DEBUG_TCP_PRINTF("***(%s:%d) tryagain, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				goto tryagain;
			}
		} else if (bytesSent == 0) {
			// 连接已经关闭
			DEBUG_TCP_PRINTF("***(%s:%d) tryagain, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
			eRet = -5;
		} else {
			// 发送出现错误，可以根据errno进行错误处理
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// 发送缓冲区已满，稍后再试
				usleep(10000);
				EAP_LOG_DEBUG("***(%s:%d) tryagain EAGAIN or EWOULDBLOCK, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				goto tryagain;
			} else if (errno == EINTR) {
				EAP_LOG_DEBUG("***(%s:%d) tryagain EINTR, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				// 发送被中断，可以重新尝试发送
				goto tryagain;
			} else {
				// 其他类型的错误
				eRet = -10;
				DEBUG_TCP_PRINTF("***(%s:%d) err, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
			}
		}

		pthread_mutex_unlock( &psClient->mutex );
	}
	else
	{
		DEBUG_TCP_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
		eRet = -2;
	}

	if(eRet < 0){
		EAP_LOG_DEBUG("***ret:%d, sock:%d, data:%p,len:%d\n",eRet,psClient->iSocket, p,len);
	}
	
	DEBUG_TCP_PRINTF("***(%s:%d) ret:%d, sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,eRet,psClient->iSocket, p,len);
	return eRet;
}

int32_t tcp_client_close( tcp_client_t *psClient )
{
	int32_t eRet = 0;

	if( psClient && psClient->bFlagConnect )
	{
		psClient->bFlagClose = true;
	}

	return eRet;
}



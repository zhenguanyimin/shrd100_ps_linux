

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <linux/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>

#include "eap_pub.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_typec.h"

#ifdef __cplusplus
}
#endif

#if 0
#define DEBUG_TYPEC_PRINTF(arg...) printf(arg)
#else
#define DEBUG_TYPEC_PRINTF(arg...) 
#endif

bool  clear_write_cache = false;

char const *ptypec_path="/dev/ttyGS0";

int usb_typec_cfg_option(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	
	if ( tcgetattr( fd,&oldtio) != 0) {
		printf("[%s, %d] err!\n", __func__, __LINE__);
		return -1;
 	}

	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 380400:
			cfsetispeed(&newtio, 380400);
			cfsetospeed(&newtio, 380400);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}

	if( nStop == 1 )
		newtio.c_cflag &= ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |= CSTOPB;

	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);

	if((tcsetattr(fd,TCSANOW,&newtio))!=0){
		printf("[%s, %d]com set error", __func__, __LINE__);
		return -1;
	}
	DEBUG_TYPEC_PRINTF("set done!\n");

	return 0;
}

void usb_typec_get_baud_rate(int fd)
{
	struct termios options;
	tcgetattr(fd, &options);
	speed_t baud_rate = cfgetospeed(&options);

	DEBUG_TYPEC_PRINTF("[%s, %d]Baud rate: %d\n", __func__, __LINE__, baud_rate);

	return ;
}

void usb_typec_port_close(int fd)
{
	if(fd < 0){
		printf("[%s, %d]close fd = %d failed!\n", __func__, __LINE__, fd);
	}else{
		close(fd);
	}
}



int usb_typec_port_open(void)
{
	int fd = 0;
	int ret = 0;
	
	fd = open(ptypec_path, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd){
		printf("[%s,%d]Can't Open %s\n", __func__, __LINE__, ptypec_path);
		return(-1);
	}else{
		DEBUG_TYPEC_PRINTF("open %s success\n", ptypec_path);
	}

	// 获取文件描述符的当前状态标志
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl");
		return -1;
	}

	// 设置文件描述符为非阻塞模式
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl");
		return -1;
	}

	if(isatty(fd)==0)
		printf("[%s, %d]standard input is not a terminal device\n", __func__, __LINE__);
	else
		DEBUG_TYPEC_PRINTF("[%s, %d]isatty success!\n", __func__, __LINE__);

	DEBUG_TYPEC_PRINTF("[%s, %d] fd=%d\n", __func__, __LINE__, fd);

	ret = usb_typec_cfg_option(fd, 380400, 8, 'N', 1);
	if(ret < 0){
		usb_typec_port_close(fd);
		fd = ret;
		EAP_LOG_ERROR("typec cfg failed, fd=%d\n", fd);
	}
	
	usb_typec_get_baud_rate(fd);
	DEBUG_TYPEC_PRINTF("usb typec open and cfg ok!\n");
	return fd;
}

static int32_t usb_typec_read_data(int fd, void* pBuffer, uint32_t uSize)
{
	int ret = 0;
	
	DEBUG_TYPEC_PRINTF("[%s, %d]uSize = %d\n", __func__, __LINE__, uSize);
	if(!fd || !pBuffer || !uSize){
		printf("[%s, %d] para err!\n", __func__, __LINE__);
		return -1;
	}

	ret= read(fd,pBuffer,uSize);

	DEBUG_TYPEC_PRINTF("[%s, %d] ret = %d\n", __func__, __LINE__, ret);
	return ret;
}

static int usb_typec_write_data(tcp_client_t *psClient, void* pBuffer, uint32_t uSize)
{
	int iLength = 0;
	int fd = 0;

	if(!psClient){
		EAP_LOG_DEBUG("psClient is NULL\n");
		return -1;
	}

	fd = psClient->iSocket;
	
	DEBUG_TYPEC_PRINTF("[%s, %d] fd = %d, uSize = %d\n", __func__, __LINE__, fd, uSize);
	
	if(!fd || !pBuffer || !uSize){
		EAP_LOG_DEBUG("para err!\n");
		return -1;
	}
	
	iLength = write(fd, pBuffer, uSize);
	DEBUG_TYPEC_PRINTF("[%s, %d]clear_write_cache = %d\n", __func__, __LINE__, clear_write_cache);
	if((iLength < 0) || (true == clear_write_cache)){
		// 清空输出缓冲区
		DEBUG_TYPEC_PRINTF("[%s, %d] clear cache\n", __func__, __LINE__);
		if (tcflush(fd, TCOFLUSH) == -1) {
			perror("tcflush");
		}
		clear_write_cache = false;
		iLength = 0;
	}
	DEBUG_TYPEC_PRINTF("[%s, %d] iLength = %d\n", __func__, __LINE__, iLength);

	return iLength;
}

static ssize_t usb_typec_data_recv( void *p_arg, void* pvBuffer, uint32_t uSize )
{
	ssize_t iLength = 0;
	tcp_client_t *psClient = (tcp_client_t*)p_arg;
	if( psClient && pvBuffer )
	{
		iLength = usb_typec_read_data(psClient->iSocket, pvBuffer, uSize);
		if( 0 >= iLength )
		{
			iLength = 0;
		}
	}
	DEBUG_TYPEC_PRINTF("[%s, %d] iLength = %d\n", __func__, __LINE__, iLength);
	return iLength;
}

static void *usb_typec_readdata_task(void *p_arg)
{
	struct timeval timeout;
	fd_set		readSet;
	int32_t		iRet;
	int32_t		iLength;
	
	tcp_client_t *psClient	= (tcp_client_t*)p_arg;

	if( psClient->bFlagClose )
		return ;


	DEBUG_TYPEC_PRINTF("[%s, %d]psClient->iSocket = %d\n", __func__, __LINE__, psClient->iSocket);
	psClient->bFlagConnect	= true;
	
	for( ; ; )
	{
		/* waiting for data */
		FD_ZERO(&readSet);
		FD_SET(psClient->iSocket, &readSet);

		timeout.tv_sec			= 2;
	    timeout.tv_usec			= 0;

		iRet = select(psClient->iSocket + 1, &readSet, NULL, NULL, &timeout);
		DEBUG_TYPEC_PRINTF("[%s, %d]iRet = %d\n", __func__, __LINE__, iRet);
		if( 0 < iRet )
		{
			DEBUG_TYPEC_PRINTF("[%s, %d] psClient->iSocket = %d\n", __func__, __LINE__, psClient->iSocket);
			// read the data
			if(FD_ISSET(psClient->iSocket, &readSet))
			{
				if( psClient->psCbk->pv_cbk_data )
				{
					psClient->psCbk->pv_cbk_data( psClient->psCbk->psHandle, psClient, usb_typec_data_recv );
				}
			}
		}
		else if( 0 > iRet )
		{
			// error, reconnect
			if (errno == EBADF && FD_ISSET(psClient->iSocket, &readSet)){
				EAP_LOG_ERROR("error, reconnect\n");
			}
		}		
		if( psClient->psCbk->pv_cbk_tcp_Isok )
		{
			psClient->bFlagClose = !psClient->psCbk->pv_cbk_tcp_Isok( psClient->psCbk->psHandle );
			DEBUG_TYPEC_PRINTF("[%s, %d]get psClient->bFlagClose is %d\n",__func__, __LINE__, psClient->bFlagClose);
		}

		if( psClient->bFlagClose ) {
			usb_typec_port_close(psClient->iSocket);
			clear_write_cache = true;
			psClient->iSocket	= -1;
			psClient->bFlagInit = false;
			psClient->bFlagClose = false;
			
		    DEBUG_TYPEC_PRINTF("[%s, %d]clear_write_cache =  %d\n",__func__, __LINE__, clear_write_cache);
			
			sleep(1);
			
			psClient->iSocket = usb_typec_port_open();
			if(psClient->iSocket > 0){
				psClient->bFlagInit = true;
				if( psClient->psCbk->pv_cbk_connect ){
					DEBUG_TYPEC_PRINTF("[%s,%d] pv_cbk_connect!\n", __func__, __LINE__);
					psClient->psCbk->pv_cbk_connect( psClient->psCbk->psHandle );
				}
			}
		}

	}

	pthread_mutex_destroy( &psClient->mutex );
	
	if( psClient->psCbk->pv_cbk_delete ){
		EAP_LOG_ERROR("delete client!\n", psClient->bFlagClose);
		psClient->psCbk->pv_cbk_delete( psClient->psCbk->psHandle );
	}
	
	return 0;
}

int32_t usb_typec_client_create( tcp_client_t *psClient, tcp_client_cfg_t *psCfg, tcp_client_cbk_t *psCbk )
{
	int32_t eRet = 0;

	if( !psClient->bFlagInit ) {
		psClient->bFlagInit	= true;
		psClient->bFlagClose = false;
		psClient->bFlagConnect = true;
		psClient->bFlagRevHeartBeat = false;
		psClient->link_net = RTH_LINK_USB_TYPEC;
		
		if( psCfg && psCbk ) {
			psClient->psCfg = psCfg;
			psClient->psCbk = psCbk;
			/*open device*/
			psClient->iSocket = usb_typec_port_open();
			/*upload the connect info to the c2   ?????*/
			if( psClient->psCbk->pv_cbk_connect ){
				DEBUG_TYPEC_PRINTF("[%s,%d] pv_cbk_connect!\n", __func__, __LINE__);
				psClient->psCbk->pv_cbk_connect( psClient->psCbk->psHandle );
			}
			
			if( 0 <= psClient->iSocket ) {
				if( 0 == pthread_mutex_init( &(psClient->mutex), NULL ) ) {
					if( 0 == pthread_create(&(psClient->pid), NULL, usb_typec_readdata_task, psClient) ) {
						pthread_setname_np(psClient->pid, "usb_typec");
						eRet = 0;
						DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
					}else{
						pthread_mutex_destroy( &psClient->mutex );
						usb_typec_port_close( psClient->iSocket );
						psClient->bFlagInit	= false;
						eRet = -2;
						DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
					}
				}else{
					usb_typec_port_close( psClient->iSocket );
					psClient->bFlagInit	= false;
					eRet = -2;
					DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
				}
			}else{
				psClient->bFlagInit = false;
				eRet = -2;
				DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
			}
		}else{
			psClient->bFlagInit	= false;
			eRet = -1;
			DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
		}
	}
	DEBUG_TYPEC_PRINTF("[%s, %d]eRet = %d\n", __func__, __LINE__, eRet);
	return eRet;
}

int32_t usb_typec_client_send( tcp_client_t *psClient, void *pvData, uint32_t uLength )
{
	int32_t eRet = 0;
	void *p = pvData;
	int   len = uLength;
	ssize_t bytesSent = 0;
	//DEBUG_TYPEC_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len);
	if( psClient && pvData && psClient->bFlagConnect )
	{
		pthread_mutex_lock( &psClient->mutex );
		//huaguoqing modify,2023.08.19
tryagain:	
		//DEBUG_TYPEC_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len);
		bytesSent = usb_typec_write_data(psClient, p, len);
		if (bytesSent > 0) {
			// 成功发送了bytesSent字节的数据
			if(bytesSent != len){
				len -= bytesSent;
				p += bytesSent;
			
				usleep(1000);
				//DEBUG_TYPEC_PRINTF("***(%s:%d) tryagain, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				goto tryagain;
			}
		} else if (bytesSent == 0) {
			// 连接已经关闭
			//DEBUG_TYPEC_PRINTF("***(%s:%d) tryagain, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
			eRet = -5;
		} else {
			// 发送出现错误，可以根据errno进行错误处理
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// 发送缓冲区已满，稍后再试
				usleep(10000);
				//DEBUG_TYPEC_PRINTF("***(%s:%d) tryagain EAGAIN or EWOULDBLOCK, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				goto tryagain;
			} else if (errno == EINTR) {
				//DEBUG_TYPEC_PRINTF("***(%s:%d) tryagain EINTR, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
				// 发送被中断，可以重新尝试发送
				goto tryagain;
			} else {
				// 其他类型的错误
				eRet = -10;
				//DEBUG_TYPEC_PRINTF("***(%s:%d) err, sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
			}
		}
		pthread_mutex_unlock( &psClient->mutex );
	}
	else
	{
		//DEBUG_TYPEC_PRINTF("***(%s:%d) sock:%d, data:%p,len:%d, bytesSend:%d\n",__FUNCTION__,__LINE__,psClient->iSocket, p,len,bytesSent);
		eRet = -2;
	}
	DEBUG_TYPEC_PRINTF("***(%s:%d) ret:%d, sock:%d, data:%p,len:%d\n",__FUNCTION__,__LINE__,eRet,psClient->iSocket, p,len);
	return eRet;
}

int32_t usb_typec_client_close( tcp_client_t *psClient )
{
	int32_t eRet = 0;

	if( psClient && psClient->bFlagConnect )
	{
		psClient->bFlagClose = true;
	}

	return eRet;
}



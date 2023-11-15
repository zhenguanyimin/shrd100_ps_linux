/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "debug.h"

/**
*@brief  ���ô���ͨ������
*@param  fd     ���� int  �򿪴��ڵ��ļ����
*@param  speed  ���� int  �����ٶ�
*@return  void
*/

int speed_arr[] = {B115200, B57600,  B38400, B19200, B9600, B4800, B2400, B1200, B300,
          B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
          19200,  9600, 4800, 2400, 1200,  300, };
int set_speed(int fd, int speed){
  int   i; 
  int   status; 
  struct termios   Opt;
  tcgetattr(fd, &Opt); 
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
    if  (speed == name_arr[i]) {     
      tcflush(fd, TCIOFLUSH);     
      cfsetispeed(&Opt, speed_arr[i]);  
      cfsetospeed(&Opt, speed_arr[i]);   
      status = tcsetattr(fd, TCSANOW, &Opt);  
      if  (status != 0) {        
        ERR_DBG_PRINT("tcsetattr failed");  
        return 1;     
      }    
      tcflush(fd,TCIOFLUSH);   
    }  
  }

  return 0;
}

/**
*@brief   ���ô�������λ��ֹͣλ��Ч��λ
*@param  fd     ����  int  �򿪵Ĵ����ļ����
*@param  databits ����  int ����λ   ȡֵ Ϊ 7 ����8
*@param  stopbits ����  int ֹͣλ   ȡֵΪ 1 ����2
*@param  parity  ����  int  Ч������ ȡֵΪN,E,O,,S
*/
int set_Parity(int fd,int databits,int parity,int stopbits, int RTSCTS)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*��������λ��*/
	{   
	case 7:		
		options.c_cflag |= CS7; 
		break;
	case 8:     
		options.c_cflag |= CS8;
		break;   
	default:    
		fprintf(stderr,"Unsupported data size\n"); 
        return -1;  
	}

    options.c_iflag |= INPCK;
    cfmakeraw(&options);
    //options.c_lflag |= (ICANON | ECHO | ECHOE);
    //options.c_lflag &= ~(ICANON | ECHO | ECHOE);
    //options.c_iflag &= ~(IXON | IXOFF);
switch (parity) 
{   
	case 'n':
	case 'N':    
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
		break;  
	case 'o':   
	case 'O':     
		options.c_cflag |= (PARODD | PARENB); /* ����Ϊ��Ч��*/  
		break;  
	case 'e':  
	case 'E':   
		options.c_cflag |= PARENB;     /* Enable parity */    
		options.c_cflag &= ~PARODD;   /* ת��ΪżЧ��*/     
		break;
	case 'S': 
	case 's':  /*as no parity*/   
	    options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
        break;  
	default:   
		fprintf(stderr,"Unsupported parity\n");    
		return -1;  
	}  

/* ����ֹͣλ*/  
switch (stopbits)
{   
	case 1:    
		options.c_cflag &= ~CSTOPB;  
		break;  
	case 2:    
		options.c_cflag |= CSTOPB;  
	   break;
	default:    
		 fprintf(stderr,"Unsupported stop bits\n");  
		 return -1; 
} 

/* Set rts/cts */ 
if (RTSCTS)
{
    printf("Set rts/cts");
	options.c_cflag |= CRTSCTS;
}

tcflush(fd,TCIFLUSH);
options.c_cc[VTIME] = 150; /* ���ó�ʱ15 seconds*/   
options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
if (tcsetattr(fd,TCSANOW,&options) != 0)   
{ 
	ERR_DBG_PRINT("SetupSerial failed");   
	return -1;  
} 
return 0;  
}

int serial_set_line_input(int fd)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}


    options.c_lflag |= ICANON;


    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* ���ó�ʱ15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
    	perror("SetupSerial 3");   
    	return -1;  
    } 
    return 0;  
}

int serial_init(const char *serial_dev_name, int *p_fd, int RTSCTS, int speed, int need_line_input)
{
    int fd;
    printf("init %s\n", serial_dev_name);
    fd = open(serial_dev_name, O_RDWR);
    if (-1 == fd)
    { 
        ERR_DBG_PRINT("init %s failed\n", serial_dev_name);
        return 1;
    }
#if 1
    set_speed(fd, speed);
    set_Parity(fd, 8, 'n', 1, RTSCTS);
    if (need_line_input) serial_set_line_input(fd);
#endif
    *p_fd = fd;
    return 0;

}

int serial_write(int fd_serial, void *src, int len)
{
    int ret = write(fd_serial, src, len);
    if (len != ret) perror("oh, write serial failed!");
    return 0;

}

int serial_read(int fd_serial, char *buf, int len)
{
    int ret = read(fd_serial, buf, len-1);
    if (-1 == ret) perror("oh, read serial failed!");
    buf[ret] = '\0';
    return ret;

}




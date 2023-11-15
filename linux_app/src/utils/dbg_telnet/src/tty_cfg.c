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

static struct termios	 old_tty_cfg;

int set_tty_input_to_raw_mode()
{
    struct termios   new_tty_cfg;

   
   tcgetattr(0, &old_tty_cfg); 
   new_tty_cfg = old_tty_cfg;
   new_tty_cfg.c_lflag &= ((~ICANON)); 
   new_tty_cfg.c_lflag &= (~(ECHO|ECHOE|ECHOK|ECHONL)); 
      tcflush(0, TCIOFLUSH);     
      if  (tcsetattr(0, TCSANOW, &new_tty_cfg)) {        
        ERR_DBG_PRINT("tcsetattr failed");  
        return -1;     
      }    
      tcflush(0,TCIOFLUSH);   



  return 0;
}


int restore_tty_input_mode()
{
      tcflush(0, TCIOFLUSH);     
      if  (tcsetattr(0, TCSANOW, &old_tty_cfg)) {        
        ERR_DBG_PRINT("tcsetattr failed");  
        return -1;     
      }    
      tcflush(0,TCIOFLUSH);   



  return 0;
}


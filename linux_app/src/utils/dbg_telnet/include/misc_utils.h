/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __MISC_UTILS_H__
#define  __MISC_UTILS_H__

#include   <stdint.h>

void nano_sleep(long sec, long nsec);
int get_self_path(char *buf, int buf_len);
void print_time();
int genPIDfile(char *szPidFile);
int register_sighandler(int signum, void (*handler)(int));
int mem_addr_vir2phy(unsigned long vir, unsigned long *phy);


#endif


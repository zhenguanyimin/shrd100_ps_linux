/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __TIMER_UTILS_H__
#define  __TIMER_UTILS_H__
#include <signal.h>
#include <time.h>

int create_posix_timer(timer_t *timerid, void *timer_handler, int sec, int nano_sec);
void itimer_init(int s, int us);
#endif


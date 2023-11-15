/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __SIGNAL_UTILS_H__
#define  __SIGNAL_UTILS_H__
#include <signal.h>

#define     RT_SIG_FOR_APP_MIN    (SIGRTMIN+10)
#define     RT_SIG_FOR_APP_MAX    (SIGRTMAX-10)
int get_a_free_sig_and_register_proc(void *sig_handler);
int restore_sig_default_proc(int sig);
int ignore_sig(int sig);
int register_sig_proc(int sig, void *sig_handler);

#endif


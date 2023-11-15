/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#include <pthread.h>
#include "signal_utils.h"
#include "debug.h"

static pthread_mutex_t sig_ops_mutex;

static void __attribute__((constructor, used)) sig_utils_init(void)
{
        pthread_mutex_init(&sig_ops_mutex, NULL);
}

int sig_has_no_handler(int sig)
{
    struct sigaction    sa;

    sigaction(sig, NULL, &sa);

    if (sa.sa_flags & SA_SIGINFO)
    {
        return sa.sa_sigaction==(void *)SIG_IGN || 
               sa.sa_sigaction==(void *)SIG_DFL;
    }

    return sa.sa_handler==(void *)SIG_IGN || 
               sa.sa_handler==(void *)SIG_DFL;
}

int register_sig_proc(int sig, void *sig_handler)
{
    struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);;
    sa.sa_sigaction = sig_handler;
    sa.sa_flags = SA_SIGINFO;
    return sigaction(sig, &sa, NULL);
}

int ignore_sig(int sig)
{
    return register_sig_proc(sig, SIG_IGN);
}

int restore_sig_default_proc(int sig)
{
    return register_sig_proc(sig, SIG_DFL);
}

int get_a_free_sig_and_register_proc(void *sig_handler)
{
    int i;
    int ret = -1;

    pthread_mutex_lock(&sig_ops_mutex);
    for (i=RT_SIG_FOR_APP_MIN; i<=RT_SIG_FOR_APP_MAX; i++)
    {
        if (!sig_has_no_handler(i))
            continue;

        if (0==register_sig_proc(i, sig_handler))
        {
            ret = i;
            goto EXIT;
        }
    }

EXIT:
    pthread_mutex_unlock(&sig_ops_mutex);
    return ret;
}





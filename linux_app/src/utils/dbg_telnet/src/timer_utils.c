/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */


#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "signal_utils.h"
#include "timer_utils.h"
#include "debug.h"

int create_posix_timer(timer_t *timerid, void *timer_handler, int sec, int nano_sec)
{
    struct sigevent  evp;
    struct itimerspec its;

    
    evp.sigev_notify = SIGEV_THREAD_ID;
    evp.sigev_signo = get_a_free_sig_and_register_proc(timer_handler);
    DBG_PRINT("evp.sigev_signo=%d", (int)evp.sigev_signo);
    evp.sigev_value.sival_ptr = timerid;
    evp._sigev_un._tid = syscall(SYS_gettid);
    DBG_PRINT("evp._sigev_un._tid=%d", (int)evp._sigev_un._tid);
    if (timer_create(CLOCK_REALTIME, &evp, timerid))
        {
            return -1;

        }

   its.it_value.tv_sec = sec;
   its.it_value.tv_nsec = nano_sec;
   its.it_interval.tv_sec = its.it_value.tv_sec;
   its.it_interval.tv_nsec = its.it_value.tv_nsec;

   if (timer_settime(*timerid, 0, &its, NULL))
    {
            return -1;
    }

   return 0;
}


#include <sys/time.h>
void itimer_init(int s, int us)
{
    struct itimerval timer;

    timer.it_value.tv_sec = s;
    timer.it_value.tv_usec = us;

    timer.it_interval.tv_sec = s;
    timer.it_interval.tv_usec = us;

    if (setitimer(ITIMER_REAL, &timer, NULL))
        DBG_PRINT("setitimer failed");

}


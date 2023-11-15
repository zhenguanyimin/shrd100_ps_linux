
#include "lock.h"

#include <string.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>


//================== CLock =======================
CLock::CLock(void)
{
    pthread_mutexattr_t   attr;
    pthread_mutexattr_init(&attr);

    /////////////////同一线程可以重复锁定
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_CriticalSection, &attr);

    pthread_mutexattr_destroy(&attr);
}

CLock::CLock(bool bIsRecursive/*为true对同一个线程表示可重入的，为false对同一个线程表示不可重入的*/)
{
	pthread_mutexattr_t   attr;
	pthread_mutexattr_init(&attr);

	/////////////////同一线程可以重复锁定
	pthread_mutexattr_settype(&attr, bIsRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_TIMED_NP);

	pthread_mutex_init(&m_CriticalSection, &attr);

	pthread_mutexattr_destroy(&attr);
}

CLock::~CLock(void)
{
    pthread_mutex_destroy(&m_CriticalSection);
}

void CLock::Lock()
{
    pthread_mutex_lock(&m_CriticalSection);
}

void CLock::Unlock()
{
    pthread_mutex_unlock(&m_CriticalSection);
}


//=================== CMySemaphore ================================
MySemaphore::MySemaphore() {
    // 2st paramater not equal to zero indicate shared in process
    // otherwise shared in all thread in current process
    // 3st paramater is the initialization value of semaphore
    sem_init(&m_sem, 0, 0);
}

MySemaphore::~MySemaphore()
{
    sem_destroy(&m_sem);
}

bool MySemaphore::wait()
{
    return sem_wait(&m_sem) == 0 ? true : false;
}

bool MySemaphore::tryWait()
{
    return sem_trywait(&m_sem) == 0 ? true : false;
}

bool MySemaphore::timedWait(const u_int64_t timeout)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        return false;
    }

    ts.tv_sec += timeout / 1000;
    ts.tv_nsec += (timeout % 1000) * 1000000;
    return sem_timedwait(&m_sem, &ts) == 0 ? true : false;
}

bool MySemaphore::post()
{
    return sem_post(&m_sem) == 0 ? true : false;
}

//=================== CMyEvent ================================
CMyEvent::CMyEvent(bool bManualReset) : m_bManualReset(bManualReset)
{
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_FCond, &cattr);

    m_semCount = 0;
}

CMyEvent::~CMyEvent()
{
    pthread_cond_destroy(&m_FCond);
}

bool CMyEvent::SetEvent()
{
    m_FEventCS.Lock();
    if (!m_semCount)
    {
        ++m_semCount;
    }
    pthread_cond_signal(&m_FCond);
    m_FEventCS.Unlock();

    return true;
}

bool CMyEvent::ResetEvent()
{
    m_FEventCS.Lock();
    if (m_semCount)
    {
        --m_semCount;
    }
    m_FEventCS.Unlock();

    return true;
}

u_int32_t CMyEvent::WaitEvent(u_int32_t dwTimeout_MS)
{
    struct timespec  ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    u_int32_t secs = dwTimeout_MS/1000;
    u_int64_t msecs = dwTimeout_MS%1000;

    u_int32_t add = 0;
    msecs = msecs*1000*1000 + ts.tv_nsec;
    add = msecs / (1000*1000*1000);
    ts.tv_sec += (add + secs);
    ts.tv_nsec = msecs%(1000*1000*1000);

    m_FEventCS.Lock();
    while(!m_semCount)
    {
        if( pthread_cond_timedwait(&m_FCond, &m_FEventCS.m_CriticalSection, &ts) == ETIMEDOUT )  //如果超时，是返回 ETIMEDOUT
        {
            m_FEventCS.Unlock();
            return WAIT_TIMEOUT;
        }
    }

    //注意：如果pthread_cond_timedwait超时到了，但是这个时候不能lock临界区，pthread_cond_timedwait并不会立即返回；
    //但是在pthread_cond_timedwait返回的时候，它仍在临界区中，且此时返回值为ETIMEDOUT.

    //只有在收到事件的情况下才能需要手动重置（即另外再在外面做ResetEvent）
    if (!m_bManualReset)
    {
        m_semCount--;
    }
    m_FEventCS.Unlock();

    return WAIT_OBJECT_0;
}

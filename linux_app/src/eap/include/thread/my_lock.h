#ifndef LOCK_H
#define LOCK_H

#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

using namespace std;

#define INFINITE 0xFFFFFFFF
#define WAIT_TIMEOUT	0x00000102L
#define WAIT_OBJECT_0	0x00000000L

class CLock
{
    friend class CMyEvent;
public:
	CLock(void);
	~CLock(void);
	void Lock();
	void Unlock();

protected:
	explicit CLock(bool bIsRecursive/*为true对同一个线程表示可重入的，为false对同一个线程表示不可重入的*/);

private:
    pthread_mutex_t   m_CriticalSection;
};

/////////////////////////////////////////
class MySemaphore {
public:
    MySemaphore();
    ~MySemaphore();

    /**
     * @brief wait resouce number > 0
     * @return true:successed, false:don't waited resouce
     */
    bool wait();

    /**
     * @brief try wait inc resouce number
     * @return true:successed, false:don't waited resouce
     */
    bool tryWait();

    /**
     * @brief wait for timeout
     * @parma [in] timeout : millisecond
     * @return true:successed, false:don't waited resouce
     */
    bool timedWait(const uint64_t timeout);

    /**
     * @brief post semaphore, inc resouce number
     * @return true:successed, false:don't waited resouce
     */
    bool post();

private:
    sem_t   m_sem;
};


class CMyEvent
{
public:
    CMyEvent(bool bManualReset = false);
    ~CMyEvent();

public:
    bool SetEvent();
    bool ResetEvent();
    uint32_t WaitEvent(uint32_t dwTimeout_MS);

private:
    const bool m_bManualReset;
    CLock            m_FEventCS;

    pthread_cond_t m_FCond;
    volatile uint32_t m_semCount;
};

inline uint32_t WaitForSingleObject(CMyEvent *hEvent, uint32_t dwMillseconds)
{
    return hEvent->WaitEvent(dwMillseconds);
}

class TdCondition
{
public:
 TdCondition()
  : m_cond()
  , m_mutex()
  , m_signal(false)
 {
 }
 ~TdCondition() {

 }

public:
 void SetEvent() {
  {
   std::unique_lock<std::mutex> lck(m_mutex);
   m_signal = true;
  }
  m_cond.notify_all();
 }

 void ResetEvent() {
  std::unique_lock<std::mutex> lck(m_mutex);
  m_signal = false;
 }


 void WaitEvent() {
  std::unique_lock<std::mutex> lck(m_mutex);
  m_cond.wait(lck, [&] { return m_signal; });
 }

private:
 std::condition_variable m_cond;
 std::mutex m_mutex;
 bool m_signal;
};

class CMySingleLock
{
public:
    inline explicit CMySingleLock(CLock &Object):m_Object(Object)
    {
        Object.Lock();
    }

    inline ~CMySingleLock(void)
    {
        m_Object.Unlock();
    }

private:
    CLock &m_Object;
};

/**************************************************************************************************************************/
#endif // LOCK_H

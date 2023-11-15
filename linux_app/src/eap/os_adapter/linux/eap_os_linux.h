#ifndef EAPOSLINUX_H
#define EAPOSLINUX_H

#include <string.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <mutex>
#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>
#include <map>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <error.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/shm.h>
#include <stdarg.h>
#include <assert.h>
#include "lock.h"
#include <mutex>
#include <condition_variable>

#define SZ_MAX_SIZE 50

typedef void* LPVOID;
typedef void* TRHEAD_RETVALUE;
typedef TRHEAD_RETVALUE (*PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
typedef void (*TIMER_JOB)(int lpTimerParameter);

typedef pthread_t   HANDLE;

struct __thread_info
{
    PTHREAD_START_ROUTINE lpStartAddress;
    LPVOID lpParameter;
    CMyEvent m_WaitEvent;
    CMyEvent m_SuspendEvent;
    u_int32_t sleepTime;
    bool  RunOneTime;
    bool  autoRelease;//是否自动销毁本结构体
    bool  RunFlag;//在sleepTime为0时用到

    bool IsCallbacking;   //是否正在回调线程函数
    char  szFilename[100];
    int   fileLineNum;
    char  szdesc[256];
    pthread_t threadID;
};

struct msg_st
{
    long int msg_type;
    char text[BUFSIZ];
};

struct shm_kv {
    uint32_t key;
    uint32_t value;
};

struct stuShareMemory
{
	int iSignal;  //读写标志 写模式-0 读模式-1
	shm_kv thread_queue_table[SZ_MAX_SIZE];
	shm_kv process_queue_table[SZ_MAX_SIZE];
	shm_kv resp_process_queue_table[SZ_MAX_SIZE];
	
	stuShareMemory()
    {
		iSignal = 0;
		memset(thread_queue_table,0,SZ_MAX_SIZE * sizeof(shm_kv));
		memset(process_queue_table,0,SZ_MAX_SIZE * sizeof(shm_kv));
		memset(resp_process_queue_table,0,SZ_MAX_SIZE * sizeof(shm_kv));
	}
};

// struct timer_info
// {
//   string timer_name;
//   u_int32_t periodMs;
//   TIMER_JOB timer_job;
//   int times;
//   bool isCycle;
//   bool isValid;
//   timer_info()
//   {
//     timer_name = "";
//     periodMs = 0;
//     timer_job = NULL;
//     times = 0;
//     isCycle = true;
//     isValid = true;
//   }
// };

class EapOsLinux
{
public:
    EapOsLinux();
    ~EapOsLinux();

    HANDLE create_thread(const char *name, LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,uint16_t priority,uint16_t stack_size,bool bCreatSuspended = false);
    int thread_start(HANDLE thread_handle);
    void thread_attach_core(HANDLE thread_handle,uint16_t core_id);
    bool resume_thread(HANDLE thread_handle);
    int thread_join(pthread_t thread_handle,LPVOID para);
    void thread_destory(HANDLE thread_handle,const char* fileName, int lineNum);
    bool set_current_thread_name(const char *pName,const char *pFileName,int fileline);
    static TRHEAD_RETVALUE threadProc(LPVOID lpPara);

    void sleep_ms(u_int32_t ms);//ms单位为毫秒
    uint64_t get_time_ms();
    void get_ticks_second(u_int32_t& second);


    int eap_queue_create(uint32_t queueItemSize, uint32_t queueItemMaxSize,const char *queueName, uint32_t queueKey);
    int eap_queue_destory(u_int32_t queueKey);
    int eap_queue_send(u_int32_t queue_id,const void * const send_data,uint32_t sendLen,uint64_t xTicksToWait,unsigned int prio);
    int32_t eap_queue_receive(int32_t queue_id,void * const recv_data,uint32_t bufLen,int64_t xTicksToWait);
    uint32_t eap_queue_remain_num(int32_t queue_id);
    
    timer_t eap_create_timer(const char * const pcTimerName,u_int32_t startTime,u_int32_t periodMs,uint64_t isCycle,TIMER_JOB timer_job);
    int eap_delete_timer(timer_t timer_id);

    int32_t eap_get_remote_queue_handle(uint16_t threadId);
    void eap_set_queue_handle_for_remote(uint16_t threadId, int32_t queueHandle);
    int32_t eap_get_remote_resp_queue(uint16_t processId);
    void eap_set_resp_queuehandle_for_remote(uint16_t processId, int32_t queueHandle);
    int16_t eap_get_remote_processid(uint16_t threadId);
    void eap_set_processid_for_remote(int16_t threadId, int16_t queueHandle);
    stuShareMemory* get_share_memory_ptr();
    void share_memory_ptr_give_back();

    void eap_printf(const char *format, va_list args);

    void* eap_create_mutex();
    void eap_mutex_lock(void* mutex);
    void eap_mutex_unlock(void* mutex);
    void eap_destory_mutex(void* mutex);
private:


    std::map<HANDLE,__thread_info*> m_ThreadInfoMap;
    CLock					m_ThreadInfoLock;
    u_int32_t m_queue_id;
    
private:
    
};

#endif // EAPOSLINUX_H

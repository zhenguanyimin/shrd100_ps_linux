#include "eap_os_linux.h"

#include <sys/syscall.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "eap_pub.h"
#include <stdio.h>
#include <execinfo.h>
#include <mqueue.h>
#include <vector>
#include <algorithm>
#include "TimerThread.h"

#include <iostream>
#include <strstream>

#define MAX_DEPTH (20)
#define EAP_TIMER_INTERNAL 10

using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void *share_memory_prt = NULL;

extern TdCondition notify;

EapOsLinux::EapOsLinux()
{

}

EapOsLinux::~EapOsLinux()
{

}

extern "C"
{
    void print_stackframe(void)
    {
        void * buffer[MAX_DEPTH];
        int depth = backtrace(buffer, MAX_DEPTH);
        char ** func_names = backtrace_symbols(buffer, depth);
        for (int i=0; i<depth; i++)
        {
            printf("Depth: %d, func name: %s\n", i, func_names[i]);
        }
    }
}

HANDLE EapOsLinux::create_thread(const char *name, LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,uint16_t priority,uint16_t stack_size,bool bCreatSuspended)
{
    __thread_info *pThreadInfo = new __thread_info;
    if(!bCreatSuspended)
    {
        pThreadInfo->m_SuspendEvent.SetEvent();//如果不想挂起就让它开始就有信号
    }
    pThreadInfo->RunOneTime = false;
    pThreadInfo->lpParameter = lpParameter;
    pThreadInfo->lpStartAddress = lpStartAddress;
    pThreadInfo->sleepTime=1;
    pThreadInfo->RunFlag=true;
    pThreadInfo->autoRelease=false;
    pThreadInfo->IsCallbacking=false;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_size);

    struct sched_param param;
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority += priority;

    pthread_attr_setschedparam(&attr, &param);

    pthread_t threadhandle;
    int iRet = 0;
    if((iRet = pthread_create(&threadhandle, &attr, lpStartAddress, this)) != 0)
    {
        errno = iRet;	//线程创建不会设置错误码，而是直接返回错误码
        return -1;
    }
    else
    {
        pthread_setname_np(threadhandle, name);
        m_ThreadInfoLock.Lock();
        m_ThreadInfoMap.insert(std::pair<HANDLE,__thread_info*>(threadhandle,pThreadInfo));
        m_ThreadInfoLock.Unlock();
        return threadhandle;
    }
}

void EapOsLinux::thread_attach_core(HANDLE thread_handle,uint16_t core_id)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core_id,&mask);
    if(pthread_setaffinity_np(thread_handle,sizeof(mask),&mask) == 0)
    {
        printf("core set success thread id %ld core id %d\n",thread_handle,core_id);
    }
}

bool EapOsLinux::resume_thread(HANDLE thread_handle)
{
    __thread_info *pThreadInfo=0;
    CMySingleLock singleLock(m_ThreadInfoLock);
    std::map<HANDLE,__thread_info*>::iterator iter=m_ThreadInfoMap.find(thread_handle);
    if(iter != m_ThreadInfoMap.end())
    {
        pThreadInfo=iter->second;
    }
    if(pThreadInfo)
    {
        pThreadInfo->m_SuspendEvent.SetEvent();
        return true;
    }
    else
    {
        return false;
    }
}

int EapOsLinux::thread_join(pthread_t thread_handle,LPVOID para)
{
    return pthread_join(thread_handle, &para);
}

void EapOsLinux::thread_destory(HANDLE thread_handle,const char* fileName, int lineNum)
{
    __thread_info *pThreadInfo=0;
    m_ThreadInfoLock.Lock();
    std::map<HANDLE,__thread_info*>::iterator iter=m_ThreadInfoMap.find(thread_handle);
    if(iter != m_ThreadInfoMap.end())
    {
        pThreadInfo=iter->second;
        m_ThreadInfoMap.erase(iter);
    }
    m_ThreadInfoLock.Unlock();
    if(pThreadInfo)
    {
        char info[300];
        memset(info,0,sizeof(info));
        sprintf(info,"OSBase:%s:DestoryThread",pThreadInfo->szdesc);
        printf("%s--%s:%d\n",info,fileName,lineNum);
        pThreadInfo->m_SuspendEvent.SetEvent();//先取消挂起
        pThreadInfo->m_WaitEvent.SetEvent();
        pThreadInfo->RunFlag = false;
        pthread_join(thread_handle, NULL);
        delete pThreadInfo;
    }
}

bool EapOsLinux::set_current_thread_name(const char *pName,const char *pFileName,int fileline)
{
    printf("thread name : %s, thread id : %ld, %s,%d\n",pName,syscall(__NR_gettid),pFileName,fileline);
    prctl(PR_SET_NAME, pName);
    return true;
}

TRHEAD_RETVALUE EapOsLinux::threadProc(LPVOID lpPara)
{
    EapOsLinux *pThreadInfo=(EapOsLinux*)lpPara;
    //set_current_thread_name(pThreadInfo->szdesc,pThreadInfo->szFilename,pThreadInfo->fileLineNum);
    //pThreadInfo->m_SuspendEvent.WaitEvent(INFINITE);//如果不想挂起就让它开始就有信号
    /*do
    {
        pThreadInfo->IsCallbacking = true;
        pThreadInfo->lpStartAddress(pThreadInfo->lpParameter);
        pThreadInfo->IsCallbacking = false;

        if(pThreadInfo->sleepTime)
        {
            if (pThreadInfo->m_WaitEvent.WaitEvent(pThreadInfo->sleepTime) != WAIT_TIMEOUT)
            {
                break;
            }
        }
    } while (pThreadInfo->RunFlag);
    if(pThreadInfo->autoRelease)
        delete pThreadInfo;*/
    return 0;
}
void EapOsLinux::sleep_ms(u_int32_t ms)
{
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;

    // many calls will be interupted. so we keep looping till we're done.
    while ( nanosleep(&req, &req) == -1 && errno == EINTR && (req.tv_nsec > 0 || req.tv_sec > 0))
        ;
}

uint64_t EapOsLinux::get_time_ms()
{
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void EapOsLinux::get_ticks_second(u_int32_t &second)
{
    second = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

#define CFG_USING_POSIX_MQ
#ifdef CFG_USING_POSIX_MQ

int EapOsLinux::eap_queue_create(uint32_t queueItemSize, uint32_t queueItemMaxSize,const char *queueName, uint32_t queueKey)
{
    mqd_t mq_cmd;
    struct mq_attr attr = {0};  //限制大小为800KB，该大小是整个消息队列的大小，不仅仅是最大消息数*消息的最大大小；还包括消息队列的额外开销，所以真正存放消息数据的大小是没有819200B的
    attr.mq_flags = 0;    //消息队列的标志：0或O_NONBLOCK,用来表示是否阻塞
    attr.mq_maxmsg = queueItemSize;  //消息队列的最大消息数           
    attr.mq_msgsize = queueItemMaxSize;
    attr.mq_curmsgs = 0;  //消息队列中当前的消息数目

	
	printf("[%s:%d]mq_open create: %d queueItemSize %d queueItemMaxSize:%d,attr.mq_msgsize:%d,queueKey:%d\n",__FUNCTION__,__LINE__,mq_cmd, queueItemSize,queueItemMaxSize,attr.mq_msgsize,queueKey);
    char path[16] = {0};
    //static int _key = 0;
    //_key++;
    sprintf(path,"/msgget.%d",queueKey);
    mq_cmd = mq_open(path,O_RDWR|O_CREAT,0666,&attr);
    if(mq_cmd<0)
    {
        mq_unlink(path);
        mq_cmd = mq_open(path,O_RDWR|O_CREAT,0666,&attr);
        perror("mq_open");
        printf("mq_open error: %d queueItemSize %d queueItemMaxSize:%d,attr.mq_msgsize:%d\n",mq_cmd, queueItemSize,queueItemMaxSize,attr.mq_msgsize);
    }
    else 
    {
      //printf("mq_open success: %d queueItemSize %d queueItemMaxSize:%d\n",mq_cmd, queueItemSize,queueItemMaxSize);
    }
    return mq_cmd;
}

int EapOsLinux::eap_queue_destory(u_int32_t queueKey)
{
    char queueNameSz[52] = {0};
    sprintf(queueNameSz,"/msgget.%d",queueKey);
    if(mq_unlink(queueNameSz) ==  -1)
    {
       EAP_LOG_DEBUG("mq_close error: %d\n",queueKey);
    }
    else
    {
       EAP_LOG_DEBUG("mq_close success: %d\n",queueKey);
    }
    return 0;
}

int EapOsLinux::eap_queue_send(u_int32_t queue_id,const void * const send_data,uint32_t sendLen,uint64_t xMsToWait,unsigned int prio)
{
    struct timespec abs_timeout;
    clock_gettime(CLOCK_REALTIME, &abs_timeout);
    abs_timeout.tv_sec += xMsToWait/1000;
    abs_timeout.tv_nsec = (xMsToWait % 1000) * 1000000;
    int nbytes = mq_timedsend(queue_id,(char*)send_data,sendLen,prio,&abs_timeout);

    int8_t msg_send_flag = -1;
    if(nbytes < 0)
    {
        int send_bytes = -1;
        // Only resend once
        for(int i = 0; i < 1; ++i)
        {
            send_bytes = mq_timedsend(queue_id,(char*)send_data,sendLen,prio,&abs_timeout);

            if(send_bytes >0)
            {
                msg_send_flag = 1;
                break;
            }
        }
        if(msg_send_flag < 0)
        {
            std::ostrstream err;
            err << "eap_queue_send error, queue_id=" << std::dec  << queue_id << ", length=" << sendLen << ", data(hex)";

            const uint8_t * const bufptr = static_cast<const uint8_t *>(send_data);
            int len = sendLen > 64 ? 64: sendLen;
            for (uint32_t pos = 0; pos < len; pos++)
            {
                err << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(bufptr[pos]);
            }

            err << ", errno: " << strerror(errno);
            std::cout << err.str() << std::endl;

            //printf("eap_queue_send error: data:%p,sendLen:%u, %s\n",send_data,sendLen,strerror(errno));
            return -1;
        }   
    }
    else
    {
       //printf("eap_queue_send success: %d\n",nbytes);
    }
    return 0;
}
int EapOsLinux::eap_queue_receive(int32_t queue_id,void * const recv_data,uint32_t bufLen,int64_t xMsToWait)
{ 
    mq_attr mqattr = {0};
    mq_getattr(queue_id,&mqattr);
    int nbytes;

    if(xMsToWait >= 0)
    {
      struct timespec abs_timeout;
      clock_gettime(CLOCK_REALTIME, &abs_timeout);
      abs_timeout.tv_sec += xMsToWait/1000;
      abs_timeout.tv_nsec = (xMsToWait % 1000) * 1000000;

      nbytes  = mq_timedreceive(queue_id,(char*)recv_data, mqattr.mq_msgsize,0,&abs_timeout);
      if(nbytes < 0)
      {
          return -1;
          //printf("mq_timedreceive error: %d xMsToWait is %d\n",nbytes,xMsToWait);
      }
      else
      {
        //printf("mq_timedreceive success: %d\n",nbytes);
      }
    }
    else
    {
      //printf("eap_queue_receive mqattr.mq_msgsize: %d bufLen:%d\n",mqattr.mq_msgsize,bufLen);
      nbytes  = mq_receive(queue_id,(char*)recv_data, mqattr.mq_msgsize,0);
      if(nbytes < 0)
      {
          return -1;
          //printf("eap_queue_receive error: %d\n",nbytes);
      }
      else
      {
        //printf("eap_queue_receive success: %d\n",nbytes);
      }
    }
    return 0;
}

uint32_t EapOsLinux::eap_queue_remain_num(int32_t queue_id)
{
  mq_attr mqattr;
  mq_getattr(queue_id,&mqattr);
  return mqattr.mq_curmsgs;
}


#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int EapOsLinux::eap_queue_create(uint32_t queueItemSize, uint32_t queueItemMaxSize,const char *queueName, uint32_t queueKey)
{
	 uint32_t key = queueKey + 0x1000;
     uint32_t queue_id = msgget(key, 0666 | IPC_CREAT);
     if(queue_id == -1)
     {
        fprintf(stderr, "queue create failed error: %d\n", errno);
        return -1;
     }
     return queue_id;
}

int EapOsLinux::eap_queue_destory(u_int32_t queue_id)
{
    if (msgctl(queue_id, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS;
}

int EapOsLinux::eap_queue_send(u_int32_t queue_id,const void * const send_data,uint32_t sendLen,uint64_t xMsToWait,unsigned int prio)
{
    msg_st* p_msg;
    p_msg = (msg_st*)send_data;
	p_msg->msg_type = MSG_CHANNEL_TYPE_DEFAULT;
	int len = sendLen - sizeof(p_msg->msg_type);

    // 向队列里发送数据
    if (msgsnd(queue_id, send_data, len, IPC_NOWAIT) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int EapOsLinux::eap_queue_receive(int32_t queue_id,void * const recv_data,uint32_t bufLen,int64_t xMsToWait)
{
	msg_st* p_msg;
    int msgflg = 0;
    if(xMsToWait == 0)
    {
        msgflg = IPC_NOWAIT;
    }
	
    if (msgrcv(queue_id, recv_data, bufLen - sizeof(p_msg->msg_type), MSG_CHANNEL_TYPE_DEFAULT, msgflg) == -1)
    {
        //EAP_LOG_INFO("msgrcv failed queue_id is %d msgflg is %d xMsToWait is %d\n",queue_id,msgflg,xMsToWait);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

uint32_t EapOsLinux::eap_queue_remain_num(int32_t queue_id)
{
	struct msqid_ds buf;	

    if (msgctl(queue_id, IPC_STAT, &buf) == -1) {
        perror("msgctl");
        return 0xffffffff;
    }

    return buf.msg_qnum;
}


#endif

void eap_timer_entry(int lpPara)
{
    notify.SetEvent();
    //printf("set singal to run\n");
}

timer_t EapOsLinux::eap_create_timer(const char * const pcTimerName,u_int32_t startTime,u_int32_t periodMs,uint64_t isCycle,TIMER_JOB timer_job)
{
    static int create_eap_timer_flag = 0;
    if(create_eap_timer_flag == 0)
    {
        create_eap_timer_flag++;
        sigevent evp;  
        itimerspec ts;  
        int ret;  
        timer_t timer_id;
        memset(&evp, 0, sizeof(sigevent));

        evp.sigev_value.sival_ptr = &timer_id;  
        evp.sigev_notify = SIGEV_SIGNAL;  
        evp.sigev_signo = SIGUSR1;  
        signal(SIGUSR1, eap_timer_entry);  
        
        ret = timer_create(CLOCK_REALTIME, &evp, &timer_id);  
        if( ret )   
        {
            fprintf(stderr, "timer_create erro: %d", errno);
        }

        ts.it_interval.tv_sec = 0;  
        ts.it_interval.tv_nsec = EAP_TIMER_INTERNAL*1000*1000;  
        ts.it_value.tv_sec = 1;
        ts.it_value.tv_nsec = 0;

        ret = timer_settime(timer_id, 0, &ts, NULL);
        if( ret )
        {
            fprintf(stderr, "timer_settime erro: %d", errno);
        }
        printf("timer create success\n");
    }
    TimerThread::Instance()->AddTimerInfo(pcTimerName,startTime,periodMs,isCycle,timer_job);
    return 0;
}

int EapOsLinux::eap_delete_timer(timer_t timer_id)
{
    int ret = timer_delete(timer_id);
    if(ret)
    {
        fprintf(stderr, "timer_settime erro: %d", errno);
    }
    else
    {
        printf("timer %ld delete success\n",timer_id);
    }
    return ret;
}

int32_t EapOsLinux::eap_get_remote_queue_handle(uint16_t threadId)
{
    if(threadId < 0)
    {
        return -1;
    }
    int32_t queueHandle = -1;
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 1;
    if(mem_ptr->iSignal != 0)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->thread_queue_table[i].key == threadId)
            {
                queueHandle = mem_ptr->thread_queue_table[i].value;
                break;
            }
        }
    }
    EapOsLinux::share_memory_ptr_give_back();
    return queueHandle;
}

void EapOsLinux::eap_set_queue_handle_for_remote(uint16_t threadId, int32_t queueHandle)
{
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 0;
    if(mem_ptr->iSignal != 1)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->thread_queue_table[i].key == 0 && mem_ptr->thread_queue_table[i].value == 0)
            {
                mem_ptr->thread_queue_table[i].key = threadId;
                mem_ptr->thread_queue_table[i].value = queueHandle;
            }
        }
      }
    mem_ptr->iSignal = 1;
    EapOsLinux::share_memory_ptr_give_back();
}

int32_t EapOsLinux::eap_get_remote_resp_queue(uint16_t processId)
{
    if(processId < 0)
    {
        return -1;
    }
    int32_t queueHandle = -1;
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 1;
    if(mem_ptr->iSignal != 0)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->resp_process_queue_table[i].key == processId)
            {
                queueHandle = mem_ptr->resp_process_queue_table[i].value;
                break;
            }
        }
    }
    EapOsLinux::share_memory_ptr_give_back();
    return queueHandle;
}

void EapOsLinux::eap_set_resp_queuehandle_for_remote(uint16_t processId, int32_t queueHandle)
{
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 0;
    if(mem_ptr->iSignal != 1)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->resp_process_queue_table[i].key == 0 && mem_ptr->resp_process_queue_table[i].value == 0)
            {
                mem_ptr->resp_process_queue_table[i].key = processId;
                mem_ptr->resp_process_queue_table[i].value = queueHandle;
            }
        }
    }
    mem_ptr->iSignal = 1;
    EapOsLinux::share_memory_ptr_give_back();
}

int16_t EapOsLinux::eap_get_remote_processid(uint16_t threadId)
{
    if(threadId < 0)
    {
        return -1;
    }
    int32_t queueHandle = -1;
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 1;
    if(mem_ptr->iSignal != 0)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->process_queue_table[i].key == threadId)
            {
                queueHandle = mem_ptr->process_queue_table[i].value;
                break;
            }
        }
    }
    EapOsLinux::share_memory_ptr_give_back();
    return queueHandle;
}
 
void EapOsLinux::eap_set_processid_for_remote(int16_t threadId, int16_t queueHandle)
{
    stuShareMemory* mem_ptr = EapOsLinux::get_share_memory_ptr();
    mem_ptr->iSignal = 0;
    if(mem_ptr->iSignal != 1)
    {
        for(int i = 0; i < SZ_MAX_SIZE; ++i)
        {
            if(mem_ptr->process_queue_table[i].key == 0 && mem_ptr->process_queue_table[i].value == 0)
            {
                    mem_ptr->process_queue_table[i].key = threadId;
                    mem_ptr->process_queue_table[i].value = queueHandle;
            }
        }
     }
    mem_ptr->iSignal = 1;
    EapOsLinux::share_memory_ptr_give_back();
}

stuShareMemory* EapOsLinux::get_share_memory_ptr()
{
    share_memory_prt = NULL;
	struct stuShareMemory *stu = NULL;
	int shmid = shmget((key_t)1234, sizeof(struct stuShareMemory), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		printf("shmget err.\n");
		return nullptr;
	}

	share_memory_prt = shmat(shmid, (void*)0, 0);
	if(share_memory_prt == (void*)-1)
	{
		printf("shmat err.\n");
		return nullptr;
	}

    return (struct stuShareMemory*)share_memory_prt;
}

void EapOsLinux::share_memory_ptr_give_back()
{
    shmdt(share_memory_prt);
}

void EapOsLinux::eap_printf(const char *format, va_list args)
{
     while (*format != '\0')
     {
         if (*format == '%')
         {
             ++format; // 移动到占位符的下一个字符
             if (*format == 'd')
             {
                 int value = va_arg(args, int);
                 printf("%d", value);
             }
             else if (*format == 'h' && *(++format) == 'd')
             {
                 int value = va_arg(args, int);
                 printf("%hd", value);
             }
             else if (*format == 'l' && *(++format) == 'd')
             {
                 long value = va_arg(args, long);
                 printf("%ld", value);
             }
             else if (*format == 'f')
             {
                 double value = va_arg(args, double);
                 printf("%f", value);
             }
             else if (*format == 's')
             {
                 char* value = va_arg(args, char*);
                 printf("%s", value);
             }
             else if (*format == 'c')
             {
                 int value = va_arg(args, int);
                 printf("%c", value);
             }
             else
             {
                 printf("Unsupported format specifier: %c", *format);
             }
         }
         else
         {
             printf("%c", *format);
         }

         ++format; // 移动到下一个字符
         }
}

void* EapOsLinux::eap_create_mutex()
{
    pthread_mutex_t * pCriticalSection   = new  pthread_mutex_t;
    *pCriticalSection = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutexattr_t   attr;
    pthread_mutexattr_init(&attr);

    /////////////////同一线程可以重复锁定
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(pCriticalSection, &attr);

    pthread_mutexattr_destroy(&attr);

    return (void*)pCriticalSection;
}

void EapOsLinux::eap_mutex_lock(void* mutex)
{
    pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void EapOsLinux::eap_mutex_unlock(void* mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

 void EapOsLinux::eap_destory_mutex(void* mutex)
 {
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    if(mutex)
    {
        delete mutex;
        mutex = nullptr;
    }
 }
 

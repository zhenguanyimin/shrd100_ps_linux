#include "TimerThread.h"
#include "my_lock.h"

DEFINE_THREAD(TimerThread, TIMER)
#define EAP_TIMER_INTERNAL 10
#define MAX_TIMER_NUM 50

TdCondition notify;

void TimerThread::PreInit()
{
    notify.ResetEvent();
    timerInfoVector.reserve(MAX_TIMER_NUM);
}

bool TimerThread::Run()
{
    //printf("TimerThread::Run()\n");
    notify.WaitEvent();
    {
        //printf("recv singal to run\n");
        static bool is_sort = false;
        if(!is_sort)
        {
            sort(timerInfoVector.begin(),timerInfoVector.end(),TimerThread::cmp);
            is_sort = true;
        }
        std::vector<timer_info>::iterator iter=timerInfoVector.begin();
        for(;iter != timerInfoVector.end(); ++iter)
        {
            if(!((*iter).isValid))
            {
            continue;
            }
            if((*iter).times != 0)
            {
                //printf("module %s is times is %d\n",(*iter).timer_name.c_str(),(*iter).times);
                (*iter).times--;
                //printf("module %s is times is %d\n",(*iter).timer_name.c_str(),(*iter).times);
            }
            if((*iter).times == 0)
            {
                (*iter).timer_job(0);
                if(!((*iter).isCycle))
                {
                    (*iter).isValid = false;
                }
                if((*iter).periodMs <= EAP_TIMER_INTERNAL)
                {
                    (*iter).times = 0;
                }
                else
                {
                    (*iter).times = (*iter).periodMs / EAP_TIMER_INTERNAL;
                    if(((*iter).periodMs % EAP_TIMER_INTERNAL) >= EAP_TIMER_INTERNAL/2)
                    {
                        (*iter).times++;
                    }
                }
                //printf("module %s is running\n",(*iter).timer_name.c_str());
            }
            //printf("timerInfoVector timer name is %s,period is %d,timer_job is %d times is %d capacity is %d size is %d\n"
            //,(*iter).timer_name.c_str(),(*iter).periodMs,(*iter).timer_job,(*iter).times,timerInfoVector.capacity(),timerInfoVector.size());
        }  
        notify.ResetEvent(); 
    }
    
    return true;
}

void TimerThread::AddTimerInfo(const char * const pcTimerName,uint32_t startTime,uint32_t periodMs,uint64_t isCycle,TIMER_JOB timer_job)
{
    timer_info info;
    info.periodMs = periodMs;
    info.timer_job = timer_job;
    info.timer_name = pcTimerName;
    info.isCycle = isCycle;
    if(info.periodMs <= EAP_TIMER_INTERNAL)
     {
       info.times = 0;
     }
     else
     {
       info.times = info.periodMs / EAP_TIMER_INTERNAL;
       if((info.periodMs % EAP_TIMER_INTERNAL) >= EAP_TIMER_INTERNAL/2)
       {
         info.times++;
       }
     }
     
     bool find = false;
    for (auto& timer : timerInfoVector)
    {
        if (strcmp(timer.timer_name.c_str(),pcTimerName) == 0)
        {
            timer = info;
            find = true;
            break;
        }
    }
    if (!find) timerInfoVector.push_back(info);    
}

void TimerThread::ShowInfo(uint8_t level)
{
	ThreadBase::ShowInfo(level);
    cout << hex;
    cout << EAP_BLANK_PREFIX(level) << "TimerNum: " << timerInfoVector.size();
    cout << EAP_BLANK_PREFIX(level) << "_timers: ";
    int i = 0;
	for (auto& timer : timerInfoVector)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "Timer[" << i << "]: ";
        cout << EAP_BLANK_PREFIX(level + 2) << "timer_name:" << timer.timer_name;
        cout << EAP_BLANK_PREFIX(level + 2) << "isValid:" << timer.isValid;
        cout << EAP_BLANK_PREFIX(level + 2) << "isCycle:" << timer.isCycle;
        cout << EAP_BLANK_PREFIX(level + 2) << "periodMs:" << timer.periodMs;
        cout << EAP_BLANK_PREFIX(level + 2) << "times:" << timer.times;
        ++i;
    }
}


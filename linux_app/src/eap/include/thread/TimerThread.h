#include "ThreadBase.h"
#include <vector>
#include <unistd.h>
#include "eap_pub.h"
#include<algorithm>

typedef void (*TIMER_JOB)(int lpTimerParameter);
struct timer_info
{
    string timer_name;
    uint32_t periodMs;
    TIMER_JOB timer_job;
    int times;
    bool isCycle;
    bool isValid;
    timer_info()
    {
        timer_name = "";
        periodMs = 0;
        timer_job = NULL;
        times = 0;
        isCycle = true;
        isValid = true;
    }
};

class TimerThread : public ThreadBase
{
    DECLARE_THREAD(TimerThread)
public:
    void PreInit() override;
    bool Run() override;
    void AddTimerInfo(const char * const pcTimerName,uint32_t startTime,uint32_t periodMs,uint64_t isCycle,TIMER_JOB timer_job);
    static bool cmp(timer_info timer_a, timer_info timer_b) {return timer_a.periodMs < timer_b.periodMs;}
	void ShowInfo(uint8_t level) override;
public:
    std::vector<timer_info> timerInfoVector;
};
#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "eap_os_adapter.h"

class ThreadQueue
{
public:
    ThreadQueue(uint16_t num, uint32_t maxItemLen, const char *queueName, uint32_t threadId);
    ~ThreadQueue();
    int32_t ConsumerMsg(uint8_t* buf, uint32_t bufLen);
    int32_t ProduceMsg(const uint8_t* buf, uint32_t bufLen);
	int ProduceMsgWithPrio(const uint8_t* buf, uint32_t bufLen,unsigned int prio);
    int32_t GetHandle() { return _handle; }
    uint16_t GetRemainMsgNum();
    uint16_t GetNum() { return _num; }
    int64_t GetWaitMs() { return _waitMs; }
    void ShowInfo(uint8_t level);
protected:
private:
    uint16_t _num;
    int64_t _waitMs;
    int32_t _handle;//returned mq handle by mq_open() 
    const char   *_queueName;
	uint32_t  _threadId;
};

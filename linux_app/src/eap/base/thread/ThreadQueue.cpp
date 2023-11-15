#include <iostream>
#include <iomanip>
#include <strstream>

#include "ThreadQueue.h"
#include "eap_os_adapter.h"


using namespace std;

ThreadQueue::ThreadQueue(uint16_t num, uint32_t maxItemLen,  const char *queueName, uint32_t threadId)
    : _num(num), _waitMs(-1), _handle(0), _queueName(queueName), _threadId(threadId)
{
    eap_queue_reset(threadId);
    _handle = eap_queue_create(_num, maxItemLen,queueName, threadId);
	cout << "ThreadQueue created,name:" << queueName << " ,handle: " << _handle << std::endl ;
}

ThreadQueue::~ThreadQueue()
{
	eap_queue_reset(_threadId);
}

int32_t ThreadQueue::ConsumerMsg(uint8_t* buf, uint32_t bufLen)
{
	
	int32_t ret;
    if (-1 == _handle) return -1;
	ret = eap_queue_receive(_handle, (void*)buf, bufLen, _waitMs);
	if(ret < 0){
		EAP_LOG_ERROR("err,ConsumerMsg fail, queue=%s,remainMsg:%hu, %s",_queueName,GetRemainMsgNum(),strerror(errno));
	}else{
		//EAP_LOG_ERROR("****ConsumerMsg queue=%s,remainMsg:%hu",_queueName,GetRemainMsgNum());
	}
	return ret;	
}

int ThreadQueue::ProduceMsg(const uint8_t* buf, uint32_t bufLen)
{
	int32_t ret;
    if (-1 == _handle) return -1;
    ret =  eap_queue_send(_handle, buf, bufLen, 10,0);
	if(ret < 0){
		EAP_LOG_ERROR("err,ProduceMsg fail, queue=%s,remainMsg:%hu, %s",_queueName,GetRemainMsgNum(),strerror(errno));
	}
	return ret;
}

int ThreadQueue::ProduceMsgWithPrio(const uint8_t* buf, uint32_t bufLen,unsigned int prio)
{
	int32_t ret;

    if (-1 == _handle) return -1;
    ret = eap_queue_send(_handle, buf, bufLen, 10,prio);
	if(ret < 0){
		EAP_LOG_ERROR("err,ProduceMsgWithPrio fail, queue=%s,remainMsg:%hu, %s",_queueName,GetRemainMsgNum(),strerror(errno));
	}
	return ret;	
}
uint16_t ThreadQueue::GetRemainMsgNum()
{
    if (-1 == _handle) return 0;
    return eap_queue_remain_num(_handle);
}

void ThreadQueue::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "_num: " << _num;
    cout << EAP_BLANK_PREFIX(level) << "_waitMs: " << _waitMs;
    cout << EAP_BLANK_PREFIX(level) << "_handle: " << _handle;
}
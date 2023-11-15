#include "lwip/sys.h"
#include "xil_printf.h"
#include "eap_os_adapter.h"

unsigned long int eap_os_create_thread(const char *name, Thread_Ob thread_ob, void *arg, int32_t stackSize, int32_t priority)
{
    return (unsigned long int )sys_thread_new(name, thread_ob, arg, stackSize, priority);
}

void eap_os_delete_thread(unsigned long int thread_handle)
{
    vTaskDelete(NULL);
}

void eap_printf(const char *ctrl1, ...)
{
    xil_printf(ctrl1);
}

int eap_queue_send(int32_t xQueue, const void * const pvItemToQueue, uint32_t sendLen, int64_t xTicksToWait,unsigned int prio)
{
    TickType_t waitTicks = (-1 == xTicksToWait) ? portMAX_DELAY : (TickType_t)xTicksToWait;
    return (pdTRUE == xQueueSend((QueueHandle_t)xQueue, pvItemToQueue, waitTicks)) ? 0 : -1;
}

int32_t eap_queue_receive(int32_t xQueue, void * const pvBuffer, uint32_t bufLen, int64_t xTicksToWait)
{
    TickType_t waitTicks = (-1 == xTicksToWait) ? portMAX_DELAY : (TickType_t)xTicksToWait;
    return xQueueReceive((QueueHandle_t)xQueue, pvBuffer, waitTicks);
}

void eap_queue_reset(int32_t xQueue)
{
    xQueueReset((QueueHandle_t)xQueue);
}

void eap_queue_send_from_isr(int32_t xQueue, const void * const pvBuffer, uint32_t sendLen,unsigned int prio)
{
    BaseType_t xHigherPriorityTaskWoken;
    if (xQueueIsQueueFullFromISR((QueueHandle_t)xQueue) == 0)
    {
        xQueueSendFromISR((QueueHandle_t)xQueue, pvBuffer, &xHigherPriorityTaskWoken);
    }
}

int32_t eap_queue_create(uint32_t queueNum, uint32_t queueItemMaxSize)
{
    return (int32_t)xQueueCreate(queueNum, queueItemSize);
}

void eap_thread_delay(uint64_t xTicksToDelay)
{
    vTaskDelay(xTicksToDelay);
}


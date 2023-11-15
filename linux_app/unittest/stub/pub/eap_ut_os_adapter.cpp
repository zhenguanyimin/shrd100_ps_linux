#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <map>
#include <stdarg.h>
#include <unistd.h>

#include "eap_os_adapter.h"

static std::map<uint16_t, int32_t> remoteQueues;
static std::map<uint16_t, int32_t> remoteRespQueues;
static std::map<uint16_t, uint16_t> remoteProcesses;

extern "C"
{

typedef unsigned long u32;
typedef unsigned short u16;
typedef long s32;

unsigned long int eap_os_create_thread(const char *name, Thread_Ob thread_ob, void *arg, int32_t stackSize, int32_t priority)
{
    return 0;
}

void eap_os_delete_thread(unsigned long int thread_handle)
{
}

void eap_os_set_thread_affinity(unsigned long int thread_handle,uint16_t core_id)
{
    
}

void eap_printf(const char *ctrl1, ...)
{
    va_list args;
    va_start(args, ctrl1);
    printf(ctrl1,args);
    va_end(args);
	
}

static Thread_Msg queueMsg;
static bool isQueueEmpty = true;
static int32_t queueHandle = 100;
int eap_queue_send(int32_t xQueue, const void * const pvItemToQueue, uint32_t sendLen, int64_t xMsToWait,unsigned int prio)
{
    memset(&queueMsg, 0, sizeof(queueMsg));
    memcpy(&queueMsg, pvItemToQueue, sizeof(queueMsg));
    isQueueEmpty = false;
    return 0;
}

int32_t eap_queue_create(unsigned int queueNum, uint32_t queueItemMaxSize,const char *queueName, uint32_t queueKey)
{
    return queueHandle;
}

int32_t eap_queue_receive(int32_t xQueue, void * const pvBuffer, uint32_t bufLen, int64_t xMsToWait)
{
    if (isQueueEmpty) return -1;
    memcpy(pvBuffer, &queueMsg, EAP_MIN(sizeof(queueMsg), bufLen));
    isQueueEmpty = true;
    return 0;
}

void eap_queue_send_from_isr(int32_t xQueue, const void * const pvBuffer, uint32_t sendLen,unsigned int prio)
{
    eap_queue_send(xQueue, pvBuffer, sendLen, 0, prio);
}

void eap_thread_delay(uint64_t xMsToWait)
{

}

void eap_queue_reset(int32_t xQueue)
{
    
}

void* eap_create_timer( const char * const pcTimerName, uint64_t periodMs, uint64_t isCycle, void * const pvTimerID, Timer_Ob timer_ob )
{
    return 0;
}

int32_t eap_get_remote_queue_handle(uint16_t threadId)
{
    auto item = remoteQueues.find(threadId);
    if (item != remoteQueues.end())
    {
        return item->second;
    }
    return 0;
}

void eap_set_queue_handle_for_remote(uint16_t threadId, int32_t queueHandle)
{
    remoteQueues[threadId] = queueHandle;
}

void stub_clear_remote_queuehandle()
{
    remoteQueues.clear();
}

int32_t eap_get_remote_resp_queue(uint16_t processId)
{
    auto item = remoteRespQueues.find(processId);
    if (item != remoteRespQueues.end())
    {
        return item->second;
    }
    return 0;
}

void eap_set_resp_queuehandle_for_remote(uint16_t processId, int32_t handle)
{
    remoteRespQueues[processId] = handle;
}

void stub_clear_remote_resp_queuehandle()
{
    remoteRespQueues.clear();
}

int16_t eap_get_remote_processid(uint16_t threadId)
{
    auto item = remoteProcesses.find(threadId);
    if (item != remoteProcesses.end())
    {
        return item->second;
    }
    return 0;
}

void eap_set_processid_for_remote(int16_t threadId, int16_t id)
{
    remoteProcesses[threadId] = id;
}

void stub_clear_remote_processid()
{
    remoteProcesses.clear();
}
}

void eap_print_memory(const void* addr, size_t size)
{
    
}

void* eap_create_mutex()
{
    return nullptr;
}

void eap_mutex_lock(void* mutex)
{

}

void eap_mutex_unlock(void* mutex)
{

}

void eap_log(int priority, const char *format, ...)
{

}

int eap_system(char *cmd)
{
    return 0;
}

uint64_t eap_get_cur_time_ms()
{
    return 0;
}

void eap_mkdir(const char *path)
{

}

void eap_del_file(const char *path)
{
    remove(path);
}

/**
 * @brief load file into a buffer.
 *
 * 
 * @param [in] 	filename    file
 * @param [in] 	buffer  	the data buffer to write
 * @param [in] 	size  		the buffer len to writ
 *
 * @return the pointer of gyro object.
 * @retval 0  		ok
 *         <0  	    fail
 *
 * @see 
 * @note 
 * @warning   Notice: should call free() to free the return buffer.
 */
int eap_file_save(const char *filename, const char *buffer, size_t size) 
{
    FILE *file;
	int ret;
	int len = 0;
	if(!filename || !buffer)
		return -1;
	
    file = fopen(filename, "wb");

    if (file == NULL) {
		ret = -2;
        goto out;
    }

    ret = fwrite(buffer, sizeof(char), size, file);
	if(ret != size){
		ret = -5;
		goto out;
	}
	
	ret = 0;
out:
	if(file)
    	fclose(file);
    return ret;
}


/**
 * @brief load file into a buffer.
 *		Notice:the buffer malloc size is (file size + 1), the last bytes is 0.
 *      the size return is the real file size.
 * @param [in] 	filename    file
 * @param [out] size  		the data len loaded.
 *
 * @return the pointer of gyro object.
 * @retval NULL  		init fail
 *         not NULL  	the pointer of the loaded data.
 *
 * @see 
 * @note 
 * @warning   Notice: should call free() to free the return buffer.
 */
char* eap_file_load(const char *filename, size_t *size) 
{
    FILE *file;
    char *buffer;
    long file_size;

    // 打开文件，以二进制读取模式打开
    file = fopen(filename, "rb");

    // 检查文件是否成功打开
    if (file == NULL) {
        printf("无法打开文件。\n");
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存来存储文件数据
    buffer = (char*)malloc(file_size+1);
    if (buffer == NULL) {
        printf("内存分配失败。\n");
        fclose(file);
        return NULL;
    }

    // 读取文件数据到缓冲区
    fread(buffer, sizeof(char), file_size, file);
	buffer[file_size] = 0;
	
    // 关闭文件
    fclose(file);

    // 设置数据大小
    *size = file_size;

    return buffer;
}

/**
 * @brief check if file exist.
 *		
 * @param [in] 	filename    file
 *
 * @return 
 * @retval 1  		file exist
 *         0        file not exist.
 *
 * @see 
 * @note 
 * @warning   Notice: should call free() to free the return buffer.
 */
int eap_file_exists(char *filename)
{
   return (access(filename, 0) == 0);
}



int64_t eap_get_modity_time(const char *file)
{
    return 0;
}

uint32_t eap_queue_remain_num(int32_t xQueue)
{
    return 0;
}

void eap_set_mqueue_attr(uint32_t msg_max,uint32_t msg_num_size)
{

}

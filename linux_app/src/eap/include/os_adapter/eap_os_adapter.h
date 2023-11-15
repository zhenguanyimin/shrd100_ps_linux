#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "eap_pub.h"

typedef void( *Thread_Ob )();
typedef void( *Timer_Ob )();

unsigned long int eap_os_create_thread(const char *name, Thread_Ob thread_ob, void *arg, int32_t stackSize, int32_t priority);
void eap_os_delete_thread(unsigned long int thread_handle);
void eap_os_set_thread_affinity(unsigned long int thread_handle,uint16_t core_id);
void eap_printf(const char *ctrl1, ...);
int32_t eap_queue_receive(int32_t xQueue, void * const pvBuffer, uint32_t bufLen, int64_t xMsToWait);
int eap_queue_send(int32_t xQueue, const void * const pvItemToQueue, uint32_t sendLen, int64_t xMsToWait,unsigned int prio);
void eap_queue_send_from_isr(int32_t xQueue, const void * const pvBuffer, uint32_t sendLen,unsigned int prio);
void eap_queue_reset(int32_t xQueue);
int32_t eap_queue_create(uint32_t queueNum,uint32_t queueItemMaxSize, const char *queueName, uint32_t queueKey);
uint32_t eap_queue_remain_num(int32_t xQueue);
void eap_thread_delay(uint64_t xMsToWait);
void* eap_create_timer( const char * const pcTimerName, uint64_t periodMs, uint64_t isCycle, void * const pvTimerID, Timer_Ob timer_ob );
int eap_delete_timer(void* timer_id);
uint64_t eap_get_cur_time_ms();
int32_t eap_get_remote_queue_handle(uint16_t threadId);
void eap_set_queue_handle_for_remote(uint16_t threadId, int32_t queueHandle);
int32_t eap_get_remote_resp_queue(uint16_t processId);
void eap_set_resp_queuehandle_for_remote(uint16_t processId, int32_t handle);
int16_t eap_get_remote_processid(uint16_t threadId);
void eap_set_processid_for_remote(int16_t threadId, int16_t id);

/** 
 *     @brief 打印一段内存的十六进制数据及对应的字符
 *            一行16个字节
 
 *     @param addr  内存地址 
 *     @param size  内存长度 
 *     @return      无
 *     @see    
 *     @note    
 */ 
void eap_print_memory(const void* addr, size_t size);

void* eap_create_mutex();
void eap_mutex_lock(void* mutex);
void eap_mutex_unlock(void* mutex);
void eap_destroy_mutex(void* mutex);

void eap_log(int priority, const char *format, ...);
int eap_system(char *cmd);

void eap_mkdir(const char *path);
void eap_del_file(const char *file);

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
int eap_file_save(const char *filename, const char *buffer, size_t size) ;


/**
 * @brief load file into a buffer.
 *
 * 
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
char* eap_file_load(const char *filename, size_t *size) ;

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
int eap_file_exists(char *filename);


int64_t eap_get_modity_time(const char *file);

void eap_set_mqueue_attr(uint32_t msg_max,uint32_t msg_num_size);

#ifdef __cplusplus
}
#endif

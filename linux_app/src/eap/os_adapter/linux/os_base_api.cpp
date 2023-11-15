#include "eap_os_adapter.h"
#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "eap_os_linux.h"

#define TIME_BUF_LEN 20
#define SYS_LOG_BUF_LEN 256


#ifdef __cplusplus
extern "C" {
#endif

static EapOsLinux osLinux;

unsigned long int eap_os_create_thread(const char *name, Thread_Ob thread_ob, void *arg, int32_t stackSize, int32_t priority)
{
    return osLinux.create_thread(name, (LPTHREAD_START_ROUTINE)thread_ob,arg,priority,stackSize);
}

void eap_os_delete_thread(unsigned long int thread_handle)
{
    osLinux.thread_destory(thread_handle,"",0);
}

void eap_os_set_thread_affinity(unsigned long int thread_handle,uint16_t core_id)
{
    osLinux.thread_attach_core(thread_handle,core_id);
}

void eap_printf(const char *ctrl1, ...);

int eap_queue_create(u_int32_t queueNum,uint32_t queueItemMaxSize, const char *queueName, uint32_t queueKey)
{
    return osLinux.eap_queue_create(queueNum,queueItemMaxSize,queueName, queueKey);
}

void eap_queue_reset(int32_t queueKey)
{
    osLinux.eap_queue_destory(queueKey);
}

int eap_queue_send(int32_t queue_id, const void * const send_data, uint32_t sendLen, int64_t xMsToWait,unsigned int prio)
{
    return osLinux.eap_queue_send(queue_id,send_data,sendLen,xMsToWait,prio);
}
int32_t eap_queue_receive(int32_t queue_id, void * const recv_data, uint32_t bufLen, int64_t xMsToWait)
{
    return osLinux.eap_queue_receive(queue_id,recv_data,bufLen,xMsToWait);
}
void eap_queue_send_from_isr(int32_t queue_id, const void * const send_data, uint32_t sendLen,unsigned int prio)
{
  osLinux.eap_queue_send(queue_id,send_data,sendLen,0,prio);
}

uint32_t eap_queue_remain_num(int32_t queue_id)
{
  return osLinux.eap_queue_remain_num(queue_id);
}

void eap_thread_delay(uint64_t xMsToWait)
{
    osLinux.sleep_ms(xMsToWait);
}
void* eap_create_timer(const char * const pcTimerName, uint64_t periodMs, uint64_t isCycle, void * const pvTimerID, Timer_Ob timer_ob)
{
    timer_t time_id = osLinux.eap_create_timer(pcTimerName,1,periodMs,isCycle,(TIMER_JOB)timer_ob);
    return &time_id;
}

int eap_delete_timer(void* timer_id)
{
    return osLinux.eap_delete_timer(timer_id);
}

int32_t eap_get_remote_queue_handle(uint16_t threadId)
{
    return osLinux.eap_get_remote_queue_handle(threadId);
}

void eap_set_queue_handle_for_remote(uint16_t threadId, int32_t queueHandle)
{
    osLinux.eap_set_queue_handle_for_remote(threadId,queueHandle);
}

int32_t eap_get_remote_resp_queue(uint16_t processId)
{
    return osLinux.eap_get_remote_resp_queue(processId);
}

void eap_set_resp_queuehandle_for_remote(uint16_t processId, int32_t handle)
{
    osLinux.eap_set_resp_queuehandle_for_remote(processId,handle);
}

int16_t eap_get_remote_processid(uint16_t threadId)
{
    return osLinux.eap_get_remote_processid(threadId);
}

void eap_set_processid_for_remote(int16_t threadId, int16_t id)
{
    osLinux.eap_set_processid_for_remote(threadId,id);
}

void eap_printf(const char *ctrl1, ...)
{
    va_list args;
    va_start(args, ctrl1);
    osLinux.eap_printf(ctrl1,args);
    va_end(args);
}

void* eap_create_mutex()
{
    return osLinux.eap_create_mutex();
}

void eap_mutex_lock(void* mutex)
{
    osLinux.eap_mutex_lock(mutex);
}

void eap_mutex_unlock(void* mutex)
{
    osLinux.eap_mutex_unlock(mutex);
}

void eap_destroy_mutex(void* mutex)
{
    osLinux.eap_destory_mutex(mutex);
}

void eap_log(int level, const char *format, ...)
{
    //openlog("mysyslog",LOG_CONS | LOG_PID | LOG_PERROR,LOG_USER);
    openlog("mysyslog",0,LOG_USER);
    
    va_list args;
    va_start(args, format);
    vsyslog(level,format,args);
    va_end(args);
    
    closelog();
}
int eap_system(char *cmd)
{
    return 0;
}

uint64_t eap_get_cur_time_ms()
{
  return osLinux.get_time_ms(); 
}

void eap_mkdir(const char *path)
{
    if (access(path, 0))
    {
        mkdir(path, 0777);
    }
}

void eap_del_file(const char *file)
{
    if (!access(file, 0))
    {
        remove(file);
    }
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
    struct stat st = {0};
    stat(file, &st);
    return st.st_mtime;
}

void eap_set_mqueue_attr(uint32_t msg_max,uint32_t msg_num_size)
{
    char sz_msg[56] = {0};
    char sz_msgsize[56] = {0};

    sprintf(sz_msg,"echo \"%d\">/proc/sys/fs/mqueue/msg_max",msg_max);
    sprintf(sz_msgsize,"echo \"%d\">/proc/sys/fs/mqueue/msgsize_max",msg_num_size);
    //sprintf(sz_ulimit,"ulimit -q %d",msg_max*800);
    system(sz_msg);
    system(sz_msgsize);
}

/** 
 *     @brief 打印一段内存的十六进制数据及对应的字符
 *            一行16个字节
 
 *     @param addr  内存地址 
 *     @param size  内存长度 
 *     @return      无
 *     @see    
 *     @note    
 */ 
void eap_print_memory(const void* addr, size_t size) 
{
    const unsigned char* p = (const unsigned char*)addr;
	size_t i;
    for (i = 0; i < (size/16)*16; i++) {
        printf("%02X ", p[i]);
        if ((i + 1) % 16 == 0) {
            printf("  ");
            for (size_t j = i - 15; j <= i; j++) {
                if (isprint(p[j])) {
                    printf("%c", p[j]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
	p = p + (size/16)*16;
    for (i = 0; i < (size%16); i++) {
        printf("%02X ", p[i]);
    }
    for (i = (size%16); i < 16; i++) {
        printf("   ");
    }   
	
	printf("  ");
	for (i = 0; i < (size%16); i++) {
        if (isprint(p[i])) {
            printf("%c", p[i]);
        } else {
            printf(".");
        }
	}		
	printf("\n");
}

#ifdef __cplusplus
}
#endif
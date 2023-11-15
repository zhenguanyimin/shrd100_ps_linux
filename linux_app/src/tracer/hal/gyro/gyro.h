/**
 * @file gyro.h
 * @brief gyro driver.
 *		
 * 
 * @author huaguoqing
 * @date 2023.09.09
 * @see 
 */

#ifndef __GYRO_H__
#define __GYRO_H__

#include <stdint.h>
#include "ev.h"
#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GYRO_PARSE_MSG_BUF  256
#define GYRO_MSG_LEN_MAX    
typedef struct {
	ev_io read_watcher;
	void  *gyro_cb;
}gyro_ev_io;

typedef struct _gyro_cb
{
    int fd;
    struct ev_loop * loop;
	gyro_ev_io gyro_io;

	void *eap_mutex;
	pthread_mutex_t mutex;
		
	int is_inited;

	//cached imu info
	imu_info_t imu;
	uint64_t   imu_tick;//the tick time to got the latest imu info.

	//cache buffer to rcv gyro msgs
	char buf[GYRO_PARSE_MSG_BUF];
	uint32_t buf_len_cached; //how much data cached in buf to be parsed.
	uint64_t   buf_cache_begin_tick;  //the tick to store first data into buf.

}gyro_cb;



/**
 * @brief gyro init func.
 *
 * 
 * @param [in] dev      dev file name,like /dev/ttyXXX
 * @param [in] ev_loop  libev instance pointer.  
 *
 * @return the pointer of gyro object.
 * @retval NULL  		init fail
 *         not NULL  	the pointer of gyro object.
 *
 * @see 
 * @note 
 * @warning 
 */
void* gyro_init(char *dev,  void*ev_loop);

int gyro_fini(void *handle);


/**
 * @brief get gyro angle info
 *
 * 
 * @param [out] angleinfo	 the output angleinfo
 * @param [in] is_need_realtime_data	 1: should be realtime data;  0: the last updated data, maybe already timeout for a long time.	
 *
 * @return 
 * @retval 0		ok ,got info
 *		   <0		fail to get info.
 *
 * @see 
 * @note 
 * @warning 
 */
int gyro_get_angle_info(Drv_AngleInfo *angleinfo, int is_need_realtime_data);



#ifdef __cplusplus
}
#endif


#endif

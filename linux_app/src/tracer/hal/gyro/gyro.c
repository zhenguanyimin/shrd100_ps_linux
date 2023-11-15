/**
 * @file gyro.c
 * @brief gyro driver.
 *
 * 
 * @author huaguoqing
 * @date 2023.09.09
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "eap_sys_drvcodes.h"
#include "eap_pub.h"

#include "gyro.h"

//FIXME: I found eap_mutex not work, when lock, program will crash. so i use pthread mutex instead.
//If fix this bug in future ,we can use eap mutex again.
#define DEBUG_UNSING_EAP_MUTEX 0

#define DEBUG_UNITTEST  1

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define DEBUG_PRINTF_MSG_DATA_ON
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif

#define GYRO_TTY_BAUD_RATE B115200
#define GYRO_TTY_DATA_BITS 8
#define GYRO_TTY_STOP_BIT  1
#define GYRO_TTY_PARITY    0


#define SENSOR_ACCELERATION_TYPE    0x51
#define SENSOR_ANGULAR_TYPE         0x52
#define SENSOR_ANGLE_HEAD    		0x53
#define SENSOR_COMPASS_HEAD    		0x54

#define GYRO_MSG_MAGIC     0x55

#define GYRO_MSG_PUSH_TIME_INTERVAL_MS  	100 
#define GRYO_MSG_DROP_TIMEOUT_MS            (GYRO_MSG_PUSH_TIME_INTERVAL_MS - 10)

#define GYRO_INFO_CACHE_TIMEOUT   			2000
#define GYRO_MSG_LEN                        (11)

static gyro_cb sg_gyro;

static inline void _gyro_reset_rcv_buf(gyro_cb *cb)
{
	cb->buf_len_cached = 0;
	cb->buf_cache_begin_tick = 0;
}

/**
 * @brief check gyro msg integrity.
 *   
 * @param [in]   data            msg addr.
 * @param [in]   data_len        msg len.     
 *
 * @return check result
 * @retval <0  msg error;  
 *         0   msg ok.
 *
 * @see 
 * @note 
 * @warning 
 */
static int _gyro_msg_check(char *data,int data_len)
{
	uint8_t sum = 0x00;
	int i;
	//check magic.
	if(data_len < GYRO_MSG_LEN || data[0] != GYRO_MSG_MAGIC){
		return -1;
	}

	//check crc
	for(i = 0; i < (GYRO_MSG_LEN - 1); i++){
		sum += (uint8_t)data[i];
	}
	if(sum != data[GYRO_MSG_LEN - 1]){
		return -1;
	}
	return 0;
}

/**
 * @brief async read data
 *   在此处理角度和指南针
 *	 Sensor上传了4类数据，且每一类数据长度都是11.
 *	 Type 0x50  时间; 0x51 加速度；Type 0x52 角速度；Type 0x53 角度；Type 0x54 磁场；
 *   数据报头是以0x55 开头，紧接着是类型...，最后一个是累加校验合；
 *   |0x55|  data 9bytes(1bytes Type + 8Bytes data) | 累加校验和1字节|
 * 
 *   如果接收一条完整消息的时间，超过GRYO_MSG_DROP_TIMEOUT_MS，则认为之前缓存的数据不全，需要丢弃。
 *   55 54 CE E9 BA 07 7D FF 00 00 9D
 *   55 53 3C F7 5E 08 56 CD E6 46 90
 *   55 52 09 00 00 00 00 00 9A 13 5D
 *   55 51 5A FE 50 FE A9 07 9A 13 A9 
 *   
 * @param [in]    w
 * @param [out]   revents
 *
 * @return 
 * @retval 
 *
 * @see 
 * @note 
 * @warning 
 */
static void _gyro_read_cb(EV_P_ ev_io *w, int revents)
{
	gyro_ev_io * g = (gyro_ev_io *)w;
	gyro_cb *cb = (gyro_ev_io *)g->gyro_cb;
	uint64_t   cur_tick = eap_get_cur_time_ms();
	int ret;
	float l_roll , l_pitch , l_yaw ;
	imu_info_t  imu_tmp;
	imu_info_t *imu = &imu_tmp;
	SAngle_t angle;
	uint32_t offset = 0;
    char buf[128];
	ssize_t n;
	int i;
	
    if (EV_ERROR & revents) {
        perror("got invalid event");
        return;
    }

    n = read(w->fd, buf, sizeof(buf));
    if (n < 0) {
        perror("read error");
        return;
    }else if(n == 0){
		//TODO
    }

	#ifdef DEBUG_PRINTF_MSG_DATA_ON
		DEBUG_PRINTF("[%s:%d] gyro data comming: <<= %ld, prevoius cached data_len:%d\n",__FUNCTION__, __LINE__,n,cb->buf_len_cached);
		eap_print_memory(buf, n);
	#endif

	if(cb->buf_len_cached && (cur_tick - cb->buf_cache_begin_tick) > GRYO_MSG_DROP_TIMEOUT_MS){
		//drop previous cached data,because cached data already timeout. 
		_gyro_reset_rcv_buf(cb);
	}

	if(!cb->buf_len_cached ){
		//cached buf empty, means a new msg comming, 
		//if wrong magic of new arrived data,just drop it.
		if(buf[0] != GYRO_MSG_MAGIC){
			return;
		}
		//a new msg, save the rcv tick time.
		cb->buf_cache_begin_tick = cur_tick;
	}
	
	//apend data to cache.
	memcpy(cb->buf + cb->buf_len_cached, buf, n);
	cb->buf_len_cached += n;

	if(cb->buf_len_cached < GYRO_MSG_LEN){
		//data not enough,should wait more data coming.
		DEBUG_PRINTF("[%s:%d] need more data: buf_len_cached:%d\n",__FUNCTION__, __LINE__,cb->buf_len_cached);
		return;
	}

	//Ok ,now we have enough data ,begin to parse msg.
#ifdef DEBUG_PRINTF_MSG_DATA_ON
	DEBUG_PRINTF("[%s:%d] *ok, begin parse msg: buf_len_cached:%d\n",__FUNCTION__, __LINE__,cb->buf_len_cached);
	eap_print_memory(cb->buf, cb->buf_len_cached);
#endif

	//maybe multi msgs coming as the same time. We should parse one by one.

	offset = 0;	
	i = 0;
	while((cb->buf_len_cached - offset) >=  GYRO_MSG_LEN){
		DEBUG_PRINTF("[%s:%d] i:%d	buf_len_cached:%d, offset:%d.\n",__FUNCTION__, __LINE__,i,cb->buf_len_cached,offset);
		char *msg = cb->buf + offset;
		//ok, we got the whole msg. first check the msg integrity.
		ret = _gyro_msg_check(msg, GYRO_MSG_LEN);
		if(ret < 0){
			DEBUG_PRINTF("[%s:%d] msg check err, i:%d  buf_len_cached:%d, ret:%d.\n",__FUNCTION__, __LINE__,i,cb->buf_len_cached,ret);
			//msg err, skip this msg.
			offset += GYRO_MSG_LEN;
			i++;
			continue;
		}

		//parse the msg
		switch(msg[1]){	
			case SENSOR_ANGLE_HEAD:
				DEBUG_PRINTF("[%s:%d] parse angle msg: i:%d	buf_len_cached:%d, offset:%d.\n",__FUNCTION__, __LINE__,i,cb->buf_len_cached,offset);
				angle = *(SAngle_t*)&msg[2];
				//转换为角度
				l_roll = (float)angle.Angle[0];
				l_roll = l_roll / 32768 * 180 ;
				l_pitch = (float)angle.Angle[1];
				l_pitch = l_pitch / 32768 * 180 ;
				l_yaw = (float)angle.Angle[2];
				l_yaw = l_yaw / 32768 * 180 ;
				//转换为枪坐标系 
				
				
#if 0				
				imu->roll = l_pitch ;
				imu->pitch = l_roll ;
				imu->yaw = l_yaw ;
				if (imu->yaw <= 0)
				{
					imu->yaw = -imu->yaw;
				}
				else
				{
					imu->yaw = 360 - imu->yaw;
				}
#else
				//转换为TRACER坐标系
				imu->roll = l_pitch ;
				imu->pitch = ((float)90) - l_roll ;
				imu->yaw = ((float)180)-l_yaw ;

#endif
				DEBUG_PRINTF("[%s:%d] angle msg ok,  %d: buf_len_cached:%d, offset:%d, roll:%f,pitch:%f,ywa:%f\n",__FUNCTION__, __LINE__,i,cb->buf_len_cached,offset,imu->roll,imu->pitch,imu->yaw);
#if DEBUG_UNSING_EAP_MUTEX
				eap_mutex_lock(cb->eap_mutex);	
#else

				pthread_mutex_lock(&cb->mutex);
#endif
				cb->imu = *imu;
				cb->imu_tick = cur_tick;
#if DEBUG_UNSING_EAP_MUTEX
				eap_mutex_unlock(cb->eap_mutex);	
#else
				pthread_mutex_unlock(&cb->mutex);
#endif

				break;
			case SENSOR_COMPASS_HEAD:
			case SENSOR_ACCELERATION_TYPE:
			case SENSOR_ANGULAR_TYPE:
			default:
				DEBUG_PRINTF("[%s:%d] i:%d	drop this msg:0x%hhx, buf_len_cached:%d, offset:%d.\n",__FUNCTION__, __LINE__,i,msg[1],cb->buf_len_cached,offset);
				break;
		}
		
		offset += GYRO_MSG_LEN;
		i++;
	}

	//some data left in cache unparsed. should move to the head of cache buf to wait more data coming .
	if(cb->buf_len_cached - offset){
		cb->buf_len_cached = cb->buf_len_cached - offset;
		DEBUG_PRINTF("[%s:%d] left data in cache buf: buf_len_cached:%d\n",__FUNCTION__, __LINE__,cb->buf_len_cached);
		for(i = 0; i < cb->buf_len_cached; i++){
			cb->buf[i] = cb->buf[offset + i];
		}	
		cb->buf_cache_begin_tick = cur_tick;
	}else{
		//all cached data consumed ,reset buf.
		_gyro_reset_rcv_buf(cb);
	}
	
}

static int _gyro_set_tty_parameters(int fd, speed_t baud_rate, int data_bits, int stop_bits, int parity) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr error");
        return -1;
    }

    // 设置波特率
    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);

    // 设置数据位
    tty.c_cflag &= ~CSIZE;
    switch (data_bits) {
        case 5:
            tty.c_cflag |= CS5;
            break;
        case 6:
            tty.c_cflag |= CS6;
            break;
        case 7:
            tty.c_cflag |= CS7;
            break;
        case 8:
            tty.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Invalid data bits\n");
            return -1;
    }

    // 设置停止位
    switch (stop_bits) {
        case 1:
            tty.c_cflag &= ~CSTOPB;
            break;
        case 2:
            tty.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Invalid stop bits\n");
            return -1;
    }

    // 设置校验位
    switch (parity) {
        case 0: // 无校验
            tty.c_cflag &= ~PARENB;
            tty.c_iflag &= ~INPCK;
            break;
        case 1: // 奇校验
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            tty.c_iflag |= INPCK;
            break;
        case 2: // 偶校验
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            tty.c_iflag |= INPCK;
            break;
        default:
            fprintf(stderr, "Invalid parity\n");
            return -1;
    }

    // 其他设置
    tty.c_cflag |= CLOCAL; // 忽略调制解调器控制信号
    tty.c_cflag |= CREAD; // 启用接收器

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 非规范模式，禁止回显和信号处理
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控制
    tty.c_oflag &= ~OPOST; // 禁用输出处理

    tty.c_cc[VMIN] = 0; // 读取的最小字节数
    tty.c_cc[VTIME] = 10; // 读取的超时时间（单位：0.1秒）

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr error");
        return -1;
    }

    return 0;
}

/**
 * @brief get gyro angle info
 *
 * 
 * @param [out] angleinfo    the output angleinfo
 * @param [in] is_need_realtime_data     1: should be realtime data;  0: the last updated data, maybe already timeout for a long time.  
 *
 * @return 
 * @retval 0  		ok ,got info
 *         <0  	    fail to get info.
 *
 * @see 
 * @note 
 * @warning 
 */
int gyro_get_angle_info(Drv_AngleInfo *angleinfo, int is_need_realtime_data)
{
	gyro_cb *cb = &sg_gyro;	
	uint64_t cur_tick = eap_get_cur_time_ms();	
	int ret;

	if(!angleinfo || !cb->is_inited || !cb->imu_tick){
		return -1;
	}

#if DEBUG_UNSING_EAP_MUTEX
	eap_mutex_lock(cb->eap_mutex);	
#else
	pthread_mutex_lock(&cb->mutex);
#endif

	if (!is_need_realtime_data || (cur_tick - cb->imu_tick) < GYRO_INFO_CACHE_TIMEOUT){ 
		angleinfo->roll = cb->imu.roll ;
		angleinfo->pitch = cb->imu.pitch ;
		angleinfo->yaw = cb->imu.yaw ;
		ret = 0;
	}else{
		//the cached imu info already timeout.
		ret = -2;
	}
#if DEBUG_UNSING_EAP_MUTEX
	eap_mutex_unlock(cb->eap_mutex);	
#else
	pthread_mutex_unlock(&cb->mutex);
#endif

	return ret;
}


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
void* gyro_init(char *dev,  void*ev_loop)
{	
	int ret = 0;
	int fd = -1;
	struct ev_loop *loop;
	gyro_cb *cb = &sg_gyro; //FIXME, if having multi gyros, should using malloc instead.
    if(!dev || cb->is_inited || !ev_loop){
		ret = -1;
		goto out;
    }

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open error");
        ret = -3;
		goto out;
    }

    cb->fd = fd;
	cb->loop = ev_loop;

    // set tty params.
    speed_t baud_rate = GYRO_TTY_BAUD_RATE;
    int data_bits = GYRO_TTY_DATA_BITS;
    int stop_bits = GYRO_TTY_STOP_BIT;
    int parity = GYRO_TTY_PARITY;

    if (_gyro_set_tty_parameters(fd, baud_rate, data_bits, stop_bits, parity) != 0) {
        fprintf(stderr, "Failed to set tty parameters\n");
		ret = -4;
		goto out;
	}

#if DEBUG_UNSING_EAP_MUTEX
	cb->eap_mutex = eap_create_mutex();
#endif
	printf("[%s:%d] *********cb->eap_mutex:%p, pthread_mutex_lock:%p\n",__FUNCTION__, __LINE__,cb->eap_mutex,pthread_mutex_lock);
	pthread_mutex_init(&cb->mutex, NULL); 
	
	cb->is_inited = 1;

	//create a read event io.
	cb->gyro_io.gyro_cb = cb;
    ev_io_init(&cb->gyro_io.read_watcher, _gyro_read_cb, cb->fd, EV_READ);
    ev_io_start(cb->loop, &cb->gyro_io.read_watcher);

	ret = 0;
out:
	if(!ret){
		return cb;
	}else{
        fprintf(stderr, "Failed to init gyro\n");
		//init fail.
		if(fd>=0){
			close(fd);
		}	

		return NULL;
	}
	
}

int gyro_fini(void *handle)
{
	//TODO
	printf("FIXME,Not impliment yet\n");
	return 0;
}

#if DEBUG_UNITTEST
void gyro_test_api()
{
	Drv_AngleInfo angleinfo = {0};
	int ret;
	ret = gyro_get_angle_info(&angleinfo,1);
	if(ret < 0){
		printf("[%s:%d] get angle info fail,ret:%d\n",__FUNCTION__, __LINE__,ret);
	}else{
		printf("[%s:%d] roll:%f,pitch:%f, yaw:%f\n",__FUNCTION__, __LINE__,angleinfo.roll,angleinfo.pitch,angleinfo.yaw);
	}
}
#endif

#ifdef __cplusplus
}
#endif

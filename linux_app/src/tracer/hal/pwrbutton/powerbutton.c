/**
# 
# powerbutton.c - serial port(connected to power button) driver
# 
# All rights reserved@Skyfend.com
# 
# Copyright (c) 2023 JiaWei
# @author JiaWei
# @date Sep 12,2023
*/

#include "powerbutton.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#include "eap_sys_drvcodes.h"
#include "eap_pub.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define DEBUG_PRINTF_MSG_DATA_ON
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif

#define POWER_BUTTON_TTY_BAUD_RATE B115200
#define POWER_BUTTON_TTY_DATA_BITS 8
#define POWER_BUTTON_TTY_STOP_BIT  1
#define POWER_BUTTON_TTY_PARITY    0

static powerbutton_cb sg_powerbutton;
pthread_mutex_t mutex;

powerbutton_callback g_powerbutton_callback = 0;

int register_powerbutton_callback(powerbutton_callback fun)
{
    int ret = -1;
    g_powerbutton_callback = fun;

    if (0 != g_powerbutton_callback)
    {
        ret = 0;
    }

    return ret;
}

int test_powerbutton_callback(powerbutton_data data)
{
    printf("powerbutton serial port receives %d,%x,%ld\n", 
        (int)data.valid, (int)data.value, data.buf_cache_begin_tick);
    return 0;
}

static inline void _powerbutton_reset_rcv_buf(powerbutton_cb *cb)
{
    cb->buf_len_cached = 0;
    cb->buf_cache_begin_tick = 0;
}

/**
 * @brief async read data
 *   read one charactor from powerbutton
 *   
 * @param [in]   
 * @param [out]   
 *
 * @return 
 * @retval 
 *
 * @see 
 * @note 
 * @warning 
 */
static void _powerbutton_read_cb(EV_P_ ev_io *w, int revents)
{
    powerbutton_ev_io * g  = (powerbutton_ev_io *)w;
    powerbutton_cb    * cb = (powerbutton_ev_io *)g->powerbutton_cb;
    uint64_t cur_tick = eap_get_cur_time_ms();
    char buf[128];
    ssize_t n;

    if (EV_ERROR & revents)
    {
        perror("got invalid event");
        return;
    }

    n = read(w->fd, buf, sizeof(buf));
    if (n < 0)
    {
        perror("read error");
        return;
    }
    else if(n == 0)
    {
        //TODO
    }

    pthread_mutex_lock(&mutex);

    cb->buf_cache_begin_tick = cur_tick;
    
    //copy data to cache, only has one byte
    memcpy(cb->buf, buf, 1);
    cb->buf_len_cached = 1;

    // Now we have enough one byte data, begin to parse msg
#ifdef DEBUG_PRINTF_MSG_DATA_ON
    DEBUG_PRINTF("[%s:%d] *ok, begin parse msg: buf_len_cached:%d\n", __FUNCTION__, __LINE__, cb->buf_len_cached);
    eap_print_memory(cb->buf, cb->buf_len_cached);
#endif

    pthread_mutex_unlock(&mutex);

    if (0 != g_powerbutton_callback)
    {
        powerbutton_data data;
        data.buf_cache_begin_tick = cb->buf_cache_begin_tick;
        data.value = cb->buf[0];
        data.valid = (cb->buf_len_cached != 0) ? 1 : 0;

        g_powerbutton_callback(data);
    }

    /*
    if (0 != cb->buf_len_cached)
    {
        if (POWER_BUTTON_SHORT_PUSH_VALUE == cb->buf[0])
        {
            printf("power button receives 0x%x\n", cb->buf[0]);
        }
        else if (POWER_BUTTON_LONG_PUSH_VALUE == cb->buf[0])
        {
            printf("power button receives 0x%x\n", cb->buf[0]);
        }
        else
        {
            perror("power button receives unknown charactor");
        }
    }
    */

    // printf("cached powerbutton buffer length %d, value = 0x%x\n", cb->buf_len_cached, (uint16_t)(cb->buf[0]));
}

static int _powerbutton_set_tty_parameters(int fd, speed_t baud_rate, int data_bits, int stop_bits, int parity)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr error");
        return -1;
    }

    // 设置波特率
    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);

    // 设置数据位
    tty.c_cflag &= ~CSIZE;
    switch (data_bits)
    {
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
    switch (stop_bits)
    {
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
    switch (parity)
    {
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

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr error");
        return -1;
    }

    return 0;
}

void powerbutton_get_buf(powerbutton_data *data)
{
    if (0 != data)
    {
        pthread_mutex_lock(&mutex);

        data->buf_cache_begin_tick = sg_powerbutton.buf_cache_begin_tick;
        data->value = sg_powerbutton.buf[0];
        data->valid = (sg_powerbutton.buf_len_cached != 0) ? 1 : 0;

        // clear after read
        sg_powerbutton.buf_cache_begin_tick = 0;
        sg_powerbutton.buf[0] = 0;
        sg_powerbutton.buf_len_cached = 0;

        pthread_mutex_unlock(&mutex);
    }
}

/**
 * @brief powerbutton init func.
 *
 * 
 * @param [in] dev      dev file name,like /dev/ttyXXX
 * @param [in] ev_loop  libev instance pointer.  
 *
 * @return the pointer of powerbutton object.
 * @retval NULL          init fail
 *         not NULL      the pointer of powerbutton object.
 *
 * @see 
 * @note 
 * @warning 
 */
void* powerbutton_init(char *dev,  void*ev_loop)
{
    int ret = 0;
    int fd = -1;
    // struct ev_loop *loop;
    powerbutton_cb *cb = &sg_powerbutton; //FIXME, if having multi powerbuttons, should using malloc instead.

    if(!dev || cb->is_inited || !ev_loop)
    {
        ret = -1;
        goto out;
    }

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("open error");
        ret = -3;
        goto out;
    }

    cb->fd = fd;
    cb->loop = ev_loop;

    // set tty params.
    speed_t baud_rate = POWER_BUTTON_TTY_BAUD_RATE;
    int data_bits = POWER_BUTTON_TTY_DATA_BITS;
    int stop_bits = POWER_BUTTON_TTY_STOP_BIT;
    int parity = POWER_BUTTON_TTY_PARITY;

    if (_powerbutton_set_tty_parameters(fd, baud_rate, data_bits, stop_bits, parity) != 0)
    {
        fprintf(stderr, "Failed to set tty parameters\n");
        ret = -4;
        goto out;
    }

    cb->is_inited = 1;

    //create a read event io.
    cb->powerbutton_io.powerbutton_cb = cb;
    ev_io_init(&cb->powerbutton_io.read_watcher, _powerbutton_read_cb, cb->fd, EV_READ);
    ev_io_start(cb->loop, &cb->powerbutton_io.read_watcher);

    ret = 0;
out:
    if(!ret)
    {
        return cb;
    }
    else
    {
        fprintf(stderr, "Failed to init powerbutton\n");
        //init fail.
        if(fd>=0){
            close(fd);
        }    

        return NULL;
    }
    
}

int powerbutton_fini(void *handle)
{
    //TODO
    printf("FIXME,Not impliment yet\n");
    return 0;
}

#ifdef __cplusplus
}
#endif

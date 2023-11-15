/**
# 
# powerbutton.h - serial port (connected to power button) driver
# 
# All rights reserved@Skyfend.com
# 
# Copyright (c) 2023 JiaWei
# @author JiaWei
# @date Sep 12,2023
*/

#ifndef __POWERBUTTON_H__
#define __POWERBUTTON_H__

#include <stdint.h>
#include "ev.h"
#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_BUTTON_SHORT_PUSH_VALUE     0x5A
#define POWER_BUTTON_LONG_PUSH_VALUE      0xA5

#define POWER_BUTTON_PARSE_MSG_BUF  32

typedef struct 
{
    ev_io read_watcher;
    void  *powerbutton_cb;
}powerbutton_ev_io;

typedef struct _powerbutton_data
{
    uint64_t buf_cache_begin_tick;
    char value;
    char valid;
}powerbutton_data;

typedef struct _powerbutton_cb
{
    int fd;
    struct ev_loop * loop;
    powerbutton_ev_io powerbutton_io;

    int is_inited;

    //cache buffer to rcv powerbutton msgs
    char buf[POWER_BUTTON_PARSE_MSG_BUF];
    uint32_t buf_len_cached; //how much data cached in buf to be parsed.
    uint64_t buf_cache_begin_tick;  //the tick to store first data into buf.

}powerbutton_cb;

typedef int (* powerbutton_callback)(powerbutton_data data);

int register_powerbutton_callback(powerbutton_callback fun);

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
void* powerbutton_init(char *dev,  void*ev_loop);

void powerbutton_get_buf(powerbutton_data *data);

int powerbutton_fini(void *handle);

#ifdef __cplusplus
}
#endif

#endif


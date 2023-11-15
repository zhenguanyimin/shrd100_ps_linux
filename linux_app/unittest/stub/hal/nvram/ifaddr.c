/**
 * @file nvram_cust.c
 * @brief nvram driver.
 *
 * 
 * @author fanghaohua
 * @date 2023.09.26
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termios.h>
//#include "eap_sys_drvcodes.h"
//#include "eap_pub.h"

#include "ifaddr.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG_NVRAM
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif


int get_ifaddr_wifi(uint8_t *addr, uint32_t len)
{
    return 0;
}


#ifdef __cplusplus
}
#endif

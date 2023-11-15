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

#include "nvram_cust.h"

#define DEBUG_UNITTEST 1
#define DEBUG_NVRAM 1

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG_NVRAM
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif


int nvram_read_sn(uint8_t *sn, uint32_t *len)
{
    return 0;
}

int nvram_write_sn(uint8_t *sn, uint32_t len)
{
    return 0;
}

#if DEBUG_UNITTEST
void nvram_test_api()
{
    DEBUG_PRINTF("[%s:%d] FIX ME! Not impliment yet!\n", __FUNCTION__, __LINE__);
}
#endif

#ifdef __cplusplus
}
#endif

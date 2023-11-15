/**
 * @file nvram_cust.h
 * @brief nvram driver.
 *		
 * 
 * @author fanghaohua
 * @date 2023.09.26
 * @see 
 */

#ifndef __IFADDR_H__
#define __IFADDR_H__

#ifdef __cplusplus
extern "C" {
#endif

    int get_ifaddr_wifi(uint8_t *addr, uint32_t len);

#ifdef __cplusplus
}
#endif


#endif

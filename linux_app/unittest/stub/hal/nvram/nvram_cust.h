/**
 * @file nvram_cust.h
 * @brief nvram driver.
 *		
 * 
 * @author fanghaohua
 * @date 2023.09.26
 * @see 
 */

#ifndef __NVRAM_CUST_H__
#define __NVRAM_CUST_H__

#ifdef __cplusplus
extern "C" {
#endif

    int nvram_read_sn(uint8_t *sn, uint32_t *len);
    int nvram_write_sn(uint8_t *sn, uint32_t len);

#ifdef __cplusplus
}
#endif


#endif

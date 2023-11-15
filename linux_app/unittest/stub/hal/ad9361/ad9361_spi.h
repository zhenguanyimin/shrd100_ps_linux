#ifndef _AD9361_H_
#define _AD9361_H_
#include <stdint.h>
// #include <xil_types.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

void SetAd(u8 *psetdat);
void delayMs(u8 dlyms);
void selectSpi(u8 spinum);
u8 ad9361_spi_read_byte(u16 addr);
void ad9361_spi_write_byte(u16 addr,u8 data);
// void ad9361_spi_test(void);
void vTaskDelay( uint32_t dlyus );

#endif


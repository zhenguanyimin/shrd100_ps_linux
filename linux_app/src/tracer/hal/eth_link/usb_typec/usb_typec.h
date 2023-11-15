
#ifndef _USB_TYPEC_H_
#define _USB_TYPEC_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sys/types.h"
#include "../socket/tcp_client.h"


typedef enum
{// USB-TypC I2C, used to set USB-Type mode: Device(UFP Mode), Host(DFP Mode),Dual Role (DRP Mode)
    TYPEC_MODE_DEVICE = 0x0, // Device(UFP Mode)
    TYPEC_MODE_HOST = 0x2, // Host(DFP Mode)
    TYPEC_MODE_DUAL_ROLE = 0x4, // Dual Role (DRP Mode)
    TYPEC_MODE_UNKNOWN = 0xFF, //UnKnown
} UsbTypecMode;

// USB-TypC I2C, used to set USB-Type mode: Device(UFP Mode), Host(DFP Mode),Dual Role (DRP Mode)
bool usb_typec_InitDev(const char* pDevName, uint16_t pSlaveAddr);
bool usb_typec_SetMode(const char* pDevName, uint16_t pSlaveAddr, UsbTypecMode pMode);
uint8_t usb_typec_GetMode(const char* pDevName, uint16_t pSlaveAddr);


int32_t usb_typec_client_create( tcp_client_t *psClient, tcp_client_cfg_t *psCfg, tcp_client_cbk_t *psCbk );

int32_t usb_typec_client_send( tcp_client_t *psClient, void *pvData, uint32_t uLength );

int32_t usb_typec_client_close( tcp_client_t *psClient );
#ifdef __cplusplus 
} 
#endif 
#endif


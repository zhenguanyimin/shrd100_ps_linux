
#ifndef _USB_TYPEC_H_
#define _USB_TYPEC_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sys/types.h"
#include "tcp_client.h"


int32_t usb_typec_client_create( tcp_client_t *psClient, tcp_client_cfg_t *psCfg, tcp_client_cbk_t *psCbk );

int32_t usb_typec_client_send( tcp_client_t *psClient, void *pvData, uint32_t uLength );

int32_t usb_typec_client_close( tcp_client_t *psClient );
#ifdef __cplusplus 
} 
#endif 
#endif


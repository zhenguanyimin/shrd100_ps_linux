
#ifndef _C2_ALINK_H_
#define _C2_ALINK_H_

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum alink_dev_id
{
	ALINK_DEV_ID_RADAR		= (3),
	ALINK_DEV_ID_PC			= (4),
	ALINK_DEV_ID_C2_BLE		= (5),
	ALINK_DEV_ID_C2			= (6),
	ALINK_DEV_ID_UAV		= (7),
}alink_dev_id_t;

typedef enum alink_port_id
{
	ALINK_PORT_ID_TCP_SRC	= (1),
	ALINK_PORT_ID_WIFI		= (2),
	ALINK_PORT_ID_TCP_C2	= (3),
	ALINK_PORT_ID_TCP_UAV	= (4),
}alink_port_id_t;

int32_t alink_init( void );

#ifdef __cplusplus
}
#endif

#endif

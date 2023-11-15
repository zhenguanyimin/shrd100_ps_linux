
#ifndef _C2_NETWORK_H_
#define _C2_NETWORK_H_

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

typedef enum    {
  NET_PROTO_TYPE_ALINK = 0,
  NET_PROTO_TYPE_TRACER_UAV = 1,  //alink,0xe5 msgid; pb payload; 
  NET_PROTO_TYPE_TRACER_UAVCONTROLLER = 2  //json string 
}NET_PROTO_TYPE;


typedef struct _c2_network_init_param
{
	uint32_t net_proto_type;//NET_PROTO_TYPE
	const char *         typec_tty_dev; //etc: /dev/ttyGS1
}c2_network_init_param;


int32_t c2_network_init( c2_network_init_param *init_param );

#ifdef __cplusplus
}
#endif


#endif

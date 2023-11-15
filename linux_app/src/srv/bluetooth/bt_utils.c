#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "bt_utils.h"
#include "bt_gatt_server_info.h"

#ifdef __cplusplus
}
#endif

#if 0
#define DEBUG_BT_UTILS(arg...) printf(arg)
#else
#define DEBUG_BT_UTILS(arg...) 
#endif

char * convert_call_state_to_string(uint8_t state)
{
	switch(state) {
		case 0:
			return "ACTIVE";
		case 1:
			return "HELD";
		case 2:
			return "DIALING";
		case 3:
			return "ALERTING";
		case 4:
			return "INCOMING";
		case 5:
			return "WAITING";
		case 6:
			return "HELD_BY_RESPONSE_AND_HELD";
		case 7:
			return "TERMINATE";
		default:
			return "ERROR";
	}
}

unsigned char parse_char_with_base(char *p, int base)
{
	char buf[2] = {0};
	memcpy(buf, p, 1);
	return strtol(buf, NULL, base);
}

void convert_uuid_order(char *out, char *in, int len)
{
	int i = 0;
	for (i; i < len; i++) {
		memcpy(out + i, in + (len - i - 1), 1);
	}
}

void printfUuid(unsigned char *tag, bt_uuid_t uuid)
{
	DEBUG_BT_UTILS(" %s: - {%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n\n", tag, uuid.uu[0], uuid.uu[1], uuid.uu[2], uuid.uu[3], uuid.uu[4], uuid.uu[5], uuid.uu[6], uuid.uu[7], uuid.uu[8], uuid.uu[9], uuid.uu[10], uuid.uu[11], uuid.uu[12], uuid.uu[13], uuid.uu[14], uuid.uu[15]);
}
void print_hex(const char* prefix,unsigned char* data,int len){
	int i;

	//char str[4096] = {0};
	char* str = (char*)malloc(len*3+100);
	if(str == NULL){
		printf("print_hex malloc err");
		return;
	}
	int str_len = 0;
	if(len == 0){
		free(str);
		return;
	}

	str_len += sprintf(str, "%s",prefix);
	for(i=0;i<len;i++){
		str_len += sprintf(str + str_len, "%02X ",data[i]);
	}
	DEBUG_BT_UTILS("%s",str);
	free(str);
}
void printfServerInfo(void * server_info)
{
	server_info_t *p = (server_info_t *)server_info;
	//TODO
}

void quit_client()
{
	DEBUG_BT_UTILS("quit client process\r\n");
	exit(1);
}

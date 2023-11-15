//#include <stdio.h>
//#include <string.h>
#ifdef __cplusplus
	extern "C" {
#endif

#include "bluetooth.h"

char * convert_call_state_to_string(uint8_t state);
unsigned char parse_char_with_base(char *p, int base);
void printfUuid(unsigned char *tag, bt_uuid_t uuid);
void convert_uuid_order(char *out, char *in, int len);
void printfServerInfo(void * server_info);
void print_hex(const char* prefix,unsigned char* data,int len);
void quit_client();

#ifdef __cplusplus
}
#endif



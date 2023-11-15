#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "bluetooth.h"



#define MAX_DEVICE_NUM 16

#define PROPERTIES_MENU		0xEB
#define SPP_MENU			0xEC
#define HFP_MENU			0xED
#define A2DP_AVRCP_MENU		0xEE
#define PBAP_MENU			0xEF
#define GATT_CLIENT_MENU	0xEA
#define GATT_SERVER_MENU	0xE9
#define BACK				0xFE
#define QUIT				0xFF
#define ENTER				0xF0

typedef struct tag_device {
	int idex;
	bt_bdaddr_t address;
	bt_bdname_t name;
} bluetooth_device_t;

typedef struct tag_conext {
	int pending_cmd;
	bluetooth_device_t devices[MAX_DEVICE_NUM];
	int target_device;
	int current_dev_num;
	int device_offset;
} context_t;

extern context_t context;

#define DBGPRINT(fmt,...) \
	fprintf(stdout, "[Test-D] " fmt "%s", ##__VA_ARGS__, ((sizeof(fmt)>=2)&&(fmt[sizeof(fmt)-2]=='\n'))?"":"\n");\
//	fclose(stdout);
#define printf(fmt,...) \
	fprintf(stdout, "[Test-E] " fmt "%s", ##__VA_ARGS__, ((sizeof(fmt)>=2)&&(fmt[sizeof(fmt)-2]=='\n'))?"":"\n");\
//	fclose(stderr);

int choose_device();

extern void bluetooth_init(void);

#ifdef __cplusplus
}
#endif

#endif


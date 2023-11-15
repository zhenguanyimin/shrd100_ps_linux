#include <stdio.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_gatt_server.h"
#include "goc_gatt_client.h"
#include "bt_gatt.h"
#include "goc_bluetooth.h"
#include "goc_sdk.h"
#include "bluetooth.h"
#include "goc_gatt_server.h"

#ifdef __cplusplus
}
#endif

#if 0
#define DEBUG_BT_GATT(arg...) printf(arg)
#else
#define DEBUG_BT_GATT(arg...) 
#endif

/*FIXME these 2 should be a random UUID*/ //GATT Server服务的UUID
static bt_uuid_t s_uuid = {0x00, 0x00, 0x12, 0x34, 0x45, 0x56, 0x78, 0x00,
                           0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44
                          };

static int enter_command_line();
static int execute_command(int *cmd, int is_pending);
static void init_profiles();
static void cleanup_profiles();

static void goc_state_changed_cb(uint8_t state) //蓝牙开关状态
{
	bt_status_t return_state;

	if (state == BT_STATE_ON) {
		DEBUG_BT_GATT("Bluetooth is enabled!\n");
		
		//注册gatt server
		return_state = goc_gatt_server_register_app(s_uuid);
		if(BT_STATUS_SUCCESS != return_state){
			printf("[%s, %d] bluetooth enable err!\n", __func__, __LINE__);
		}else{
			DEBUG_BT_GATT("[%s, %d] gatt server register ok!\n", __func__,__LINE__);
		}

	} else if (state == BT_STATE_OFF) {
		DEBUG_BT_GATT("Bluetooth is disabled!\n");
	}
}

static void bt_version_cb(char * version)//蓝牙SDK协议栈版本号
{
	DEBUG_BT_GATT("gocsdk Version:%s.\n", version);
}


static bluetooth_callback_t s_bluetooth_cbks = {
	goc_state_changed_cb,
	bt_version_cb,
	NULL,//功能不需要不需要实现
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static int enter_local_properties_command_line()
{
	int cmd = 0;
	char buffer[16];

	printf("\n");
	printf("0.	GO to uplater menu.\n");
	printf("1.	Get bluetooth state.\n");
	printf("2.	Get name of local device.\n");
	printf("3.	Get address of local device.\n");
	printf("4.	Set name of local device.\n");
	printf("\n");
	printf("Please choose a command(enter the index of command, 'q' to quit.):\n");
	printf(">");
	fgets(buffer, 16, stdin);

	if (buffer[0] == 'q' || buffer[0] == 'Q') {
		quit_client();
	} else if (buffer[0] == 'b' || buffer[0] == 'B') {
		return 0;
	} else if (buffer[0] == '\n') {
		return ENTER;
	}

	cmd = atoi(buffer);

	return cmd;
}

static int execute_local_properties_command(int cmd)
{
	bt_bdname_t *name;
	bt_bdaddr_t *addr;
	uint32_t cod;
	uint32_t scan_mode;
	int i, j, number = 8;
	bt_remote_device_t devices[8];
	uint8_t *address;
	char buffer[128];
	bt_bdname_t local_name = {0};
	int svc_num;
	uint16_t *svcs;

	switch (cmd) {
		case 1:
			printf("Bluetooth state is %s.\n", goc_bluetooth_get_state() == BT_STATE_ON ? "enabled" : "disabled");
			break;
		case 2:
			name = goc_bluetooth_get_name();
			if (name) {
				printf("Loal device name:%s.\n", name->name);
			}
			break;
		case 3:
			addr = goc_bluetooth_get_address();
			if (addr) {
				printf("Local device address:%02X:%02X:%02X:%02X:%02X:%02X.\n",
				       addr->address[0], addr->address[1], addr->address[2], addr->address[3], addr->address[4], addr->address[5]);
			}
			break;
		case 4:
			printf("Please enter the device name:\n>");
			fgets(buffer, 128, stdin);
			memcpy(local_name.name, buffer, strlen(buffer) - 1/*ignore the last 0x0A*/);
			goc_bluetooth_set_name(local_name);
			break;
		case 0:
			return BACK;
		default :
			printf(" NOT DEFINR COMMAND !\n");
	}
	return PROPERTIES_MENU;
}

static int enter_command_line()
{
	int cmd = 0;
	char buffer[16];

	printf("\n");
	printf("1.	Enable the bluetooth.\n");
	printf("2.	Disable the bluetooth.\n");
	printf("3.	Set/Get local properties.\n");
	printf("4.	Enter GATT_SERVER subcommands.\n");
	printf("\n");
	printf("Please choose a command(enter the index of command, 'q' to quit.):\n");
	printf(">");
	fgets(buffer, 16, stdin);

	if (buffer[0] == 'q' || buffer[0] == 'Q') {
		quit_client();
	}

	cmd = atoi(buffer);

	return cmd;
}

static int execute_command(int *cmd, int is_pending)
{
	switch (*cmd) {
		case 1:
			goc_bluetooth_enable();//打开蓝牙
			break;
		case 2:
			goc_bluetooth_disable();//关闭蓝牙
			break;
		case 3:
			*cmd = enter_local_properties_command_line();
			return execute_local_properties_command(*cmd);
			break;
		case 4:
			*cmd = enter_gatt_server_command_line();
			return execute_gatt_server_command(*cmd);
			break;
	}
	return -1;
}

static int profiles_initialized = 0;
static void init_profiles()
{
	goc_gatt_server_init(hlp_gatt_server_get_cbks());//初始gatt server的服务
	profiles_initialized = 1;
}

static void cleanup_profiles()
{
	if (!profiles_initialized) {
		return;
	}

	profiles_initialized = 0;
	goc_gatt_server_done();
}

void bluetooth_init(void)
{
	int ret ;
	bt_status_t return_state;

	int cmd = 0;
	int execute_result = -1;
	
	printf("[%s, %d] start to init!\n", __func__,__LINE__);
	
	ret = goc_sdk_init();//使用默认参数节点
	if (ret  > 0) {
		printf("[%s, %d]Server connected!\n", __func__, __LINE__);
	} else {
		printf("[%s, %d]Can NOT connect to the bluetooth server.\n", __func__,__LINE__);
		return;
	}
	
	//初始通用的接口，注册回调
	goc_bluetooth_init(&s_bluetooth_cbks);
	printf("[%s, %d]\n", __func__,__LINE__);

	init_profiles();//初始协议

	//打开蓝牙
	return_state = goc_bluetooth_enable();
	if(BT_STATUS_SUCCESS != return_state){
		printf("[%s, %d] bluetooth enable err!\n", __func__, __LINE__);
	}
	
	printf("[%s, %d] int finished!\n", __func__,__LINE__);

}





#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
	extern "C" {
#endif

#include "bt_gatt.h"
#include "bt_utils.h"
#include "bt_gatt_server.h"
#include "bt_gatt_server_info.h"
#include "../../tracer/hal/eth_link/eth_link.h"
#include "../../eap/include/pub/eap_sys_cmds.h"
#ifdef __cplusplus
}
#endif

#if 0
#define DEBUG_BLUETOOTH_SERVER(arg...) printf(arg)
#else
#define DEBUG_BLUETOOTH_SERVER(arg...) 
#endif


static int send_data_count = 0;

static bt_uuid_t null_uuid = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*FIXME these 2 should be a random UUID*/ //GATT Server服务的UUID
static bt_uuid_t back_s_uuid = {0x00, 0x00, 0x12, 0x34, 0x45, 0x56, 0x78, 0x00,0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44};

static int ATT_MTU = 18;//FIXME may be not 18
static char long_data[MAX_VALUE_LEN] = {0};
static int long_chara_handle = -1;
static int long_data_len = 0;


static bt_uuid_t DEFAULT_SRVC_UUID = {0xff, 0xf1, 0x18, 0xff, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
static bt_uuid_t CHARA_UUID_1 = {0xff, 0xf2, 0x18, 0x07, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
#if 0
static bt_uuid_t CHARA_UUID_2 = {0x00, 0x00, 0x18, 0x08, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
#endif 

static bt_uuid_t DESC_UUID_1 = {0x00, 0x00, 0x18, 0xee, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};
static bt_uuid_t DESC_UUID_2 = {0x00, 0x00, 0x29, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};

static GATT_Service_t s_gatt_srvc;
static GATT_Declaration_t *s_cur_declar;

typedef struct tag_gatt_server {
	uint8_t server_if;
	uint16_t conn_id;
	uint8_t is_connected;
} gatt_server_t;

static gatt_server_t m_gatt_server = { -1, -1, -1};

void continue_service_declaration(GATT_Declaration_t * declar, int status, int srvcHandle);
int get_attr_handle_type(uint16_t attr_handle);
void server_info_add_included_service(uint16_t srvc_handle, uint16_t incl_srvc_handle);
static uint8_t *get_chara_value_from_gatt_service(GATT_Service_t *gatt_srvc, bt_uuid_t char_uuid);
static uint16_t get_chara_value_len_from_gatt_service(GATT_Service_t gatt_srvc, bt_uuid_t char_uuid);
//static void add_gatt_server_service();


uint8_t gatt_get_server_if(void)
{
	return m_gatt_server.server_if;
}

bt_uuid_t gatt_get_uuid()
{
	return CHARA_UUID_1;
}


bt_uuid_t gatt_get_server_uuid()
{
	return DEFAULT_SRVC_UUID;
}



static void server_registered_cbk(uint8_t status, uint8_t server_if, bt_uuid_t uuid)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT_SERVER: on_server_regsitered: status : %d, server_if: %d\n", status, server_if);
	printfUuid("server UUID:", uuid);
	char uu[16];
	if (memcmp(uuid.uu, uu, 16) == 0) {//判断注册和回掉上报的uuid是否相同
		DEBUG_BLUETOOTH_SERVER("-------- server_if: %d \n", server_if);
		m_gatt_server.server_if = server_if;
		server_info_update_server_if(server_if);
	}
	add_gatt_server_service();
}

static void client_connected_cbk(bt_bdaddr_t address, uint8_t connected, uint16_t conn_id, uint8_t server_if)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT_SERVER: on_connected: address : {%02x:%02x:%02x:%02x:%02x:%02x} , *** connected : %d ***, conn_id: %d, server_if: %d\n", address.address[0], address.address[1], address.address[2], address.address[3], address.address[4], address.address[5],
	       connected, conn_id, server_if);

	if (connected) {
		m_gatt_server.conn_id = conn_id;
		server_info_update_conn_id(conn_id);
	} else {
		m_gatt_server.conn_id = -1;
	}
	m_gatt_server.is_connected = connected;
}

static void service_added_cbk(uint8_t status, uint8_t server_if, uint8_t svc_type, uint16_t svc_inst_id, bt_uuid_t srvc_uuid, uint16_t handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT_SERVER: on_service_added: status: %d, server_if: %d, svc_type: %d, srvcInsdId: %d, handle: %d \n", status, server_if, svc_type, svc_inst_id, handle);
	printfUuid("srvcUUID", srvc_uuid);


	server_info_add_service(svc_type, svc_inst_id, srvc_uuid, handle);//保存的service info,主要是handle信息
	continue_service_declaration(s_cur_declar, status, handle);

}
static void included_service_added_cbk(uint8_t status, uint8_t server_if, uint16_t srvc_handle, uint16_t incl_srvc_handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER: on_included_srvc_added: status: %d, srvc_handle: %d, incl_srvc_handle: %d\n", status, srvc_handle, incl_srvc_handle);

	server_info_add_included_service(srvc_handle, incl_srvc_handle);//保存的include info,主要是handle信息

	continue_service_declaration(s_cur_declar, status, srvc_handle);
}

static void characteristic_added_cbk(uint8_t status, uint8_t server_if, bt_uuid_t char_uuid, uint16_t srvc_handle, uint16_t char_handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER on_chara_added: status: %d, server_if: %d, srvc_handle: %d, char_handle: %d\n", status, server_if,
	       srvc_handle, char_handle);
	printfUuid("charUUID", char_uuid);

	server_info_add_characteristic(char_uuid, srvc_handle, char_handle);//保存的characteristic info,主要是handle信息

	uint16_t valueLen = get_chara_value_len_from_gatt_service(s_gatt_srvc, char_uuid);
	uint8_t *data_value = get_chara_value_from_gatt_service(&s_gatt_srvc, char_uuid);
	DEBUG_BLUETOOTH_SERVER("-----valuelen %d\n",valueLen);
	print_hex("------data_value:",data_value,valueLen);
	server_info_update_characteristic_value(srvc_handle, char_handle, valueLen, data_value);

	continue_service_declaration(s_cur_declar, status, srvc_handle);

}

static void descriptor_added_cbk(uint8_t status, uint8_t server_if, bt_uuid_t descr_uuid, uint16_t srvc_handle, uint16_t descr_handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER: on_desc_added: status: %d, server_if: %d, srvc_handle: %d, descr_handle: %d\n", status, server_if, srvc_handle, descr_handle);
	printfUuid("descUUID", descr_uuid);

	server_info_add_descriptor(descr_uuid, srvc_handle, descr_handle);

	continue_service_declaration(s_cur_declar, status, srvc_handle);
}

static void service_started_cbk(uint8_t status, uint8_t server_if, uint16_t srvc_handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on_service_started : status : %d, server_if: %d, srvc_handle: %d\n", status, server_if, srvc_handle);
}


static void response_send_completed_cbk(uint8_t status, uint16_t handle)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on_response_send, handle : %d\n", handle);
}

static void attribute_read_cbk(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t attr_handle, uint16_t offset, uint8_t is_long)
{

	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on_attribute_read_req, conn_id: %d, trans_id: %d, attr_handle: %d, offset : %d, is_long: %d\n", conn_id, trans_id, attr_handle, offset, is_long);

	if (get_attr_handle_type(attr_handle) == TYPE_CHARACTERISTIC) {
		DEBUG_BLUETOOTH_SERVER("This is Chara Read\n");
		GATT_Chara_t *gatt_chara = server_info_get_gatt_chara_by_handle(UNKNOWN_HANDLE, attr_handle);
		if (gatt_chara) {
			uint16_t data_len = gatt_chara->valueLen;
			uint8_t *data_value = malloc(gatt_chara->valueLen);
			memcpy(data_value, gatt_chara->value, data_len);
			DEBUG_BLUETOOTH_SERVER("-----valuelen %d\n",data_len);
			print_hex("------data_value:",data_value,data_len);
			goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_SUCCESS, attr_handle, offset, 0, data_len, data_value);

			if (data_value) {
				free(data_value);
			}
		} else {
			printf("ERROR \n");
			char data[0] = {};
			goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_FAILURE, attr_handle, offset, 0, 0, data);
		}
	} else {
		//TODO
		DEBUG_BLUETOOTH_SERVER("This is Desc Read\n");
		char data[0] = {};
		goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_FAILURE, attr_handle, offset, 0, 0, data);
	}

}

static void attribute_write_cbk(bt_bdaddr_t address, uint16_t conn_id, uint16_t trans_id, uint16_t attr_handle, uint16_t offset, uint8_t need_rsp, uint8_t is_prep, uint16_t valueLen, uint8_t * value)
{
	eth_protocol_head_t *psHead = NULL;
	char data_value[valueLen + 1];
	memset(data_value, 0, valueLen + 1);
	memcpy(data_value, value, valueLen);
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on_attribute_write_req, conn_id: %d, trans_id: %d, attr_handle: %d, offset: %d, need_rsp: %d, is_prep: %d, valueLen: %d, value : %s \n", conn_id, trans_id, attr_handle, offset, need_rsp, is_prep, valueLen, data_value);
	if (get_attr_handle_type(attr_handle) == TYPE_CHARACTERISTIC) {
		if (is_prep) {
			//TODO this is long_chara write
			long_chara_handle = attr_handle;
			if (offset == 0) {
				//first part
				memset(long_data, 0, MAX_VALUE_LEN);
				memcpy(long_data, data_value, valueLen);
				long_data_len = valueLen;
			} else {
				memcpy(long_data + long_data_len, data_value, valueLen);
				long_data_len += valueLen;
				if (long_data_len > MAX_VALUE_LEN) {
					printf("WARNING: chara value len is over MAX_VALUE_LEN\n");
				}
			}
			//goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_SUCCESS, attr_handle, offset, 0, valueLen, data_value);
			/* send the read  data to alink to analyze */
			psHead = (eth_protocol_head_t*)(data_value);
			if(psHead){
				DEBUG_BLUETOOTH_SERVER("[%s, %d]psHead->msgid = 0x%x, m_gatt_server.server_if = 0x%x, attr_handle = 0x%x\n", __func__, __LINE__, psHead->msgid, m_gatt_server.server_if, attr_handle);
				if(EAP_ALINK_CMD_TCP_WIFI_INFO == psHead->msgid){
					send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_BLE_BT, 0xffff, data_value, valueLen);
				}
			}
		} else {
			//set value to local
			server_info_update_characteristic_value(UNKNOWN_HANDLE, attr_handle, valueLen, data_value);

			//goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_SUCCESS, attr_handle, offset, 0, valueLen, data_value);
			/* send the read  data to alink to analyze */
			psHead = (eth_protocol_head_t*)(data_value);
			if(psHead){
				DEBUG_BLUETOOTH_SERVER("[%s, %d]psHead->msgid = 0x%x, m_gatt_server.server_if = 0x%x, attr_handle = 0x%x\n", __func__, __LINE__, psHead->msgid, m_gatt_server.server_if, attr_handle);
				if(EAP_ALINK_CMD_TCP_WIFI_INFO == psHead->msgid){
					send_alinkmsg_to_eap(EAP_CHANNEL_TYPE_BLE_BT, 0xffff, data_value, valueLen);
				}
			}
		}

	} else {
		DEBUG_BLUETOOTH_SERVER("This is Desc write\n");
		int i = 0;
		for (i; i < valueLen; i++) {
			DEBUG_BLUETOOTH_SERVER("value : %d\n", *(value + i));
		}
		//goc_gatt_server_send_response(m_gatt_server.server_if, conn_id, trans_id, GATT_SUCCESS, attr_handle, offset, 0, valueLen, data_value);
	}

}


static void notification_sent_cbk(uint16_t conn_id, uint8_t status)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on-notify_send, conn_id: %d, status : %d\n", conn_id, status);
}

static void mtu_changed_cbk(uint16_t conn_id, uint16_t mtu)
{
	DEBUG_BLUETOOTH_SERVER("---> GATT SERVER : on_mtu_changed, conn_id: %d, mtu: %d\n", conn_id, mtu);
	ATT_MTU = mtu;
}

static gatt_server_callback_t s_gatt_cbks = {
	server_registered_cbk,
	client_connected_cbk,
	service_added_cbk,
	included_service_added_cbk,
	characteristic_added_cbk,
	descriptor_added_cbk,
	service_started_cbk,
	response_send_completed_cbk,
	attribute_read_cbk,
	attribute_write_cbk,
	notification_sent_cbk,
	mtu_changed_cbk,
};

//FIXME jsut a test service for test
static void create_gatt_srvc()
{
	memset(&s_gatt_srvc, 0, sizeof(s_gatt_srvc));

	s_gatt_srvc.srvcType = SERVICE_TYPE_PRIMARY;//主服务
	memcpy(&(s_gatt_srvc.srvcUuid), &DEFAULT_SRVC_UUID, sizeof(bt_uuid_t));

	//init chara
	GATT_Chara_t gatt_chara_1;
	memcpy(&(gatt_chara_1.charUuid), &CHARA_UUID_1, sizeof(bt_uuid_t));//特征uuid
	gatt_chara_1.charProp =PROPERTY_INDICATE + PROPERTY_READ + PROPERTY_WRITE ;//特征属性
	gatt_chara_1.charPerm = PERMISSION_READ + PERMISSION_WRITE;//权限
	gatt_chara_1.valueLen = 5;
	uint8_t value_1[5] = {0x41, 0x30, 0x31, 0x32, 0x42};
	memset(gatt_chara_1.value, 0, MAX_VALUE_LEN);
	memcpy(gatt_chara_1.value, value_1, 5);
#if 0
	GATT_Chara_t gatt_chara_2;
	memcpy(&(gatt_chara_2.charUuid), &CHARA_UUID_2, sizeof(bt_uuid_t));
	gatt_chara_2.charProp = PROPERTY_READ + PROPERTY_WRITE + PROPERTY_NOTIFY;
	gatt_chara_2.charPerm = PERMISSION_READ + PERMISSION_WRITE;
	gatt_chara_2.valueLen = 4;
	uint8_t value_2[5] = {0x41, 0x33, 0x34, 0x35};
	memset(gatt_chara_2.value, 0, MAX_VALUE_LEN);
	memcpy(gatt_chara_2.value, value_2, 5);
#endif

	//init desc
	GATT_Desc_t gatt_desc_1;
	gatt_desc_1.descrPerm = PERMISSION_READ + PERMISSION_WRITE;
	memcpy(&(gatt_desc_1.descrUuid), &DESC_UUID_1, sizeof(bt_uuid_t));
#if 0
	GATT_Desc_t gatt_desc_2;
	gatt_desc_2.descrPerm = PERMISSION_READ + PERMISSION_WRITE;
	memcpy(&(gatt_desc_2.descrUuid), &DESC_UUID_2, sizeof(bt_uuid_t));
#endif
	//init include srvc, no include_srvc
	s_gatt_srvc.include_srvc_num = 0;

	//construct service
	gatt_chara_1.desc_num = 1;
	memcpy(gatt_chara_1.gatt_desc_array, &gatt_desc_1, sizeof(GATT_Desc_t));
#if 0
	gatt_chara_2.desc_num = 1;
	memcpy(gatt_chara_2.gatt_desc_array, &gatt_desc_1, sizeof(GATT_Desc_t));
#endif
	s_gatt_srvc.chara_num = 1;
	memcpy(s_gatt_srvc.gatt_chara_array, &gatt_chara_1, sizeof(GATT_Chara_t));

#if 0
	memcpy(s_gatt_srvc.gatt_chara_array + 1, &gatt_chara_2, sizeof(GATT_Chara_t));
#endif
}

static uint16_t get_chara_value_len_from_gatt_service(GATT_Service_t gatt_srvc, bt_uuid_t char_uuid)
{
	int i = 0;
	char out[16];

	for (i; i < gatt_srvc.chara_num; i++) {
		if (memcmp(char_uuid.uu, gatt_srvc.gatt_chara_array[i].charUuid.uu, 16) == 0) {
			return gatt_srvc.gatt_chara_array[i].valueLen;
		}
	}
	return 0;
}

static uint8_t *get_chara_value_from_gatt_service(GATT_Service_t *gatt_srvc, bt_uuid_t char_uuid)
{
	int i = 0;
	char out[16];

	for (i; i < gatt_srvc->chara_num; i++) {
		if (memcmp(char_uuid.uu, gatt_srvc->gatt_chara_array[i].charUuid.uu, 16) == 0) {
			//FIXME because gatt_srvc is a static param , we can return like this.
			return gatt_srvc->gatt_chara_array[i].value;
		}
	}
	return NULL;
}

void continue_service_declaration(GATT_Declaration_t * declar, int status, int srvcHandle)
{
	int finished = 0;
	GATT_Declaration_t *cur_declar = NULL;
	if (status == 0) {
		cur_declar = get_next_declaration(declar);
		s_cur_declar = cur_declar;
	}

	if (cur_declar) {
		DEBUG_BLUETOOTH_SERVER("cur_declar->type: %d\n ", cur_declar->type);
		switch (cur_declar->type) {
			case TYPE_SERVICE: {
					goc_gatt_server_add_service(m_gatt_server.server_if, cur_declar->srvcType, 0, cur_declar->uuid, 0);
				}
				break;
			case TYPE_CHARACTERISTIC:{
					goc_gatt_server_add_characteristic(m_gatt_server.server_if, srvcHandle, cur_declar->uuid, cur_declar->prop, cur_declar->perm);
				}
				break;
			case TYPE_DESCRIPTOR:{
					goc_gatt_server_add_descriptor(m_gatt_server.server_if, srvcHandle, cur_declar->uuid, cur_declar->perm);
				}
				break;
			case TYPE_INCLUDED_SERVICE:
				//XXXXXX
				{
					//If you want to add include service, the include service must be added before this action!!!!
					uint16_t include_srvc_handle = server_info_get_include_srvc_handle(cur_declar->uuid);
					if (include_srvc_handle != 0) {
						goc_gatt_server_add_included_service(m_gatt_server.server_if, srvcHandle, include_srvc_handle);
					} else {
						printf("TODO ??? \n");
						finished = true;
					}
				}
				break;
		}
	} else {
		printfServerInfo(get_server_info());
		//declara is complete
		destroy_gatt_declaration();
		// start service
		goc_gatt_server_start_service(m_gatt_server.server_if, srvcHandle, 3/*FIXME*/);
		finished = 1;
	}

	if (finished) {
		//TODO compara to android ??? why do this
		DEBUG_BLUETOOTH_SERVER("finished == true \n");

	}
}

//service must be added one by one, this method just add a default service
void add_gatt_server_service()
{

	create_gatt_srvc();//创建gatt服务特征表

	GATT_Declaration_t *s_cur_declar = create_gatt_declaration(s_gatt_srvc);//添加声明

	continue_service_declaration(NULL, 0 , 0);//开始添加服务
}


static void notify_chara_change()
{
	//FIXME just notify a specific chara for test
	uint16_t handle = server_info_get_chara_handle_by_uuid(DEFAULT_SRVC_UUID, CHARA_UUID_1);
	uint8_t data_value[5] = {0x41, 0x42, 0x43, 0x44, 0x45};
	goc_gatt_server_send_notification(m_gatt_server.server_if, handle, m_gatt_server.conn_id, 5, data_value);
	//FIXME update chara value. Do here is appropriately ?
	server_info_update_characteristic_value(server_info_get_srvc_handle_by_uuid(DEFAULT_SRVC_UUID),
	                                        server_info_get_chara_handle_by_uuid(DEFAULT_SRVC_UUID, CHARA_UUID_1),
	                                        5,
	                                        data_value);
}

static void indicate_chara_change()
{
	//FIXME just notify a specific chara for test
	uint16_t handle = server_info_get_chara_handle_by_uuid(DEFAULT_SRVC_UUID, CHARA_UUID_1);
	uint8_t data_value[5] = {0x41, 0x42, 0x43, 0x44, 0x45};
	goc_gatt_server_send_indication(m_gatt_server.server_if, handle, m_gatt_server.conn_id,0, 5, data_value);
	//FIXME update chara value. Do here is appropriately ?
	server_info_update_characteristic_value(server_info_get_srvc_handle_by_uuid(DEFAULT_SRVC_UUID),
	                                        server_info_get_chara_handle_by_uuid(DEFAULT_SRVC_UUID, CHARA_UUID_1),
	                                        5,
	                                        data_value);
}




gatt_server_callback_t * hlp_gatt_server_get_cbks()
{
	return &s_gatt_cbks;
}

enum {
	GATT_SERVER_BACK,
	GATT_SERVER_REG,
	GATT_SERVER_UNREG,
	GATT_SERVER_ADD_SRVC,
	GATT_SERVER_CHANGE_NOTIFY,
	GATT_SERVER_CHANGE_INDICATE,
};

int enter_gatt_server_command_line()
{
	int cmd = 0;
	char buffer[16];

	printf("\n");
	printf("%d.	Go to uplater menu.\n", GATT_SERVER_BACK);
	printf("%d.	Register APP\n", GATT_SERVER_REG);
	printf("%d.	UnRegitser APP\n", GATT_SERVER_UNREG);
	printf("%d.	Add Service\n", GATT_SERVER_ADD_SRVC);
	printf("%d.	Notify Chara Change\n", GATT_SERVER_CHANGE_NOTIFY);
	printf("%d.	Indicate Chara Change\n", GATT_SERVER_CHANGE_INDICATE);
	printf("\n");
	printf("Please choose a command(enter the index of command, 'q' to quit.):\n");
	printf(">");
	fgets(buffer, 16, stdin);

	if (buffer[0] == 'q' || buffer[0] == 'Q') {
		quit_client();
	} else if (buffer[0] == 'c' || buffer[0] == 'C') {
		return 0;
	} else if (buffer[0] == '\n') {
		return ENTER;
	}

	cmd = atoi(buffer);

	return cmd;
}

int execute_gatt_server_command(int cmd)
{
	switch (cmd) {
		case GATT_SERVER_REG:
			goc_gatt_server_register_app(back_s_uuid);
			break;
		case GATT_SERVER_UNREG://when clsoe gatt server  to unregister APP
			goc_gatt_server_unregister_app(m_gatt_server.server_if);
			break;
		case GATT_SERVER_ADD_SRVC:/*add service*/
			add_gatt_server_service();
			break;
		case GATT_SERVER_CHANGE_NOTIFY:/*Notify*/
			notify_chara_change();
			break;
		case GATT_SERVER_CHANGE_INDICATE:/*Notify*/
			indicate_chara_change();
			break;
		case GATT_SERVER_BACK:
			return BACK;
		default:
			break;
	}
	return GATT_SERVER_MENU;
}



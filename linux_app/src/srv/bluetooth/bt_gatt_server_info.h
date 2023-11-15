#ifndef _GATT_SERVER_INFO_H_
#define _GATT_SERVER_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_gatt_declaration.h"


#define UNKNOWN_HANDLE 0xFFFF
#define UNKNOWN_UUID {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

typedef struct tag_gatt_server_info {
	uint8_t server_if;
	uint16_t conn_id;
	int srvc_num;
	GATT_Service_t gatt_srvc_array[MAX_GATT_SERVICE_NUM];
} server_info_t;

server_info_t *get_server_info();
void server_info_update_server_if(uint8_t server_if);
void server_info_update_conn_id(uint16_t conn_id);
void create_server_info();
void destroy_server_info();
GATT_Service_t *server_info_get_gatt_srvc_by_handle(uint16_t srvcInstId);
void server_info_add_service(uint8_t svc_type, uint16_t svc_inst_id, bt_uuid_t srvc_uuid, uint16_t handle);
void server_info_add_characteristic(bt_uuid_t char_uuid, uint16_t srvc_handle, uint16_t char_handle);
GATT_Chara_t * server_info_get_lastest_chara_by_handle(uint16_t srvc_handle);
void server_info_add_descriptor(bt_uuid_t descr_uuid, uint16_t srvc_handle, uint16_t descr_handle);
uint16_t server_info_get_include_srvc_handle(bt_uuid_t uuid);
GATT_Chara_t * server_info_get_gatt_chara_by_handle(uint16_t srvc_handle, uint16_t chara_handle);
void server_info_update_characteristic_value(uint16_t srvc_handle, uint16_t char_handle, uint16_t valueLen, uint8_t *data_value);
uint16_t server_info_get_srvc_handle_by_uuid(bt_uuid_t srvc_uuid);
uint16_t server_info_get_chara_handle_by_uuid(bt_uuid_t  srvc_uuid, bt_uuid_t uuid);
uint16_t server_info_get_desc_handle_by_uuid(bt_uuid_t srvc_uuid, bt_uuid_t chara_uuid, bt_uuid_t desc_uuid);

#ifdef __cplusplus
}
#endif

#endif 


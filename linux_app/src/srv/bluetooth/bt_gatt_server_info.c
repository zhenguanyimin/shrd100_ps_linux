#include <string.h>

#ifdef __cplusplus
	extern "C" {
#endif

#include "bt_utils.h"
#include "bt_gatt_server_info.h"
#ifdef __cplusplus
}
#endif

/************************************

the saved uuid is like {fb349b5f-8000-0080-0010-000008180000}

*****************************************/

static server_info_t server_info;

void server_info_add_included_service(uint16_t srvc_handle, uint16_t incl_srvc_handle);

void server_info_update_server_if(uint8_t server_if)
{
	memset(&server_info, 0, sizeof(server_info));
	server_info.server_if = server_if;
}

void server_info_update_conn_id(uint16_t conn_id)
{
	server_info.conn_id = conn_id;
}

void create_server_info()
{
	//TODO
}

void destroy_server_info()
{
	memset(&server_info, 0, sizeof(server_info));
}

server_info_t *get_server_info()
{
	return &server_info;
}

void server_info_add_service(uint8_t svc_type, uint16_t svc_inst_id, bt_uuid_t srvc_uuid, uint16_t handle)
{
	GATT_Service_t *gatt_srvc_array = server_info.gatt_srvc_array;

	gatt_srvc_array[server_info.srvc_num].srvcType = svc_type;
	gatt_srvc_array[server_info.srvc_num].srvcInstId = handle;
	memcpy(&gatt_srvc_array[server_info.srvc_num].srvcUuid, &srvc_uuid, sizeof(bt_uuid_t));

	server_info.srvc_num++;
}

void server_info_add_included_service(uint16_t srvc_handle, uint16_t incl_srvc_handle)
{
	GATT_Service_t *gatt_srvc = server_info_get_gatt_srvc_by_handle(srvc_handle);
	GATT_Include_Srvc_t *gatt_include_srvc_array = gatt_srvc->gatt_include_srvc_array;

	gatt_include_srvc_array[gatt_srvc->include_srvc_num].srvcInstId = srvc_handle;
	gatt_include_srvc_array[gatt_srvc->include_srvc_num].inclSrvcInstId = incl_srvc_handle;

	gatt_srvc->include_srvc_num++;
}

void server_info_add_characteristic(bt_uuid_t char_uuid, uint16_t srvc_handle, uint16_t char_handle)
{
	GATT_Service_t *gatt_srvc = server_info_get_gatt_srvc_by_handle(srvc_handle);
	if (gatt_srvc) {
		gatt_srvc->gatt_chara_array[gatt_srvc->chara_num].srvcInstId = srvc_handle;
		gatt_srvc->gatt_chara_array[gatt_srvc->chara_num].charInstId = char_handle;
		memcpy(&(gatt_srvc->gatt_chara_array[gatt_srvc->chara_num].charUuid), &char_uuid, sizeof(bt_uuid_t));

		gatt_srvc->chara_num++;
	}
}

void server_info_add_descriptor(bt_uuid_t descr_uuid, uint16_t srvc_handle, uint16_t descr_handle)
{
	GATT_Chara_t *gatt_chara = server_info_get_lastest_chara_by_handle(srvc_handle);

	gatt_chara->gatt_desc_array[gatt_chara->desc_num].srvcInstId = srvc_handle;
	gatt_chara->gatt_desc_array[gatt_chara->desc_num].descrInstId = descr_handle;
	memcpy(&(gatt_chara->gatt_desc_array[gatt_chara->desc_num].descrUuid), &descr_uuid, sizeof(bt_uuid_t));

	gatt_chara->desc_num++;
}

GATT_Service_t *server_info_get_gatt_srvc_by_handle(uint16_t srvcInstId)
{
	int i = 0;
	for (i; i < server_info.srvc_num; i++) {
		if (srvcInstId == server_info.gatt_srvc_array[i].srvcInstId) {
			return &(server_info.gatt_srvc_array[i]);
		}
	}
	return NULL;
}

GATT_Chara_t * server_info_get_lastest_chara_by_handle(uint16_t srvc_handle)
{
	GATT_Service_t *gatt_srvc = server_info_get_gatt_srvc_by_handle(srvc_handle);
	GATT_Chara_t *lastest_chara = &(gatt_srvc->gatt_chara_array[gatt_srvc->chara_num - 1]);

	return lastest_chara;
}

GATT_Chara_t * server_info_get_gatt_chara_by_handle(uint16_t srvc_handle, uint16_t chara_handle)
{
	if (srvc_handle == UNKNOWN_HANDLE) {//srvc_handle unknown
		int i = 0;
		for (i; i < server_info.srvc_num; i++) {
			GATT_Service_t *gatt_srvc = server_info.gatt_srvc_array + i;
			int j = 0;
			for (j; j < gatt_srvc->chara_num; j++) {
				if(chara_handle == gatt_srvc->gatt_chara_array[j].charInstId) {
					return &(gatt_srvc->gatt_chara_array[j]);
				}
			}
		}
	} else {
		GATT_Service_t *gatt_srvc = server_info_get_gatt_srvc_by_handle(srvc_handle);

		int i = 0;
		for (i; i < gatt_srvc->chara_num; i++) {
			if (gatt_srvc->gatt_chara_array[i].charInstId == chara_handle) {
				return &(gatt_srvc->gatt_chara_array[i]);
			}
		}
	}
	return NULL;
}

uint16_t server_info_get_include_srvc_handle(bt_uuid_t uuid)
{
	int i = 0;
	for (i; i < server_info.srvc_num; i++) {
		GATT_Service_t gatt_srvc = server_info.gatt_srvc_array[i];
		if (memcmp(uuid.uu, gatt_srvc.srvcUuid.uu, 16) == 0) {
			return gatt_srvc.srvcInstId;
		}
	}

	return 0;
}

void server_info_update_characteristic_value(uint16_t srvc_handle, uint16_t char_handle, uint16_t valueLen, uint8_t *data_value)
{
	GATT_Chara_t * gatt_chara = server_info_get_gatt_chara_by_handle(srvc_handle, char_handle);

	if (gatt_chara == NULL) {
		return;
	}
	//printf("char_handle:%X,charInstId:%X",char_handle,gatt_chara->charInstId);
	gatt_chara->valueLen = valueLen;
	memset(gatt_chara->value, 0, MAX_VALUE_LEN);
	memcpy(gatt_chara->value, data_value, valueLen);
}

int get_attr_handle_type(uint16_t attr_handle)
{
	if (server_info_get_gatt_chara_by_handle(UNKNOWN_HANDLE, attr_handle) == NULL) {
		return TYPE_DESCRIPTOR;
	} else {
		return TYPE_CHARACTERISTIC;
	}
}

uint16_t server_info_get_srvc_handle_by_uuid(bt_uuid_t srvc_uuid)
{

	int i = 0;
	for (i; i < server_info.srvc_num; i++) {
		GATT_Service_t *gatt_srvc = server_info.gatt_srvc_array + i;
		if (memcmp(srvc_uuid.uu, gatt_srvc->srvcUuid.uu, 16) == 0) {
			return gatt_srvc->srvcInstId;
		}
	}

	return UNKNOWN_HANDLE;
}

uint16_t server_info_get_chara_handle_by_uuid(bt_uuid_t srvc_uuid, bt_uuid_t char_uuid)
{

	uint16_t gatt_srvc_handle = server_info_get_srvc_handle_by_uuid(srvc_uuid);
	GATT_Service_t *gatt_srvc = server_info_get_gatt_srvc_by_handle(gatt_srvc_handle);
	int i = 0;
	for (i; i < gatt_srvc->chara_num; i++) {
		GATT_Chara_t *gatt_chara = gatt_srvc->gatt_chara_array + i;
		if (memcmp(char_uuid.uu, gatt_chara->charUuid.uu, 16) == 0) {
			return gatt_chara->charInstId;
		}
	}

	return UNKNOWN_HANDLE;
}

uint16_t server_info_get_desc_handle_by_uuid(bt_uuid_t srvc_uuid, bt_uuid_t chara_uuid, bt_uuid_t desc_uuid)
{
	uint8_t desc_uu[16];
	convert_uuid_order(desc_uu, desc_uuid.uu, 16);

	uint16_t gatt_srvc_handle = server_info_get_srvc_handle_by_uuid(srvc_uuid);
	uint16_t gatt_chara_handle = server_info_get_chara_handle_by_uuid(srvc_uuid, chara_uuid);
	GATT_Chara_t *gatt_chara = server_info_get_gatt_chara_by_handle(gatt_srvc_handle, gatt_chara_handle);

	int i = 0;
	for (i; i < gatt_chara->desc_num; i++) {
		GATT_Desc_t *gatt_desc = gatt_chara->gatt_desc_array + i;
		if (memcmp(desc_uu, gatt_desc->descrUuid.uu, 16) == 0) {
			return gatt_desc->descrInstId;
		}
	}

	return UNKNOWN_HANDLE;
}

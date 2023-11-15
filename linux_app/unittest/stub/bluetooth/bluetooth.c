/** 
 * for ut stub
 *
 * author: jiawei@autel.com
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../src/srv/bluetooth/bluetooth.h"
#include "../../../src/srv/bluetooth/bt_gatt_server.h"
#include "../../../src/srv/bluetooth/bt_gatt_server_info.h"
#include "../../../src/srv/bluetooth/goc_gatt_server.h"


#ifdef __cplusplus
}
#endif

static bt_uuid_t CHARA_UUID_test = {0x00, 0x00, 0x18, 0x08, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};


uint8_t gatt_get_server_if(void)
{
    return 0;
}

bt_uuid_t gatt_get_uuid(void)
{
    return CHARA_UUID_test;
}

bt_uuid_t gatt_get_server_uuid(void)
{
    return CHARA_UUID_test;
}

uint16_t server_info_get_chara_handle_by_uuid(bt_uuid_t  srvc_uuid, bt_uuid_t uuid)
{
    return 0;
}

bt_status_t goc_gatt_server_send_response(uint8_t server_if, uint16_t conn_id, uint16_t trans_id, uint8_t status, uint16_t handle, uint16_t offset, uint8_t auth_req, uint16_t valLen, uint8_t * val)
{
    return BT_STATUS_SUCCESS;
}

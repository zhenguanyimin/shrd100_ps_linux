#ifndef _GATT_SERVER_TEST_H_
#define _GATT_SERVER_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "goc_gatt_server.h"
#include "bluetooth.h"

gatt_server_callback_t * hlp_gatt_server_get_cbks();
int enter_gatt_server_commmand_line();
int execute_gatt_server_command(int cmd);
void add_gatt_server_service();
uint8_t gatt_get_server_if(void);
bt_uuid_t gatt_get_uuid(void);
bt_uuid_t gatt_get_server_uuid(void);


#ifdef __cplusplus
}
#endif

#endif


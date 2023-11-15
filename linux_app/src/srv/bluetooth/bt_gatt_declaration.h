#ifndef _GATT_DECLARATION_H_
#define _GATT_DECLARATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gatt_structs.h"

#define TYPE_UNDEFINED			0
#define TYPE_SERVICE			1
#define TYPE_CHARACTERISTIC		2
#define TYPE_DESCRIPTOR			3
#define TYPE_INCLUDED_SERVICE	4

GATT_Declaration_t *create_gatt_declaration(GATT_Service_t gatt_srvc);
void destroy_gatt_declaration();
GATT_Declaration_t *get_next_declaration(GATT_Declaration_t * cur_declar);

#ifdef __cplusplus
}
#endif

#endif


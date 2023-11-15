#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
	extern "C" {
#endif

#include "bt_gatt_declaration.h"
#ifdef __cplusplus
}
#endif

static GATT_Declaration_t *s_gatt_declar = NULL;
static GATT_Declaration_t *s_gatt_declar_end = NULL;

GATT_Declaration_t *create_gatt_declaration(GATT_Service_t gatt_srvc)
{
	int declar_num = 1 + gatt_srvc.chara_num + gatt_srvc.include_srvc_num;
	int i = 0;
	for (i; i < gatt_srvc.chara_num; i++) {
		GATT_Chara_t gatt_chara = gatt_srvc.gatt_chara_array[i];
		declar_num += gatt_chara.desc_num;
	}

	s_gatt_declar = (GATT_Declaration_t *)malloc(declar_num * sizeof(GATT_Declaration_t));
	memset(s_gatt_declar, 0, declar_num * sizeof(GATT_Declaration_t));
	s_gatt_declar_end = s_gatt_declar + declar_num - 1;

	GATT_Declaration_t *declar_pos = s_gatt_declar;
	//add srvc to declar
	declar_pos->type = TYPE_SERVICE;
	declar_pos->srvcType  = gatt_srvc.srvcType;
	memcpy(&(declar_pos->uuid), &gatt_srvc.srvcUuid, sizeof(bt_uuid_t));
	declar_pos += 1;

	i = 0;
	//add include srvc to declar
	int include_srvc_num = gatt_srvc.include_srvc_num;
	for (i; i < include_srvc_num; i++) {
		declar_pos->type = TYPE_INCLUDED_SERVICE;
		declar_pos->srvcType = gatt_srvc.gatt_include_srvc_array[i].srvcType;
		memcpy(&(declar_pos->uuid), &(gatt_srvc.gatt_include_srvc_array[i].srvcUuid), sizeof(bt_uuid_t));
		declar_pos += 1;
	}

	//add chara to declar
	i = 0;
	int chara_num = gatt_srvc.chara_num;
	GATT_Chara_t *gatt_chara_array = gatt_srvc.gatt_chara_array;
	for (i; i < chara_num; i++) {
		declar_pos->type = TYPE_CHARACTERISTIC;
		memcpy(&(declar_pos->uuid), &(gatt_chara_array[i].charUuid), sizeof(bt_uuid_t));
		declar_pos->perm = gatt_chara_array[i].charPerm;
		declar_pos->prop = gatt_chara_array[i].charProp;
		declar_pos += 1;

		//add desc to declar
		int j = 0;
		int desc_num = gatt_chara_array[i].desc_num;
		GATT_Desc_t * gatt_desc_array = gatt_chara_array[i].gatt_desc_array;
		for (j ; j < desc_num; j++) {
			declar_pos->type = TYPE_DESCRIPTOR;
			memcpy(&(declar_pos->uuid), &(gatt_desc_array[j].descrUuid), sizeof(bt_uuid_t));
			declar_pos->perm = gatt_desc_array[j].descrPerm;
			declar_pos += 1;
		}
	}

	return s_gatt_declar;

}

GATT_Declaration_t *get_next_declaration(GATT_Declaration_t * cur_declar)
{
	if (cur_declar == NULL) {
		return s_gatt_declar;//first declar
	}

	if (cur_declar == s_gatt_declar_end) {
		printf("Got The End of The Declaration !!! \n");
		return NULL;
	}

	return cur_declar + 1;
}

void destroy_gatt_declaration()
{
	if (s_gatt_declar) {
		free(s_gatt_declar);
	}
	s_gatt_declar = NULL;
	s_gatt_declar_end = NULL;
}


#ifndef __GATT_STRUCTS__H_
#define __GATT_STRUCTS__H_


#ifdef __cplusplus
	extern "C" {
#endif

#include "bluetooth.h"


//FIXME best use list
#define MAX_GATT_SERVICE_NUM		20
#define MAX_GATT_CHARA_NUM		15
#define MAX_GATT_DESC_NUM		10
#define MAX_GATT_INCLUDE_NUM		10

#define MAX_VALUE_LEN		100

#define SERVICE_TYPE_PRIMARY		0
#define SERVICE_TYPE_SECONDARY		1

#define PROPERTY_READ		0x02
#define PROPERTY_WRITE		0x08
#define PROPERTY_NOTIFY		0x10
#define PROPERTY_INDICATE	0x20

#define PERMISSION_READ		0x01
#define PERMISSION_WRITE	0x10

/** A GATT operation completed successfully */
#define GATT_SUCCESS		 0

/** GATT read operation is not permitted */
#define GATT_READ_NOT_PERMITTED 0x2

/** GATT write operation is not permitted */
#define GATT_WRITE_NOT_PERMITTED 0x3

/** Insufficient authentication for a given operation */
#define GATT_INSUFFICIENT_AUTHENTICATION 0x5

/** The given request is not supported */
#define GATT_REQUEST_NOT_SUPPORTED  0x6

/** Insufficient encryption for a given operation */
#define GATT_INSUFFICIENT_ENCRYPTION  0xf

/** A read or write operation was requested with an invalid offset */
#define GATT_INVALID_OFFSET  0x7

/** A write operation exceeds the maximum length of the attribute */
#define GATT_INVALID_ATTRIBUTE_LENGTH  0xd

/** A remote device connection is congested. */
#define GATT_CONNECTION_CONGESTED  0x8f

/** A GATT operation failed, errors other than the above */
#define GATT_FAILURE  0x101



//Desc
typedef struct tag_gatt_desc {
	uint8_t srvcType;
	uint16_t srvcInstId;
	bt_uuid_t srvcUuid;
	uint16_t charInstId;
	bt_uuid_t charUuid;
	uint16_t descrInstId;
	uint16_t descrPerm;/*permission*/
	bt_uuid_t descrUuid;
	uint16_t valueLen;
	uint8_t value[MAX_VALUE_LEN];
} GATT_Desc_t;

//Chara
typedef struct tag_gatt_chara {
	uint8_t srvcType;
	uint16_t srvcInstId;
	bt_uuid_t srvcUuid;
	uint16_t charInstId;
	bt_uuid_t charUuid;
	uint8_t charProp; /*property*/
	uint16_t charPerm;/*permission*/
	uint16_t valueLen;
	uint8_t value[MAX_VALUE_LEN];
	int desc_num;
	GATT_Desc_t gatt_desc_array[MAX_GATT_DESC_NUM];
} GATT_Chara_t;

//Include service
typedef struct tag_gatt_include_srvc {
	uint8_t srvcType;
	uint16_t srvcInstId;
	bt_uuid_t srvcUuid;
	uint8_t inclSrvcType;
	uint16_t inclSrvcInstId;
	bt_uuid_t inclSrvcUuid;
} GATT_Include_Srvc_t;

//Service
typedef struct tag_gatt_service {
	uint8_t srvcType; //服务类型
	uint16_t srvcInstId;/*server means handle*/
	bt_uuid_t srvcUuid;
	int chara_num;
	GATT_Chara_t gatt_chara_array[MAX_GATT_CHARA_NUM];
	int include_srvc_num;
	GATT_Include_Srvc_t gatt_include_srvc_array[MAX_GATT_INCLUDE_NUM];

} GATT_Service_t;

//Service Declaration
typedef struct tag_gatt_declaration {
	uint8_t type;
	/*uint16_t instId;*/
	uint8_t srvcType;
	bt_uuid_t uuid;
	uint16_t perm;
	uint16_t prop;
} GATT_Declaration_t;

#ifdef __cplusplus
}
#endif

#endif


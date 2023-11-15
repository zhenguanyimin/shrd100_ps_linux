
#include "c2_alink.h"
#include "analysis/alink_order.h"
#include "analysis/alink_recv.h"
#include "analysis/alink_socket.h"

#include "upload/alink_package.h"
#include "upload/alink_upload.h"

// #include "command/control/alink_control.h"
// #include "command/system/alink_system.h"
// #include "command/track/alink_track.h"
// #include "command/transfer/alink_transfer.h"
// #include "command/upgrade/alink_upgrade.h"
// #include "command/record/alink_record.h"


static uint8_t abySendBuffer[ALINK_SEND_BUFFER_SIZE] = { 0 };
static alink_order_t sOrder	= { 0 };

static uint8_t abyUploadBuffer[ALINK_UPLOAD_BUFFER_SIZE] = { 0 };
static alink_upload_t sUpload = { 0 };

int32_t alink_init( void )
{
	int32_t eRet = 0;

	alink_order_init( &sOrder, abySendBuffer, sizeof(abySendBuffer) );

	alink_upload_init( &sUpload, ALINK_DEV_ID_RADAR, abyUploadBuffer, sizeof(abyUploadBuffer) );

	// alink_control_init();
	// alink_system_init();
	// alink_track_init();
	// alink_transfer_init();
	alink_upgrade_init();
	// alink_record_init();

	return eRet;
}

int32_t alink_connect_port( alink_socket_t *psSocket, alink_port_t *psPort )
{
	int32_t eRet = 0;

	eRet = alink_socket_init( psSocket, &sOrder );
//	eRet = alink_socket_connect( psSocket, psPort );

	return eRet;
}

#if 0
//****please using register_alink_cmd instead
int32_t alink_register_cmd( alink_cmd_list_t *psCmdList, uint8_t byOrder, uint16_t wPayloadLen,
								void (*pv_cmd_task)(alink_payload_t *psPayload, alink_resp_t *psResp) )
{
	int32_t eRet = 0;

	psCmdList->byCmd		= byOrder;
	psCmdList->wPayloadLen	= wPayloadLen;
	psCmdList->pv_cmd_task	= pv_cmd_task;
	psCmdList->psNext		= NULL;
	eRet = alink_order_register_cmd( &sOrder, psCmdList );

	return eRet;
}
#endif

int32_t alink_upload_package( alink_package_list_t *psPackage, void *psImport )
{
	return alink_upload_send_package( &sUpload, psPackage, psImport );
}

int32_t alink_register_package( alink_package_list_t *psPackageList, uint8_t byMsgId, uint8_t byAns, 
									uint16_t (*pw_package_task)(uint8_t* pbyBuffer, uint16_t wSize, void *psImport) )
{
	int32_t eRet = 0;

	eRet = alink_package_init( psPackageList, byMsgId, byAns, pw_package_task );

	eRet = alink_upload_register_package( &sUpload, psPackageList );

	return eRet;
}

int32_t alink_connect_send( alink_send_list_t *psSendList, uint8_t byMsgId, uint8_t byRemoteId, void *handle,
								int32_t (*pe_send)(uint8_t *pbyData, uint32_t uLen) )
{
	psSendList->byRemoteId	= byRemoteId;
	psSendList->handle		= handle;
	psSendList->pe_send		= pe_send;

	return alink_upload_connect_send( &sUpload, byMsgId, psSendList );
}

int32_t alink_disconnect_send( alink_send_list_t *psSendList, uint8_t byMsgId )
{
	return alink_upload_disconnect_send( &sUpload, byMsgId, psSendList );
}

int32_t alink_set_upload_dev_id( uint8_t byDevId )
{
	if( ALINK_DEV_ID_RADAR == (0x0F & byDevId) )
	{
		sUpload.byDevId = byDevId;
		return 0;
	}
	else
	{
		return -1;
	}
}


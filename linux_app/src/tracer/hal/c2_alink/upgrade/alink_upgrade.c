
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <assert.h>

#include <pthread.h>

#include "eap_sys_cmds.h"

#ifdef __cplusplus 
extern "C" { 
#endif

#include "alink_upgrade.h"
#include "app_version.h"
#include "firmware.h"

#if 0
#define DEBUG_UPGRADE_PRINTF(arg...) printf(arg)
#else
#define DEBUG_UPGRADE_PRINTF(arg...) 
#endif

#define CFG_FIRMWARE_SAVE_DELAY_TO_REBOOT_STAGE

//#define DEBUG_UPGRADE_NO_REBOOT
#define FLAG_DEBUG_UPGRADE_NO_REBOOT "/run/media/mmcblk0p1/flag_debug_upgrade_no_reboot"


#define FLAG_FILE_SWITCH_VER_OK  "/tmp/PAREPARE_TO_SWITCH_VER_OK"
#define PREPARE_TO_SWITCH_VER_SH  "/usr/bin/prepare_to_switch_ver.sh"


enum 
{
	UPDATE_STAT_NO_START = 0,
	UPDATE_STAT_REQUEST,
	UPDATE_STAT_DOWNLOAD_IMAGE,
	UPDATE_STAT_CHECK,
	UPDATE_STAT_WRITE_IMAGE,
	UPDATE_STAT_WAIT_REBOOT,
};

//huaguoqing modify,2023.09.05   We want to add a ack_offset field for psDownloadImageAck.
//so we extend the hightest bit of status as a extend flag. Through check the highest bits of status, we know the new psDownloadImageAck have ack_offset field.
//0-6 bits is the real ret status.

#define UPDATE_RET_OK           (0)
#define UPDATE_RET_ERROR        (1)
#define UPDATE_RET_COMPLETE     (2)
#define UPDATE_RET_STATUS_ERROR (10) //the return value

#define UPDATE_RET_STATUS_MASK  (0x7f)
#define UPDATE_RET_STATUS_EXTEND_MASK (0x80)

static int update_state = UPDATE_STAT_NO_START;
static int firmware_save_ok = 0;
static uint32_t run_mode = 1; //1: normal mode, app runing. 0:boot mode

extern int AlinkDisableReportAlinkInfoSet(int isDisable);

static uint16_t upgrade_get_version	(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;

	upgrade_version_ack_t *psVersionAck = (upgrade_version_ack_t *)rsp->buffer;
	DEBUG_UPGRADE_PRINTF("%s[%d]in\n", __FUNCTION__, __LINE__);
	psVersionAck->uRunVersion = run_mode;

	strncpy( psVersionAck->strBootVersion, "N/A", sizeof(psVersionAck->strBootVersion) );
//	strncpy( psVersionAck->strAppVersion, get_embed_software_ps_version_string(), (size_t)sizeof(psVersionAck->strAppVersion) );
	strncpy( psVersionAck->strAppVersion, EAP_EMBED_SOFTWARE_PS_VERSION_STR, (size_t)sizeof(psVersionAck->strAppVersion) );

	strncpy( psVersionAck->strHwVersion, "N/A", sizeof(psVersionAck->strHwVersion) );
//	strncpy( psVersionAck->strProtocolVersion, get_embed_alink_version_string(), sizeof(psVersionAck->strProtocolVersion) );
	strncpy( psVersionAck->strProtocolVersion, "N/A", sizeof(psVersionAck->strProtocolVersion) );

	printf("%s[%d]in\n", __FUNCTION__, __LINE__);

	return sizeof(upgrade_version_ack_t);
}

static void multi_boot_run_golden(void)
{
	printf("reboot system....");
	sync();
	
#ifdef DEBUG_UPGRADE_NO_REBOOT
	printf("\n\n***debuging upgarde, ignore reboot cmd\n\n");
	return ;
#else
	if(access(FLAG_DEBUG_UPGRADE_NO_REBOOT, 0) == 0){
		printf("\n\n***found %s, ignore reboot cmd\n\n",FLAG_DEBUG_UPGRADE_NO_REBOOT);
		return;
	}
#endif

	reboot(RB_AUTOBOOT);
}

static uint16_t upgrade_reboot_system	(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;

	upgrade_reset_t *psReset;
	upgrade_reset_ack_t *psResetAck;

	psReset = (upgrade_reset_t*)(req->buffer);
	psResetAck = (upgrade_reset_ack_t*)(rsp->buffer);

	
	printf("%s[%d]in\n", __FUNCTION__, __LINE__);

	if( 0x55AA == psReset->resetCode )
	{
		// get system status and check upgrade flag
		switch( psReset->type )
		{
			case 0x0004:
				run_mode = 0;
				/*
				if( true == firmware_set_upgrade_flag(true) )
				{
					multi_boot_run_golden();
				}
				else
				{
					psResetAck->byStatus = 1;
					psResp->wCount = 1;
				}
				*/
				psResetAck->byStatus = 0;

				break;
			default:
				break;
		}
	}
	else
	{
		psResetAck->byStatus = 0x0F;
	}
	return sizeof(psResetAck);
}

static uint16_t upgrade_request_upgrade(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	bool ret;

	upgrade_request_upgrade_t *psRequestDownload;
	upgrade_request_upgrade_ack_t *psRequestDownloadAck;

	psRequestDownload = (upgrade_request_upgrade_t*)(req->buffer);
	psRequestDownloadAck = (upgrade_request_upgrade_ack_t*)(rsp->buffer);

	printf("%s[%d]in\n", __FUNCTION__, __LINE__);
	//assert(resp->wSize >= sizeof(upgrade_request_upgrade_ack_t));
	DisableStoreCmd(255);

	if (UPDATE_STAT_WAIT_REBOOT == update_state )
	{
		psRequestDownloadAck->byStatus = UPDATE_RET_STATUS_ERROR;
		return;
	}

	ret = firmware_recv_init(&(psRequestDownload->sFirmwareHeader));
	if (ret)
	{
		psRequestDownloadAck->byStatus = UPDATE_RET_OK;

		update_state = UPDATE_STAT_REQUEST;
		AlinkDisableReportAlinkInfoSet(1);
	}
	else
	{
		psRequestDownloadAck->byStatus = UPDATE_RET_ERROR;
	}

	printf("%s[%d]out\n", __FUNCTION__, __LINE__);	
	return sizeof(upgrade_request_upgrade_ack_t);
}

static uint16_t upgrade_download_image(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	int ret;

	upgrade_download_image_t *psDownloadImage;
	upgrade_download_image_ack_t *psDownloadImageAck;

	psDownloadImage = (upgrade_download_image_t*)(req->buffer);
	psDownloadImageAck = (upgrade_download_image_ack_t*)(rsp->buffer);

	memset(psDownloadImageAck,0,sizeof(*psDownloadImageAck));
	
	DEBUG_UPGRADE_PRINTF(">>>>>>%s[%d] imageOffset:%x,imageLength:%x ->end:0x%x\n", __FUNCTION__, __LINE__,psDownloadImage->imageOffset,psDownloadImage->imageLength,
			(psDownloadImage->imageOffset + psDownloadImage->imageLength));

	if ((update_state != UPDATE_STAT_DOWNLOAD_IMAGE) && (update_state != UPDATE_STAT_REQUEST))
	{
		psDownloadImageAck->byStatus = UPDATE_RET_STATUS_EXTEND_MASK | UPDATE_RET_STATUS_ERROR;
		printf("<<<<<<<<<<<<%s[%d]state error:%d\n", __FUNCTION__, __LINE__, update_state);
		return ;
	}

	ret = firmware_recv_data(psDownloadImage->imageOffset, psDownloadImage->imageData, psDownloadImage->imageLength); 
	if (ret >=0 )
	{
		psDownloadImageAck->byStatus = UPDATE_RET_STATUS_EXTEND_MASK | UPDATE_RET_OK;
		psDownloadImageAck->ack_offset = (uint32_t)ret;
		DEBUG_UPGRADE_PRINTF(">>>>>>%s[%d] imageOffset:0x%x,imageLength:0x%x ->end:0x%x, ack_offset:0x%x(%u)\n", __FUNCTION__, __LINE__,psDownloadImage->imageOffset,psDownloadImage->imageLength,
		(psDownloadImage->imageOffset + psDownloadImage->imageLength),psDownloadImageAck->ack_offset,psDownloadImageAck->ack_offset);

		update_state = UPDATE_STAT_DOWNLOAD_IMAGE;		
	}
	else
	{
		psDownloadImageAck->byStatus = UPDATE_RET_STATUS_EXTEND_MASK | UPDATE_RET_ERROR;

		printf("<<<<<<<<<<<----%s[%d]firmware_recv_data error\n", __FUNCTION__, __LINE__);
	}
	DEBUG_UPGRADE_PRINTF(">>>>>>%s[%d] ack:%hhx: ack_offset:0x%x(%u), imageOffset:0x%x,imageLength:0x%x ->end:0x%x\n", __FUNCTION__, __LINE__,psDownloadImageAck->byStatus,
		psDownloadImageAck->ack_offset,psDownloadImageAck->ack_offset,psDownloadImage->imageOffset,psDownloadImage->imageLength,
			(psDownloadImage->imageOffset + psDownloadImage->imageLength));

	return sizeof(upgrade_download_image_ack_t);
}

static uint16_t upgrade_check_image(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	upgrade_check_image_ack_t *psCheckImageAck;

	psCheckImageAck = (upgrade_check_image_ack_t*)(rsp->buffer);

	printf("%s[%d]in\n", __FUNCTION__, __LINE__);
	
	AlinkDisableReportAlinkInfoSet(0);

	if (firmware_check())
	{
		psCheckImageAck->byStatus = UPDATE_RET_OK;
		update_state = UPDATE_STAT_CHECK;
	}
	else
	{
		psCheckImageAck->byStatus = UPDATE_RET_ERROR;
		printf("%s[%d]check error\n", __FUNCTION__, __LINE__);
	}

	printf("%s[%d]out\n", __FUNCTION__, __LINE__);
	return sizeof(upgrade_check_image_ack_t);
}

static uint16_t upgrade_write_image(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	bool ret;

	upgrade_write_image_ack_t *psWriteImageAck;

	psWriteImageAck = (upgrade_write_image_ack_t*)(rsp->buffer);

	
	printf("%s[%d]in\n", __FUNCTION__, __LINE__);

	if (update_state < UPDATE_STAT_DOWNLOAD_IMAGE)
	{
		psWriteImageAck->byStatus = UPDATE_RET_STATUS_ERROR;
		goto out;
	}

	ret = firmware_set_upgrade_flag(false);
	if (!ret)
	{
		psWriteImageAck->byStatus = UPDATE_RET_ERROR;
		goto out;
	}

	update_state = UPDATE_STAT_WRITE_IMAGE;

#ifdef CFG_FIRMWARE_SAVE_DELAY_TO_REBOOT_STAGE
	ret  = true;
#else
	ret = firmware_save();
#endif
	if (ret)
	{
		ret = firmware_set_upgrade_flag(true);
		if (ret)
		{
			firmware_save_ok = 1;
			psWriteImageAck->byStatus = UPDATE_RET_OK;
		}
		else
		{
			psWriteImageAck->byStatus = UPDATE_RET_ERROR;
			printf("%s[%d] set upgrade flag failed!\n", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		psWriteImageAck->byStatus = UPDATE_RET_ERROR;
		printf("%s[%d] write firmware status failed!\n", __FUNCTION__, __LINE__);
	}

out:	
	printf("%s[%d]out, psWriteImageAck->byStatus = %d\n", __FUNCTION__, __LINE__, psWriteImageAck->byStatus);
	return sizeof(upgrade_write_image_ack_t);	
}

static uint16_t upgrade_write_status(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	upgrade_write_status_ack_t *psWriteStatusAck;

	printf("%s[%d]\n", __FUNCTION__, __LINE__);
		
	psWriteStatusAck = (upgrade_write_status_ack_t*)(rsp->buffer);

	if (update_state != UPDATE_STAT_WRITE_IMAGE)
	{
		psWriteStatusAck->byStatus = UPDATE_RET_STATUS_ERROR;
		psWriteStatusAck->wPermil = 0;
	}
	else 
	{
		if (firmware_save_ok)
		{
			psWriteStatusAck->byStatus = UPDATE_RET_COMPLETE;
			psWriteStatusAck->wPermil = 1000;
		}
		else
		{
			psWriteStatusAck->byStatus = UPDATE_RET_ERROR;
			psWriteStatusAck->wPermil = 0;
		}
	}
	return sizeof(upgrade_write_status_ack_t);
}

static void *auto_reboot_task(void *param)
{
#ifdef CFG_FIRMWARE_SAVE_DELAY_TO_REBOOT_STAGE
	int ret;
	ret = firmware_save();
	if(ret == true){
		system(PREPARE_TO_SWITCH_VER_SH);
	}else{
		printf("\n\n*****switch to new version fail******\n");
		return NULL;
	}
#else
	system(PREPARE_TO_SWITCH_VER_SH);
#endif
	
	if((access(FLAG_FILE_SWITCH_VER_OK,0) != 0)){
		printf("\n\n*****switch to new version fail******\n");
		return NULL;
	}

	//FIXME: better to broadcast a  module stop msg to every module .Now just a temp way to make things work
	//if not stop remoteid, the kernel will crash and hangup, because the capture packet from wifi is working.
	extern remoteid_stop();
	remoteid_stop();

	sleep(3);
	printf("OTA reboot!!!!!!!!!!!!\n");
	multi_boot_run_golden();
	return NULL;
}

static uint16_t upgrade_goto_app(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	upgrade_goto_app_ack_t *psGotoAppAck;
	int ret;
	pthread_t tid;

	printf("%s[%d]\n", __FUNCTION__, __LINE__);

	psGotoAppAck = (upgrade_goto_app_ack_t*)(rsp->buffer);


	ret = pthread_create(&tid, NULL, auto_reboot_task, NULL);
	if (ret)
	{
		psGotoAppAck->byStatus = UPDATE_RET_ERROR;
	}
	else
	{
		psGotoAppAck->byStatus = UPDATE_RET_OK;
		pthread_setname_np(&tid, "alink_upgrade");
		update_state = UPDATE_STAT_WAIT_REBOOT;
		firmware_save_ok = 0;
	}

	///multi_boot_run_golden();
	return sizeof(upgrade_goto_app_ack_t);
}

//align to 1KB
#define ALINK_MSG_RCV_BLOCK_SIZE                    (ALINK_MAX_PAYLOAD_LEN  & 0xfffffc00)
static uint16_t upgrade_get_timeout_info(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	upgrade_timeout_info_ack_t *psTimeoutInfoAck;
	
	psTimeoutInfoAck = (upgrade_timeout_info_ack_t*)(rsp->buffer);

	psTimeoutInfoAck->chechTimeoutS = 60;
	psTimeoutInfoAck->imagePkgLenMax = ALINK_MSG_RCV_BLOCK_SIZE;
	psTimeoutInfoAck->writeTimeoutS = 10*60;
	psTimeoutInfoAck->cmdTimeoutMs = 1000;
	return sizeof(upgrade_timeout_info_ack_t);
}

/*
EAP_ALINK_CMD_PC_REQUEST_SYS_RESET = 0xa1,//upgrade
EAP_ALINK_CMD_PC_GET_FIRMWARE_WRITE_STATUS = 0xa6,//upgrade
EAP_ALINK_CMD_PC_REQUEST_FIRMWARE_UPGRADE = 0xa7,//upgrade
EAP_ALINK_CMD_PC_REQUEST_DOWNLOAD_FIRMWARE_DATA = 0xa8,//upgrade
EAP_ALINK_CMD_PC_REQUEST_WRITE_FIRMWARE_DATA = 0xa9,//upgrade
EAP_ALINK_CMD_PC_GET_VERSION_INFO = 0xab,//upgrade
EAP_ALINK_CMD_PC_REQUEST_VERITY_FIRMWARE_DATA = 0xac,//upgrade
EAP_ALINK_CMD_PC_REQUEST_RUN_FIRMWARE = 0xad,//upgrade
EAP_ALINK_CMD_PC_GET_TIMEOUT_TIME = 0xae,//upgrade
*/
void alink_upgrade_init( void )
{
	firmware_save_ok = 0;
	run_mode = 1;
	update_state = UPDATE_STAT_NO_START;

	register_alink_cmd(EAP_ALINK_CMD_PC_GET_VERSION_INFO,	upgrade_get_version );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_SYS_RESET,	upgrade_reboot_system );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_FIRMWARE_UPGRADE,	upgrade_request_upgrade );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_DOWNLOAD_FIRMWARE_DATA,	upgrade_download_image );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_VERITY_FIRMWARE_DATA,	upgrade_check_image );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_WRITE_FIRMWARE_DATA,	upgrade_write_image );
	register_alink_cmd(EAP_ALINK_CMD_PC_GET_FIRMWARE_WRITE_STATUS,	upgrade_write_status );
	register_alink_cmd(EAP_ALINK_CMD_PC_REQUEST_RUN_FIRMWARE,	upgrade_goto_app );
	register_alink_cmd(EAP_ALINK_CMD_PC_GET_TIMEOUT_TIME,	upgrade_get_timeout_info );

}

#ifdef __cplusplus 
} 
#endif 


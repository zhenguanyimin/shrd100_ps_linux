
#ifndef ALINK_UPGRADE_H
#define ALINK_UPGRADE_H

#include "c2_alink.h"
#include "firmware.h"

#define ALINK_UPGRADE_RECV_BUFFER_SIZE (4096)

#pragma pack(1)
		typedef struct upgrade_version_ack
		{
			uint32_t	uRunVersion;
			char		strAppVersion[64];
			char		strBootVersion[32];
			char		strHwVersion[32];
			char		strProtocolVersion[32];
		} upgrade_version_ack_t;
#pragma pack()
	
#pragma pack(1)
			typedef struct upgrade_reset
			{
				uint16_t resetCode;
				uint16_t type;
			} upgrade_reset_t;
			typedef struct upgrade_reset_ack
			{
				uint8_t byStatus;
			} upgrade_reset_ack_t;
#pragma pack()

	
#pragma pack(1)
		typedef struct upgrade_request_upgrade
		{
			firmware_header_t sFirmwareHeader;
		} upgrade_request_upgrade_t;
		typedef struct upgrade_request_upgrade_ack
		{
			uint8_t byStatus;
		} upgrade_request_upgrade_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_download_image
		{
			uint32_t	imageOffset;
			uint32_t	imageLength;
			uint8_t 	imageData[ALINK_UPGRADE_RECV_BUFFER_SIZE];
		} upgrade_download_image_t;
		typedef struct upgrade_download_image_ack
		{
			uint8_t byStatus;//(byStatus & 0x80) == 1, using new struct which having ack_offset field; (byStatus & 0x7f) is ret value.
			uint8_t rsv1;
			uint8_t rsv2;
			uint8_t rsv3;
			uint32_t  ack_offset;//the confirmed offset of file.  little endian.
		} upgrade_download_image_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_check_image_ack
		{
			uint8_t byStatus;
		} upgrade_check_image_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_write_image_ack
		{
			uint8_t byStatus;
		} upgrade_write_image_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_write_status_ack
		{
			uint8_t 	byStatus;
			uint16_t	wPermil;
		} upgrade_write_status_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_goto_app_ack
		{
			uint8_t byStatus;
		} upgrade_goto_app_ack_t;
#pragma pack()
	
#pragma pack(1)
		typedef struct upgrade_timeout_info_ack
		{
			uint16_t	imagePkgLenMax;
			uint16_t	cmdTimeoutMs;
			uint16_t	chechTimeoutS;
			uint16_t	writeTimeoutS;
		} upgrade_timeout_info_ack_t;
#pragma pack()


void alink_upgrade_init( void );

#endif /* UPGRADE_H */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <linux/types.h>

#include "firmware.h"

#define FIRMWARE_FILE_NAME "firmware.bin"
#define FIRMWARE_HEADER_FILE_NAME "firmware-header"

#define FIRMWARE_FILE_PATH "/run/media/mmcblk0p2/"
#define FIRMWARE_CACHE_DIR "/tmp"
#define FIRMWARE_CACHE_FILE FIRMWARE_CACHE_DIR"/cache.bin"
#define FIRMWARE_DIR FIRMWARE_FILE_PATH"firmware"
#define FIRMWARE_UPDATE_FLAG FIRMWARE_FILE_PATH"update.flag"


typedef struct firmware_info
{
	uint32_t 			uOffset;
	firmware_header_t	sHeader;
	
	int cache_file_fd;
	uint32_t crc;
}firmware_info_t;


typedef struct file_info_s
{
	uint8_t flag[4];
	uint8_t name[64];
	uint8_t len[4];
}file_info_t;

static int firmware_wrote_image = 0;
static firmware_info_t firmware = {
	.cache_file_fd = -1,
};

#define WRITE_FAIL_RETRY_CNT (10)

void firmware_print_progress_bar(int progress, int total, int bar_steps) 
{
    int64_t percentage = ((int64_t)progress * 100) / (int64_t)total;
    int64_t numBars = ((int64_t)progress * (int64_t)bar_steps) / (int64_t)total;
	char buf[FIRMWARE_UPGRADE_PROGRESS_BAR_STEPS + 256] = {0};
	int len = 0;
	int buf_len = sizeof(buf) - 1;
	
    len += snprintf(buf+len,buf_len - len,"[");
    for (int i = 0; i < numBars; i++) {
        len += snprintf(buf+len,buf_len - len,"=");
    }
    for (int i = numBars; i < bar_steps; i++) {
        len += snprintf(buf+len,buf_len - len," ");
    }
    len += snprintf(buf+len,buf_len - len,"]  (%d/%d) %ld%%\r",progress,total,percentage);
	printf("%s",buf);
    fflush(stdout);
}


static uint32_t firmware_get_crc32(uint32_t *last_crc, uint8_t *buf, uint32_t len )
{
    uint8_t i;
    uint8_t* data = buf;
    uint32_t crc = 0xffffffff;        // Initial value

	if (last_crc) crc = ~(*last_crc);
    while(len--)
    {
        crc ^= *data++;                // crc ^= *data; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;// 0xEDB88320= reverse 0x04C11DB7
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

static bool firmware_check_header( firmware_header_t *psFwHeader )
{
	uint32_t uCrc;

	do{
		if( sizeof(firmware_header_t) != psFwHeader->headerLen )
			break;

		uCrc = firmware_get_crc32(NULL, (uint8_t*)psFwHeader, sizeof(firmware_header_t) - sizeof(psFwHeader->headCrc) );
		if( psFwHeader->headCrc != uCrc )
			break;

		if( FIRMWARE_BUFFER_SIZE < psFwHeader->firmwareLength )
			break;

		return true;
	}while(0);

	return false;
}

static bool firmware_write( firmware_info_t *psFirmware )
{
	loff_t len, all_len;
	int ret;
	file_info_t file_info;
	loff_t file_len;
	unsigned char full_name[64+64];
	unsigned long time;
	loff_t file_offset = 0;
	int fd;
	unsigned char buffer[4096];
	int cnt;

	printf("%s[%d]\n", __func__, __LINE__);
	
	for(cnt = 0; file_offset < psFirmware->uOffset && cnt < WRITE_FAIL_RETRY_CNT; cnt++)
	{						
		lseek(psFirmware->cache_file_fd, file_offset, SEEK_SET);
		ret = read(psFirmware->cache_file_fd, &file_info, sizeof(file_info));
		if (ret != sizeof(file_info))
		{
			goto out;
		}
		
		if (memcmp(file_info.flag, "AAAA", 4))
		{
			goto out;
		}
		
		file_len = (file_info.len[0] << 24) + (file_info.len[1] << 16) + (file_info.len[2] << 8) + file_info.len[3];
		file_offset += sizeof(file_info);
		if ((file_len + file_offset) >  psFirmware->uOffset)
		{
			goto out;
		}

		memset(full_name, 0, sizeof(full_name));
		snprintf(full_name, sizeof(full_name)-1, FIRMWARE_DIR"/%s", file_info.name);

		printf("%s[%d]write file:%s\n", __func__, __LINE__, full_name);

		fd = open(full_name, O_CREAT|O_RDWR, 0666);
		if (fd < 0)
		{
			goto out;
		}

		for (all_len=0; all_len < file_len; all_len+=len)
		{
			len = (sizeof(buffer)>(file_len - all_len))? (file_len - all_len):sizeof(buffer);
			ret = read(psFirmware->cache_file_fd, buffer, len);
			if (ret == len)
			{
				len = ret;
				ret = write(fd, buffer, len);
				if (ret != len)
				{
					break;
				}
			}
		}
		close(fd);

		if (all_len == file_len)
		{
			file_offset += file_len;
		}
	}

	if (file_offset == psFirmware->uOffset)
	{
		return true;
	}

	out:
	return false;
}

bool firmware_set_upgrade_flag( bool bFlag )
{
	int ret;
	loff_t len;
	int fd;
	
	if (bFlag)
	{
		fd = open(FIRMWARE_UPDATE_FLAG, O_CREAT|O_RDWR, 0666);
		if (fd < 0)
		{
			return false;
		}
		
		ret = write(fd, "update", 6);
		close(fd);
		
		return (ret != 6)? false:true;
	}
	else
	{
		unlink(FIRMWARE_UPDATE_FLAG);
		return true;
	}
}

bool firmware_check(void  )
{
	firmware_info_t *psFirmware = &firmware;
	
	if( firmware_check_header( &(psFirmware->sHeader) ) )
	{
		if( psFirmware->sHeader.firmwareLength == psFirmware->uOffset )
		{
			if ( psFirmware->sHeader.firmwareCrc == psFirmware->crc )
				return true;
		}
	}
	return false;
}


bool firmware_save(void)
{
	bool ret;
	
	ret = firmware_write(&firmware);
	
	if (firmware.cache_file_fd >= 0)
	{
		close(firmware.cache_file_fd);
		firmware.cache_file_fd = -1;
	}

	return ret;
}


bool firmware_recv_init(firmware_header_t *psFwHeader)
{
	int fd;
	int ret;
	
	if( firmware_check_header(psFwHeader) )
	{
		ret = system("rm "FIRMWARE_DIR" -rf");
		mkdir(FIRMWARE_DIR, 0666);
		unlink(FIRMWARE_CACHE_FILE);
		fd = open(FIRMWARE_CACHE_FILE, O_CREAT|O_RDWR, 0666);
		if (fd < 0)
		{
			return false;
		}

		memset(&firmware, 0, sizeof(firmware));
		firmware.cache_file_fd = fd;
		memcpy( &(firmware.sHeader), psFwHeader, sizeof(firmware_header_t) );

		firmware_print_progress_bar(0,psFwHeader->firmwareLength,FIRMWARE_UPGRADE_PROGRESS_BAR_STEPS);

		return true;
	}
	
	return false;
}

/*
return   >=0    the current offset of file received.
         <0     err.
*/
int firmware_recv_data(uint32_t uOffset, uint8_t* pbyData, uint32_t uLen )
{
	int fd;
	loff_t offset;
	ssize_t ret;
	
	fd = firmware.cache_file_fd;
	offset = firmware.uOffset;

	if( offset == uOffset )
	{
		if( FIRMWARE_BUFFER_SIZE >= (offset + uLen ))
		{
			lseek(fd, offset, SEEK_SET);
			ret = write(fd, pbyData, uLen);
			if (ret != uLen)
			{
				printf("write fail, %d!=%d, offset:%d\n", ret, uLen, uOffset);
				return -1;
			}

			firmware.crc = firmware_get_crc32(&firmware.crc, pbyData, uLen);
			firmware.uOffset += uLen;

			
			firmware_print_progress_bar(firmware.uOffset,firmware.sHeader.firmwareLength,FIRMWARE_UPGRADE_PROGRESS_BAR_STEPS);
			return firmware.uOffset;
		}
	}

	//the received fragment data is not what we want,just drop it.
	return firmware.uOffset;
}

